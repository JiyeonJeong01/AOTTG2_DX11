#include "TestComponentBSystem.h"
#include "Component_System.h"
#include "Component_Spec.h"

HRESULT CTestComponentBSystem::Initialize()
{
    SYS_COMPONENT.Register_Factory<CTestComponentB, TEST_B_SPEC>(COMPONENT_TYPE::TEST_B);

    return S_OK;
}

void CTestComponentBSystem::LateUpdate(_float fDT)
{
}

void CTestComponentBSystem::Process_B(_float fDT)
{
    const auto& Pages = m_Pool.GetPages();

    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Active(i)) continue;
            auto* pData = pPage->Get_Ptr(i);
            TEST_DATA_B& data = *pData;

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

std::unique_ptr<CTestComponentBSystem> CTestComponentBSystem::Create()
{
    auto pInstance = std::make_unique<CTestComponentBSystem>();

    if (FAILED(pInstance->Initialize()))
    {
        _DEBUG_ERROR_BREAK("Create instance failed");
        return nullptr;
    }

    return pInstance;
}

void CTestComponentBSystem::Update(_float fDT)
{
    Process_B(fDT);
}
