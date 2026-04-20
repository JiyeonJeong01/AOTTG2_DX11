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

NS_BEGIN(Client)

CMainApp::CMainApp()
{

}

CMainApp::~CMainApp()
{
    SYS_CORE.DestroyInstance();
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
