#pragma once
#include "Engine_Log.h"
#include "CComponent_Proxy_Base.h"
#include "Event.h"

/* Memory Layout Constants for Paging System */
static constexpr uint32_t PAGE_SIZE = 1024;
static constexpr uint32_t PAGE_OFFSET_MASK = PAGE_SIZE - 1; /* % */
static constexpr uint32_t PAGE_SHIFT = 10;
static constexpr uint32_t BITSET_WIDTH = 64;
static constexpr uint32_t BITSET_COUNT = PAGE_SIZE / BITSET_WIDTH; // 1024 / 64 = 16

template<typename TProxy>
class CComponent_Pool final
{
public:
    /* Ensure that the TProxy type is derived from CComponent_Proxy_Base at compile-time */
    static_assert(
        std::is_base_of_v<CComponent_Proxy_Base<typename TProxy::DataType, TProxy, TProxy::ComponentType>, TProxy>,
        "Error: TProxy must inherit from CComponent_Proxy_Base!");

    /* Alias for the raw data type managed by this pool */
    using DATA_T = typename TProxy::DataType;

    /**
     * PAGE Structure: Contiguous memory block for Cache-Friendliness
     * Components must use a placement-new lifetime policy,
     * since they are non-trivial objects with explicit construction/destruction needs.
     */
    struct PAGE
    {
        alignas(DATA_T) std::byte storage[sizeof(DATA_T) * PAGE_SIZE]{};
        uint32_t iVersion[PAGE_SIZE]{};
        uint64_t bAllocatedBitset[BITSET_COUNT]{};

        bool Is_Allocated(uint32_t iOffset) const
        {
            return bAllocatedBitset[iOffset / BITSET_WIDTH] & (1ULL << (iOffset % BITSET_WIDTH));
        }
        void Set_Allocated(uint32_t iOffset)
        {
            bAllocatedBitset[iOffset / BITSET_WIDTH] |= (1ULL << (iOffset % BITSET_WIDTH));
        }
        void Set_Deallocated(uint32_t iOffset)
        {
            bAllocatedBitset[iOffset / BITSET_WIDTH] &= ~(1ULL << (iOffset % BITSET_WIDTH));
        }

        DATA_T* Get_Ptr(uint32_t iOffset)
        {
            return reinterpret_cast<DATA_T*>(storage + (size_t)iOffset * sizeof(DATA_T));
        }
        const DATA_T* Get_Ptr(uint32_t iOffset) const
        {
            return reinterpret_cast<const DATA_T*>(storage + (size_t)iOffset * sizeof(DATA_T));
        }
    };

    static_assert((PAGE_SIZE& (PAGE_SIZE - 1)) == 0, "PAGE_SIZE must be a power of two.");
    static_assert((1u << PAGE_SHIFT) == PAGE_SIZE, "PAGE_SHIFT must match PAGE_SIZE.");

    CComponent_Pool() = default;
    CComponent_Pool(const CComponent_Pool&) = delete;
    CComponent_Pool& operator=(const CComponent_Pool&) = delete;
    CComponent_Pool(CComponent_Pool&&) noexcept = default;
    CComponent_Pool& operator=(CComponent_Pool&&) noexcept = default;

    ~CComponent_Pool()
	{
        /* NOTE: destroy all active objects before deleting pages */
        for (auto& pPage : m_pages)
        {
            if (!pPage)
                continue;

            for (uint32_t i = 0; i < PAGE_SIZE; ++i)
            {
                if (pPage->Is_Allocated(i))
                {
                    /* Call destructor for alive object */
                    Destroy_At(pPage->Get_Ptr(i));
                    pPage->Set_Deallocated(i);
                }
            }
        }
        m_pages.clear();
    }

    COMPONENT_HANDLE Allocate()
    {
        uint32_t iGlobalIndex = 0;

        if (!m_freeIndices.empty()) 
        {
            iGlobalIndex = m_freeIndices.back();
            m_freeIndices.pop_back();
        }
        else 
        {
            iGlobalIndex = m_iNextIndex++;
        }

        const uint32_t iPageIndex = iGlobalIndex >> PAGE_SHIFT;
        const uint32_t iOffset = iGlobalIndex & PAGE_OFFSET_MASK;

        /* Allocate a new page if current capacity is exceeded */
        while (m_pages.size() <= iPageIndex)
        {
            m_pages.emplace_back(std::make_unique<PAGE>());
            _DEBUG_INFO("Create new page to allocate component");
        }

        PAGE* pPage = m_pages[iPageIndex].get();

        if (pPage->iVersion[iOffset] > Component::MAX_VERSION) /* 2047, 11bit */
            pPage->iVersion[iOffset] = 0;

        /* Construct object IN PLACE (placement new) */
        Construct_At(pPage->Get_Ptr(iOffset));

        pPage->Set_Allocated(iOffset);
        pPage->Get_Ptr(iOffset)->bEnable = true;

        /* TODO ================= TEST ===============================*/
        COMPONENT_HANDLE hTest = COMPONENT_HANDLE::Create(iGlobalIndex, pPage->iVersion[iOffset]);
        _DEBUG_INFO("Add Component with handle { %d }", hTest.iHandle);
        /* TODO =======================================================*/


        return COMPONENT_HANDLE::Create(iGlobalIndex, pPage->iVersion[iOffset]);
    }

