#pragma once
#include "Base.h"
#include "Engine_Log.h"
#include "CComponent_Proxy_Base.h"

/* Memory Layout Constants for Paging System */
static constexpr uint32_t PAGE_SIZE = 1024;
static constexpr uint32_t PAGE_OFFSET_MASK = PAGE_SIZE - 1;
static constexpr uint32_t PAGE_SHIFT = 10;
static constexpr uint32_t BITSET_WIDTH = 64;
static constexpr uint32_t BITSET_COUNT = PAGE_SIZE / BITSET_WIDTH; // 1024 / 64 = 16

template<typename PROXY>
class CComponent_Pool : public CBase
{
public:
    /* Ensure that the PROXY type is derived from CComponent_Proxy_Base  at compile-time! */
    static_assert(std::is_base_of_v<CComponent_Proxy_Base<typename PROXY::DataType, PROXY>, PROXY>,
        "Error: PROXY must inherit from CComponent_Proxy_Base!");

    /* Alias for the raw data type managed by this pool */
    using DATA_T = typename PROXY::DataType;

    /**
     * PAGE Structure: Contiguous memory block for Cache-Friendliness
     * rawData: Actual component data.
     * iVersion: Version tracking for Handle-Validation.
     * bActiveBitset: Slot occupancy state.
     */
    struct PAGE
    {
        DATA_T      rawData[PAGE_SIZE]{};
        uint32_t    iVersion[PAGE_SIZE]{};
        uint64_t    bActiveBitset[BITSET_COUNT]{};

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
    };

    CComponent_Pool() {};
    ~CComponent_Pool() override
	{
        for (auto p : m_pages)
            Safe_Delete(p);
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
        }

        PAGE* pPage = m_pages[iPageIndex];

        pPage->iVersion[iOffset]++;
        if (pPage->iVersion[iOffset] > ComponentConfig::MAX_VERSION) /* 2047 */
            pPage->iVersion[iOffset] = 1;

        pPage->Set_Active(iOffset);

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

        if (m_pages[iPageIndex]->iVersion[iOffset] != handle.Get_Version())
        {
            _DEBUG_ERROR_BREAK("Invalid Handle: Version mismatch!");
            return;
        }

        m_pages[iPageIndex]->Set_Inactive(iOffset);
        m_freeIndices.push_back(iIndex);

        /* Execute cleanup callback (delegated to Component System) */
    	if (m_OnDeallocate)
            m_OnDeallocate(&m_pages[iPageIndex]->rawData[iOffset]);
    }

    /* Create and return a Proxy object initialized with the raw data address */
    PROXY Get_Proxy(COMPONENT_HANDLE handle)
    {
        DATA_T* pRawData = Get_Data_By_Handle(handle);
        if (!pRawData)
        {
            _DEBUG_ERROR_BREAK("Can't find such data!");
            return PROXY(nullptr, COMPONENT_HANDLE{});
        }
        return PROXY(pRawData, handle);
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

        return &(pPage->rawData[iOffset]);
    }

    const vector<PAGE*>& GetPages() const { return m_pages; }

private:
    vector<PAGE*>       m_pages{};
    vector<uint32_t>    m_freeIndices{};
    uint32_t            m_iNextIndex = 0;

    function<void(DATA_T*)> m_OnDeallocate;
};
