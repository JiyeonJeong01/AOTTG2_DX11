#include "MainApp.h"
#include "Core_System.h"
#include "GameObject.h"

/* ====== Component Test ====== */
#include "Tester.h"
#include "Prototype_System.h"
#include "Component_Spec.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Resource_System.h"
#include "CanvasRenderer.h"
#include "RectTransform.h"
#include "Asset_Registry.h"
#include "Script_Handler.h"

#include "BuiltIn_GUID.h"
#include "Script_Registry.h"


namespace Engine
{
    class CRectTransform;
    class CUIImage;
    class CUIButton;
}

static ASSET_GUID testGUID{};
/* ============================ */

NS_BEGIN(Client)

CMainApp::CMainApp()
{

}

CMainApp::~CMainApp()
{
    delete m_pTester;

    SYS_CORE.DestroyInstance();
}

HRESULT CMainApp::Initialize(const ENGINE_DESC& EngineDesc)
{
    if (FAILED(SYS_CORE.Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
        return E_FAIL;
    
    ASSET_GUID tmp("8976CDE9-2AE4-4DFC-A580-17E41FFB00D3");

    /* TEST : Create GameObject with various components */
    Engine::CGameObject* pGO = nullptr;
    Engine::CGameObject* pUO = nullptr;
    for (int i = 0; i < 1; ++i) {
        {
            pGO = SYS_GAMEOBJECT.Create_GameObject();
            pGO->Add_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
            auto mr = pGO->Get_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
            auto* d = mr._Data();
            d->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::SHADER_VTXTEX);
            d->hMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_CUBE);      // 또는 CreateCubeMesh()
            d->layer = RENDER_LAYER::NONBLEND;
            d->flags = RF_NONE;
            d->hMainTex = SYS_RESOURCE.Load_Texture(tmp);

            auto tr1 = pGO->Get_Component<CTransform>(COMPONENT_TYPE::TRANSFORM);
            auto mr1 = pGO->Get_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
        }

        {
            pUO = SYS_GAMEOBJECT.Create_GameObjectUI();
            pUO->Add_Component<CCanvasRenderer>(COMPONENT_TYPE::CANVAS_RENDERER);
            auto mr = pUO->Get_Component<CCanvasRenderer>(COMPONENT_TYPE::CANVAS_RENDERER);
            auto* d = mr._Data();
            d->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::SHADER_VTXTEX);
            d->layer = RENDER_LAYER::NONBLEND;
            d->flags = RF_NONE;
            d->hTexture = SYS_RESOURCE.Load_Texture(tmp);
        }

        auto tr1 = pGO->Get_Component<CTransform>(COMPONENT_TYPE::TRANSFORM);
        auto mr1 = pGO->Get_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);

        auto rt1 = pGO->Get_Component<CRectTransform>(COMPONENT_TYPE::RECT_TRANSFORM);
        auto cr1 = pGO->Get_Component<CCanvasRenderer>(COMPONENT_TYPE::CANVAS_RENDERER);

    }

    // Somewhere test code
    //PROTOTYPE_SPEC spec{};
    //spec.strName = "TEST_PROTO";
    // bundle 채우기 (예시)
    //COMPONENT_SPEC_BUNDLE bundle{};
    //m_pTestA = std::make_unique<TEST_A_SPEC>();
    //m_pTestA->vData[0] = { 0, 0, 0 };
    //m_pTestA->vData[1] = { 1.f, 1.f, 1.f };
    //m_pTestA->vData[2] = { 2.f, 2.f, 2.f };
    //m_pTestA->vData[3] = { 3.f, 3.f, 3.f };
    //bundle.components.push_back(std::move(m_pTestA));
    //spec.tComponentBundle = std::move(bundle);
    //HRESULT hr = CPrototype_System::GetInstance().Register_Prototype(testGUID = ASSET_GUID::New_GUID(), std::move(spec));
    //if (hr > 0)
    //    _DEBUG_INFO("Prototype 등록됨");

    m_pTester = Tester::Create();
    m_pTester->Initialize_Tester(this);

    HRESULT hr = SYS_ASSET.Scripts().Generate("../../Client/Private/Script_Registry.gen.cpp");
    IF_FAIL_RETURN_MSG_BREAK(hr, hr, "Create registry filed failed");

    Register_AllScripts();


    return S_OK;
}

void CMainApp::Update(_float fDT, Engine::APP_MODE eMode)
{
    /* Priority_Update */

    /* Update */

    /* Late_Update */



    /*  Test  */

    if (GetAsyncKeyState('M') & 0x8000)
    {
        m_voidEvent.Invoke();
    }
    if (GetAsyncKeyState('N') & 0x8000)
    {
        m_intFloatEvent.Invoke(0, 2.0);
    }
    if (GetAsyncKeyState('B') & 0x8000)
    {
        m_intEvent.Invoke(10);
    }
    if (GetAsyncKeyState('A') & 0x8000)
    {
        //Engine::CGameObject* pObj = CPrototype_System::GetInstance().Clone(testGUID);
    }

    switch (eMode)
    {
    case Engine::APP_MODE::EDITOR_EDIT :
        SYS_CORE.Update_Editor_Engine(fDT);
        break;
    case APP_MODE::GAME_PLAY :
        SYS_CORE.Update_Game_Engine(fDT);
        break;
    }

}

void CMainApp::Fixed_Update(_float fDT)
{
    /* Fixed_Update */
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