    void Deallocate(COMPONENT_HANDLE handle)
    {
        const uint32_t iIndex = handle.Get_Index();
        const uint32_t iPageIndex = iIndex >> PAGE_SHIFT;
        const uint32_t iOffset = iIndex & PAGE_OFFSET_MASK;

        IF_TRUE_RETURN_MSG_BREAK((iPageIndex >= SCAST(uint32_t, m_pages.size())), , "Invalid Handle: Page index out of range!");

        PAGE* pPage = m_pages[iPageIndex].get();
        IF_TRUE_RETURN_MSG_BREAK((pPage->iVersion[iOffset] != handle.Get_Version()), , "Invalid Handle: Version mismatch!");
        IF_TRUE_RETURN_MSG_BREAK((!pPage->Is_Allocated(iOffset)), , "Invalid Handle: Already deallocated!");

        DATA_T* pData = pPage->Get_Ptr(iOffset);

        /* 컴포넌트 해제를 알리거나 페이지의 슬롯 정리 용도로 호출한다. */
        m_OnDeallocate.Invoke(handle, pData);

        /* Destroy data */
        Destroy_At(pData);

        /* Invalidate handle */
        pPage->iVersion[iOffset]++;
        pPage->Set_Deallocated(iOffset);
        m_freeIndices.push_back(iIndex);
    }

    /**
     * \brief Reference-only view; not an owning handle.
     */
    TProxy Get_Proxy(COMPONENT_HANDLE handle) noexcept
    {
        /* Create and return a Proxy object initialized with the raw data address */
        DATA_T* pRawData = Get_Data_By_Handle(handle);

        IF_NULL_RETURN_MSG_BREAK(pRawData, TProxy(nullptr, COMPONENT_HANDLE{}), "Can't find such data!");
        return TProxy(pRawData, handle);
    }

    /* Retrieve the raw data pointer for internal processing. */
    DATA_T* Get_Data_By_Handle(COMPONENT_HANDLE handle) noexcept
	{
        const uint32_t iIndex = handle.Get_Index();
        uint32_t iPageIndex = iIndex >> PAGE_SHIFT;
        uint32_t iOffset = iIndex & PAGE_OFFSET_MASK;

        if (iPageIndex >= SCAST(uint32_t, m_pages.size()))
            return nullptr;

        PAGE* pPage = m_pages[iPageIndex].get();
        if (pPage->iVersion[iOffset] != handle.Get_Version() || !pPage->Is_Allocated(iOffset))
            return nullptr;

        return pPage->Get_Ptr(iOffset);
    }

    const std::vector<std::unique_ptr<PAGE>>& GetPages() const
    {
        return m_pages;
    }

public :
    template <typename TProc>
    ListenerID Subscribe_OnDeallocate(void(TProc::*func)(COMPONENT_HANDLE, DATA_T*), TProc* pProc)
    {
        return m_OnDeallocate.Add_Listener(func, pProc);
    }

private:
    std::vector<std::unique_ptr<PAGE>> m_pages{};
    std::vector<uint32_t>    m_freeIndices{};
    uint32_t            m_iNextIndex = 1; /* 0 is invalid handle */

    CEvent<COMPONENT_HANDLE, DATA_T*>     m_OnDeallocate{};

    /* Construction/Destruction helpers */
    static void Construct_At(DATA_T* p)
    {
        /* default-construct */
#pragma push_macro("new")
#undef new
        ::new ((void*)p) DATA_T();
#pragma pop_macro("new")

    }

    static void Destroy_At(DATA_T* p) noexcept
    {
        /* DATA_T가 trivially destructible 타입이면 컴파일 타임에 아래 블록을 무시한다. */
        if constexpr (!std::is_trivially_destructible_v<DATA_T>)
        {
            std::destroy_at(p);
        }
    }
};
