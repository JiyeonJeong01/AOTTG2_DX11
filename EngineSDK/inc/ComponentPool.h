#pragma once
#include "Base.h"
#include "Component.h"

static constexpr uint32_t PAGE_SIZE = 1024;
static constexpr uint32_t PAGE_OFFSET_MASK = PAGE_SIZE - 1;
static constexpr uint32_t PAGE_SHIFT = 10;

template<typename T>
class CComponentPool : public CBase
{
public:
    struct PAGE
    {
        T           components[PAGE_SIZE];
        uint32_t    iVersion[PAGE_SIZE];
        bool        bActive[PAGE_SIZE];
    };

    CComponentPool() = default;
    ~CComponentPool() override
	{
        for (auto p : m_pages)
            Safe_Delete(p);
        m_pages.clear();
    }

    T* Allocate(COMPONENT_HANDLE& outHandle)
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

            /* 현재 페이지 전부 사용 중인 경우 */
            if (iGlobalIndex >= m_pages.size() * PAGE_SIZE) 
            {
                PAGE* pNewPage = new PAGE();
                memset(pNewPage->iVersion, 0, sizeof(uint32_t) * PAGE_SIZE);
                memset(pNewPage->bActive, 0, sizeof(bool) * PAGE_SIZE);
                m_pages.push_back(pNewPage);
            }
        }

        uint32_t iPageIndex = iGlobalIndex >> PAGE_SHIFT; /* / : 몇 번째 페이지인지 */
        uint32_t iOffset = iGlobalIndex & PAGE_OFFSET_MASK; /* % : 페이지의 몇 번째 원소인지 */
        PAGE* pPage = m_pages[iPageIndex];

        /* 버전 관리 */
        pPage->iVersion[iOffset]++;
        if (pPage->iVersion[iOffset] > 4095)
            pPage->iVersion[iOffset] = 1;

        pPage->bActive[iOffset] = true;

        outHandle = COMPONENT_HANDLE::Create(iGlobalIndex, pPage->iVersion[iOffset]);
        return &pPage->components[iOffset];
    }

    void Deallocate(COMPONENT_HANDLE handle)
    {
        const uint32_t iIndex = handle.Get_Index();
        uint32_t iPageIndex = iIndex >> PAGE_SHIFT;
        uint32_t iOffset = iIndex & PAGE_OFFSET_MASK;

        if (m_pages[iPageIndex]->iVersion[iOffset] == handle.Get_Version()) 
        {
            m_pages[iPageIndex]->bActive[iOffset] = false;
            m_freeIndices.push_back(iIndex);

            // 컴포넌트 정리는 컴포넌트 시스템에게 위임한다.
            if (m_OnDeallocate)
                m_OnDeallocate(m_pages[iPageIndex]->components[iOffset]);
        }
    }

    T* Get_By_Handle(COMPONENT_HANDLE handle)
	{
        const uint32_t iIndex = handle.Get_Index();
        uint32_t iPageIndex = iIndex >> PAGE_SHIFT;
        uint32_t iOffset = iIndex & PAGE_OFFSET_MASK;

        if (iPageIndex >= m_pages.size()) 
            return nullptr;

        PAGE* pPage = m_pages[iPageIndex];
        if (pPage->iVersion[iOffset] != handle.Get_Version() || !pPage->bActive[iOffset])
            return nullptr;

        return &(pPage->components[iOffset]);
    }

    const vector<PAGE*>& GetPages() const { return m_pages; }

private:
    vector<PAGE*>       m_pages;
    vector<uint32_t>    m_freeIndices;
    uint32_t            m_iNextIndex = 0;

    function<void(T*)> m_OnDeallocate;
};