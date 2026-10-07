#include "MainApp.h"
#include "Core_System.h"
#include "GameObject.h"

/* ====== Component Test ====== */
#include "Tester.h"
#include "Prototype_System.h"
#include "Component_Spec.h"
#include "Resource_System.h"
#include "Asset_Registry.h"
#include "Script_Handler.h"
#include "Script_Registry.h"
#include "CinematicSystem.h"
#include "Input_System.h"
#include "GameObject_System.h"

NS_BEGIN(Client)

CMainApp::CMainApp()
{

}

CMainApp::~CMainApp()
{
    /* 시스템이 모두 살아 있을 때 씬 객체(스크립트 포함)를 씬 전환과 같은 경로로 정리한다.
       시스템 파괴 도중 스크립트 소멸자가 SYS_XXX 를 부르면 빈 싱글톤이 되살아난다. */
    if (CGameObject_System::GetInstancePtr())
    {
        SYS_GAMEOBJECT.Destroy_All_SceneObjects();
        SYS_GAMEOBJECT.Flush_PendingDestroy();
    }

    /* Initialize_Engine 이 AddRef 해서 넘겨준 참조 */
    Safe_Release(m_pDevice);
    Safe_Release(m_pContext);

    CCore_System::DestroyInstance();
}

HRESULT CMainApp::Initialize(const ENGINE_DESC& EngineDesc)
{
    if (FAILED(SYS_CORE.Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
        return E_FAIL;

    HRESULT hr = SYS_ASSET.Scripts().Generate("../../Client/Private/Script_Registry.gen.cpp");
    IF_FAIL_RETURN_MSG_BREAK(hr, hr, "Create registry filed failed");

    Register_AllScripts();

    return S_OK;
}

void CMainApp::Update(_float fDT, Engine::APP_MODE eMode)
{
    if (SYS_INPUT.Get_KeyDown(VK_F9))
        Tester::Run_ComponentPoolPageTest();

    switch (eMode)
    {
    case Engine::APP_MODE::EDITOR_EDIT :
        SYS_CORE.Update_Editor_Engine(fDT);
        break;
    case APP_MODE::GAME_PLAY :
        SYS_CORE.Update_Game_Engine(fDT);
        break;
    }

    //if (SYS_INPUT.Get_KeyDown('T') && m_camTest.Is_Valid())
    //{
    //    GAME_INSTANCE.Test_Start_Cinematic(&m_camTest);
    //    m_bTestStarted = true;
    //}
    //if (m_bTestStarted)
    //{
    //    SYS_CINEMATIC.Update(fDT);
    //}
}

void CMainApp::Late_Update(_float fDT)
{

}

void CMainApp::Fixed_Update(_float fDT)
{

}

HRESULT CMainApp::Render()
{
    /* Render all GameObjects*/
    SYS_CORE.Draw();

    return S_OK;
}

std::unique_ptr<CMainApp> CMainApp::Create(const ENGINE_DESC& Engine_Desc)
{
    auto pInstance = std::make_unique<CMainApp>();

    if (FAILED(pInstance->Initialize(Engine_Desc)))
    {
        MSG_BOX("Failed to Created : CMainApp");
        return nullptr;
    }
    return pInstance;
}

NS_END;
