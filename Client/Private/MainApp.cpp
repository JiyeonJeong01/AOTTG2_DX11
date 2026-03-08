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
#include "Input_System.h"

#include "BuiltIn_GUID.h"
#include "Script_Registry.h"
#include "Collider.h"
#include "Rigidbody.h"
#include "UIButton.h"



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

static CGameObject* pObject1 = nullptr;
static CGameObject* pObject2 = nullptr;
static CGameObject* pUI1 = nullptr;


HRESULT CMainApp::Initialize(const ENGINE_DESC& EngineDesc)
{
    if (FAILED(SYS_CORE.Initialize_Engine(EngineDesc, &m_pDevice, &m_pContext)))
        return E_FAIL;
    
    ASSET_GUID tmp("8976CDE9-2AE4-4DFC-A580-17E41FFB00D3");
    ASSET_GUID fiona("A727B7CD-8D3A-4EFE-BC5F-E59D0C34821C");
    ASSET_GUID default_mat("B69F88A4-F574-48AC-BD74-8410EA2AA6BB");

    /* TEST : Create GameObject with various components */
    //Engine::CGameObject* pGO = nullptr;
    //Engine::CGameObject* pUO = nullptr;
    for (int i = 0; i < 1; ++i) {

        //for (int j = 0; j < 1; ++j)
        //{
        //    pObject1 = SYS_GAMEOBJECT.Create_GameObject();
        //    pObject1->Get_Component<CTransform>().Translate({ 0.f, 5.f, 0.f });
        //    pObject1->Add_Component<CMeshRenderer>();
        //    pObject1->Add_Component<CCollider>();
        //    auto mr = pObject1->Get_Component<CMeshRenderer>();
        //    auto* d = mr._Data();
        //    d->hMaterial = SYS_RESOURCE.Load_Material(default_mat);
        //    d->hMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_CUBE);
        //    d->layer = RENDER_LAYER::NONBLEND;
        //    d->flags = RF_NONE;
        //    auto tr1 = pObject1->Get_Component<CTransform>();
        //    auto mr1 = pObject1->Get_Component<CMeshRenderer>();
        //    auto cldr = pObject1->Get_Component<CCollider>();
        //    cldr.Set_Shape(SHAPE::BOX);
        //    auto rb = pObject1->Add_Component<CRigidbody>();
        //    rb.Set_BodyType(BODY_TYPE::DYNAMIC);
        //    tr1->vPosition = { j * 3.f, j * 3.f, j * 2.f};
        //}
        //for (int j = 0; j < 1; ++j)
        //{
        //    Engine::CGameObject* pGO = nullptr;
        //    pGO = SYS_GAMEOBJECT.Create_GameObject();
        //    pGO->Add_Component<CMeshRenderer>();
        //    pGO->Add_Component<CCollider>();
        //    auto mr = pGO->Get_Component<CMeshRenderer>();
        //    auto* d = mr._Data();
        //    d->hMaterial = SYS_RESOURCE.Load_Material(default_mat);
        //    d->hMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT_NORTEX);
        //    d->layer = RENDER_LAYER::NONBLEND;
        //    d->flags = RF_NONE;
        //    auto tr1 = pGO->Get_Component<CTransform>();
        //    auto mr1 = pGO->Get_Component<CMeshRenderer>();
        //    auto cldr = pGO->Get_Component<CCollider>();
        //    cldr.Set_Shape(SHAPE::SPHERE);
        //    tr1->vPosition = { 2 + j * 3.f, j * 3.f, j * 2.f};
        //}
        //for (int j = 0; j < 1; ++j)
        //{
        //    pObject2 = SYS_GAMEOBJECT.Create_GameObject();
        //    pObject2->Add_Component<CMeshRenderer>();
        //    pObject2->Add_Component<CCollider>();
        //    pObject2->Get_Component<CCollider>().Set_Shape(SHAPE::PLANE);
        //    pObject2->Get_Component<CTransform>().Rotate({ 1.f, 0.f, 0.f }, 90.f);
        //    auto rb = pObject2->Add_Component<CRigidbody>();
        //    rb.Set_BodyType(BODY_TYPE::STATIC);
        //    rb.Set_Gravity(false);
        //    auto mr = pObject2->Get_Component<CMeshRenderer>();
        //    auto* d = mr._Data();
        //    d->hMaterial = SYS_RESOURCE.Load_Material(default_mat);
        //    d->hMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT);
        //    d->layer = RENDER_LAYER::NONBLEND;
        //    d->flags = RF_NONE;
        //}


        //    tr1->vPosition = { -2 + j * 3.f, j * 3.f, j * 2.f};
        //}

        //{
        //    pUI1 = SYS_GAMEOBJECT.Create_GameObjectUI();
        //    pUI1->Add_Component<CCanvasRenderer>();
        //    auto mr = pUI1->Get_Component<CCanvasRenderer>();
        //    auto* d = mr._Data();
        //    d->hMaterial = SYS_RESOURCE.Load_Material(DefaultAssetGuid::MATERIAL_VTXTEX);
        //    d->layer = RENDER_LAYER::NONBLEND;
        //    d->flags = RF_NONE;
        //    d->hTexture = SYS_RESOURCE.Load_Texture(tmp);
        //    auto btn = pUI1->Add_Component<CUIButton>();
        //    btn.OnClick().Add_Listener<CMainApp>(&CMainApp::OnClickTest, this);
        //    btn.OnHover().Add_Listener<CMainApp>(&CMainApp::OnHoverTest, this);

        //}

        //auto tr1 = pGO->Get_Component<CTransform>();
        //auto mr1 = pGO->Get_Component<CMeshRenderer>();

        //auto rt1 = pGO->Get_Component<CRectTransform>();
        //auto cr1 = pGO->Get_Component<CCanvasRenderer>();

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
    switch (eMode)
    {
    case Engine::APP_MODE::EDITOR_EDIT :
        SYS_CORE.Update_Editor_Engine(fDT);
        break;
    case APP_MODE::GAME_PLAY :
        SYS_CORE.Update_Game_Engine(fDT);
        break;
    }

    //const _float fSpeed = 5.f;
    //if (SYS_INPUT.Get_KeyDown('D'))
    //{
    //    pObject1->Get_Component<CRigidbody>().Add_Force({ fSpeed, 0.f, 0.f });
    //}
    //if (SYS_INPUT.Get_KeyDown('A'))
    //{
    //    pObject1->Get_Component<CRigidbody>().Add_Force({ -fSpeed, 0.f, 0.f });
    //}
    //if (SYS_INPUT.Get_KeyDown('W'))
    //{
    //    pObject1->Get_Component<CRigidbody>().Add_Force({ 0.f, fSpeed, 0.f });
    //}
    //if (SYS_INPUT.Get_KeyDown('S'))
    //{
    //    pObject1->Get_Component<CRigidbody>().Add_Force({  0.f, -fSpeed, 0.f });
    //}
    //if (SYS_INPUT.Get_KeyDown(VK_SPACE))
    //{
    //    pObject1->Get_Component<CRigidbody>().Add_Force({ 0.f, fSpeed * 100.f, 0.f });
    //}

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
