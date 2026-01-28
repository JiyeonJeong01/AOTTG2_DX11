#include "TestComponentBSystem.h"

HRESULT CTestComponentBSystem::Init()
{
    return S_OK;
}

void CTestComponentBSystem::LateUpdate(_float fDT)
{
}

void CTestComponentBSystem::Process_B(_float fDT)
{
    auto Pages = m_Pool.GetPages();
    for (auto* pPage : Pages)
    {
        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Active(i)) continue;

            TEST_DATA_B& data = pPage->rawData[i];
            for (int j = 0; j < 4; ++j)
            {
                data.vData[j].x += data.vData[j].y * fDT;
                data.vData[j].y -= data.vData[j].z * fDT;
                data.vData[j].z *= 1.00001f;
                data.fAcc += data.vData[j].x + data.vData[j].y + data.vData[j].z;
            }
        }
    }
}

void CTestComponentBSystem::Update(_float fDT)
{
    Process_B(fDT);
}
