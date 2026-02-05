#include "MainApp.h"
#include "Core_System.h"
#include "GameObject.h"

/* ====== Component Test ====== */
#include "TestComponentA.h"
#include "TestComponentB.h"
#include "Tester.h"
#include "Prototype_System.h"
#include "Component_Spec.h"
/* ============================ */

NS_BEGIN(Client)

CMainApp::CMainApp()
{
}

HRESULT CMainApp::Initialize(const ENGINE_DESC& EngineDesc)
{
    if (FAILED(SYS_CORE->Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
        return E_FAIL;

    /* TEST : Create GameObject with various components */
    for (int i = 0; i < 5; ++i) {
        Engine::CGameObject* pObj = Engine::CGameObject::Create();
        pObj->Add_Component<CTestComponentA>(COMPONENT_TYPE::TEST_A);
        pObj->Add_Component<CTestComponentB>(COMPONENT_TYPE::TEST_B);
        pObj->Add_Component<CTestComponentA>(COMPONENT_TYPE::TEST_A);

        auto comA = pObj->Get_Component<CTestComponentA>(COMPONENT_TYPE::TEST_A);
        auto comB = pObj->Get_Component<CTestComponentB>(COMPONENT_TYPE::TEST_B);
        auto comsA = pObj->Get_Components<CTestComponentA>(COMPONENT_TYPE::TEST_A);
        auto comsB = pObj->Get_Components<CTestComponentB>(COMPONENT_TYPE::TEST_B);
        m_GameObjects.push_back(pObj);
    }

    LOG_INFO("%d개 생성됨", (int)m_GameObjects.size());

    m_pTester = Tester::Create();
    m_pTester->Initialize_Tester(this);
    return S_OK;
}

void CMainApp::Update(_float fDT)
{
    /* Priority_Update */

    /* Update */

    /* Late_Update */



    /*  Test  */

    //if (GetAsyncKeyState('M') & 0x8000)
    //{
    //    m_voidEvent.Invoke();
    //}
    //if (GetAsyncKeyState('N') & 0x8000)
    //{
    //    m_intFloatEvent.Invoke(0, 2.0);
    //}
    //if (GetAsyncKeyState('B') & 0x8000)
    //{
    //    m_intEvent.Invoke(10);
    //}

}


void CMainApp::Fixed_Update(_float fDT)
{
    /* Fixed_Update */
}

HRESULT CMainApp::Begin_Render()
{
    _float4		vClearColor = { 0.18f, 0.18f, 0.18f, 1.0f };
    if (FAILED(SYS_CORE->Clear_Buffers(&vClearColor)))
        return E_FAIL;

    return S_OK;
}

HRESULT CMainApp::Render()
{
    /* Render all GameObjects*/


    return S_OK;
}

HRESULT CMainApp::End_Render()
{
    if (FAILED(SYS_CORE->Present()))
        return E_FAIL;

    return S_OK;
}

CMainApp* CMainApp::Create(const ENGINE_DESC& Engine_Desc)
{
    CMainApp* pInstance = new CMainApp();

    if (FAILED(pInstance->Initialize(Engine_Desc)))
    {
        MSG_BOX("Failed to Created : CMainApp");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CMainApp::Free()
{
    __super::Free();
}

NS_END;
