#pragma once
#include "Base.h"
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
class CComponent_Pool : public CBase
{
public:
    /* Ensure that the TProxy type is derived from CComponent_Proxy_Base at compile-time */
    static_assert(
        std::is_base_of_v<CComponent_Proxy_Base<typename TProxy::DataType, TProxy>, TProxy>,
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
        uint64_t bActiveBitset[BITSET_COUNT]{};

        bool Is_Active(uint32_t iOffset) const
        {
            return bActiveBitset[iOffset / BITSET_WIDTH] & (1ULL << (iOffset % BITSET_WIDTH));
        }
        void Set_Active(uint32_t iOffset)
        {
            bActiveBitset[iOffset / BITSET_WIDTH] |= (1ULL << (iOffset % BITSET_WIDTH));
        }
        void Set_Inactive(uint32_t iOffset)
        {
            bActiveBitset[iOffset / BITSET_WIDTH] &= ~(1ULL << (iOffset % BITSET_WIDTH));
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

    CComponent_Pool() {};
    ~CComponent_Pool() override
	{
        /* NOTE: destroy all active objects before deleting pages */
        for (PAGE* pPage : m_pages)
        {
            if (!pPage)
                continue;

            for (uint32_t i = 0; i < PAGE_SIZE; ++i)
            {
                if (pPage->Is_Active(i))
                {
                    /* Call destructor for alive object */
                    Destroy_At(pPage->Get_Ptr(i));
                    pPage->Set_Inactive(i);
                }
            }

            Safe_Delete(pPage);
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

        uint32_t iPageIndex = iGlobalIndex >> PAGE_SHIFT;
        uint32_t iOffset = iGlobalIndex & PAGE_OFFSET_MASK;

        /* Allocate a new page if current capacity is exceeded */
        while (m_pages.size() <= iPageIndex)
        {
            m_pages.push_back(new PAGE());
            _DEBUG_INFO("Create new page to allocate component");
        }

        PAGE* pPage = m_pages[iPageIndex];

        if (pPage->iVersion[iOffset] > Component::MAX_VERSION) /* 2047, 11bit */
            pPage->iVersion[iOffset] = 0;

        /* Construct object IN PLACE (placement new) */
        Construct_At(pPage->Get_Ptr(iOffset));

        pPage->Set_Active(iOffset);


        /* TODO ================= TEST ===============================*/
        COMPONENT_HANDLE hTest = COMPONENT_HANDLE::Create(iGlobalIndex, pPage->iVersion[iOffset]);
        LOG_INFO("Add Component with handle { %d }", hTest.iHandle);
        /* TODO =======================================================*/


        return COMPONENT_HANDLE::Create(iGlobalIndex, pPage->iVersion[iOffset]);
    }

    void Deallocate(COMPONENT_HANDLE handle)
    {
        const uint32_t iIndex = handle.Get_Index();
        uint32_t iPageIndex = iIndex >> PAGE_SHIFT;
        uint32_t iOffset = iIndex & PAGE_OFFSET_MASK;

        if (iPageIndex >= SCAST(uint32_t, m_pages.size()))
        {
            _DEBUG_ERROR_BREAK("Invalid Handle: Page index out of range!");
            return;
        }

        PAGE* pPage = m_pages[iPageIndex];

        if (m_pages[iPageIndex]->iVersion[iOffset] != handle.Get_Version())
        {
            _DEBUG_ERROR_BREAK("Invalid Handle: Version mismatch!");
            return;
        }

        if (!pPage->Is_Active(iOffset))
        {
            _DEBUG_ERROR_BREAK("Invalid Handle: Already deallocated!");
            return;
        }

        DATA_T* pData = pPage->Get_Ptr(iOffset);

        /* For a notification event to external system; not for cleanup */
        m_OnDeallocate.Invoke(pData);

        /* Destroy data */
        Destroy_At(pData);

        /* Invalidate handle */
        pPage->iVersion[iOffset]++;
        pPage->Set_Inactive(iOffset);
        m_freeIndices.push_back(iIndex);
    }

    /**
     * \brief Reference-only view; not an owning handle.
     */
    TProxy Get_Proxy(COMPONENT_HANDLE handle)
    {
        /* Create and return a Proxy object initialized with the raw data address */
        DATA_T* pRawData = Get_Data_By_Handle(handle);
        if (!pRawData)
        {
            _DEBUG_ERROR_BREAK("Can't find such data!");
            return TProxy(nullptr, COMPONENT_HANDLE{});
        }
        return TProxy(pRawData, handle);
    }

    /* Retrieve the raw data pointer for internal processing. */
    DATA_T* Get_Data_By_Handle(COMPONENT_HANDLE handle)
	{
        const uint32_t iIndex = handle.Get_Index();
        uint32_t iPageIndex = iIndex >> PAGE_SHIFT;
        uint32_t iOffset = iIndex & PAGE_OFFSET_MASK;

        if (iPageIndex >= m_pages.size())
            return nullptr;

        PAGE* pPage = m_pages[iPageIndex];
        if (pPage->iVersion[iOffset] != handle.Get_Version() || !pPage->Is_Active(iOffset))
            return nullptr;

        return pPage->Get_Ptr(iOffset);
    }

    const std::vector<PAGE*>& GetPages() const
    {
        return m_pages;
    }

private:
    vector<PAGE*>       m_pages{};
    vector<uint32_t>    m_freeIndices{};
    uint32_t            m_iNextIndex = 1; /* 0 is invalid handle */

    CEvent<DATA_T*>     m_OnDeallocate{};

    /* Construction/Destruction helpers */
    static void Construct_At(DATA_T* p)
    {
        /* default-construct */
#pragma push_macro("new")
#undef new
        ::new ((void*)p) DATA_T();
#pragma pop_macro("new")

    }

    static void Destroy_At(DATA_T* p)
    {
        /* If DATA_T is trivial destructible, this compiles away */
        if constexpr (!std::is_trivially_destructible_v<DATA_T>)
        {
            std::destroy_at(p);
        }
    }
};
