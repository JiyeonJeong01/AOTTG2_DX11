#include "TestComponentASystem.h"
#include "Component_System.h"
#include "Component_Spec.h"

HRESULT CTestComponentASystem::Initialize()
{
    SYS_COMPONENT.Register_Factory<CTestComponentA, TEST_A_SPEC>(COMPONENT_TYPE::TEST_A);

    return S_OK;
}

void CTestComponentASystem::LateUpdate(_float fDT)
{
}

HRESULT CTestComponentASystem::Initialize_From_Spec(COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    TEST_DATA_A* pData = m_Pool.Get_Data_By_Handle(handle);

    for (int i = 0; i < 4; ++i)
    {
        pData->vData[i] = SCAST(const TEST_A_SPEC*, pSpec)->vData[i];
    }

    pData->fAcc = 0.f;

    return S_OK;
}

void CTestComponentASystem::Process_A(_float fDT)
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
            TEST_DATA_A& data = *pData;
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

std::unique_ptr<CTestComponentASystem> CTestComponentASystem::Create()
{
    auto pInstance = std::make_unique<CTestComponentASystem>();

    if (FAILED(pInstance->Initialize()))
    {
        _DEBUG_ERROR_BREAK("Create instance failed");
        return nullptr;
    }

    return pInstance;
}


void CTestComponentASystem::Update(_float fDT)
{
    Process_A(fDT);
}
