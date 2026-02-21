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

    /* TEST : Create GameObject with various components */
    for (int i = 0; i < 1; ++i) {
        {
            Engine::CGameObject* pObj = SYS_GAMEOBJECT.Create_GameObject();
            pObj->Add_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
            auto mr = pObj->Get_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
            auto* d = mr._Data();
            d->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::SHADER_VTXCOL); // 임시: material=shader
            d->hMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_CUBE);      // 또는 CreateCubeMesh()
            d->layer = RENDER_LAYER::NONBLEND;
            d->flags = RF_NONE;
        }

        {
            Engine::CGameObject* pObj = SYS_GAMEOBJECT.Create_GameObjectUI();
            pObj->Add_Component<CCanvasRenderer>(COMPONENT_TYPE::CANVAS_RENDERER);
            auto mr = pObj->Get_Component<CCanvasRenderer>(COMPONENT_TYPE::CANVAS_RENDERER);
            auto* d = mr._Data();
            d->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::SHADER_VTXCOL); // 임시: material=shader
            d->layer = RENDER_LAYER::NONBLEND;
            d->flags = RF_NONE;
        }

        //{
        //    {
        //        Engine::CGameObject* pObj = SYS_GAMEOBJECT.Create_GameObjectUI();

        //        // 1) 필수 컴포넌트들
        //        auto hCR = pObj->Add_Component<CCanvasRenderer>(COMPONENT_TYPE::CANVAS_RENDERER);
        //        auto hImg = pObj->Add_Component<CUIImage>(COMPONENT_TYPE::UI_IMAGE);
        //        auto hBtn = pObj->Add_Component<CUIButton>(COMPONENT_TYPE::UI_BUTTON);

        //        // 2) RectTransform 세팅 (Update_Buttons가 vPosPx/vSizePx 씀)
        //        auto rt = pObj->Get_Component<CRectTransform>(COMPONENT_TYPE::RECT_TRANSFORM);
        //        if (auto* d = rt._Data())
        //        {
        //            d->vPosPx = { 400.f, 300.f };   // 화면 좌표계랑 SYS_INPUT.Get_MousePos()랑 같은 기준이어야 함
        //            d->vSizePx = { 220.f, 80.f };
        //            d->dirty = true;                // 너 구조에 dirty 있으면
        //        }

        //        // 3) CanvasRenderer 세팅 (일단 임시)
        //        auto cr = pObj->Get_Component<CCanvasRenderer>(COMPONENT_TYPE::CANVAS_RENDERER);
        //        if (auto* d = cr._Data())
        //        {
        //            d->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::SHADER_VTXTEX); // 있으면 이걸로
        //            // 없으면 너가 쓰던 임시도 OK:
        //            // d->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::SHADER_VTXCOL);

        //            d->layer = RENDER_LAYER::NONBLEND;
        //            d->flags = RF_NONE;

        //            d->hTexture = YOUR_BUTTON_TEX;  // 필수 (0이면 안 나옴)
        //            d->rcUV = { 0.f, 0.f, 1.f, 1.f };
        //            d->vColor = { 1.f, 1.f, 1.f, 1.f };
        //        }

        //        // 4) UIImage: CanvasRenderer에 “동기화 대상” 걸어주기
        //        auto img = pObj->Get_Component<CUIImage>(COMPONENT_TYPE::UI_IMAGE);
        //        if (auto* d = img._Data())
        //        {
        //            d->hCanvasRenderer = hCR;   // 네 HANDLE 타입에 맞게 (uint32면 그대로)
        //            d->hTexture = YOUR_BUTTON_TEX;
        //            d->rcUV = { 0.f, 0.f, 1.f, 1.f };
        //            d->color = { 1.f, 1.f, 1.f, 1.f };
        //            d->dirty = true;               // Sync_Images_To_Canvas가 이거 보고 cr에 반영
        //        }

        //        // 5) UIButton: 히트박스(RT) + 바꿀 대상(CanvasRenderer) 연결
        //        auto btn = pObj->Get_Component<CUIButton>(COMPONENT_TYPE::UI_BUTTON);
        //        if (auto* d = btn._Data())
        //        {
        //            d->bInteractable = true;
        //            d->eState = ::Normal;

        //            d->hRectTransform = hRT;
        //            d->hTargetCanvas = hCR;

        //            d->normal = { 1,1,1,1 };
        //            d->hover = { 1,1,1,1 };
        //            d->pressed = { 0.8f,0.8f,0.8f,1 };
        //            d->disabled = { 0.3f,0.3f,0.3f,1 };

        //            d->normalTex = YOUR_BUTTON_TEX;
        //            d->hoverTex = INVALID_HANDLE_UINT;   // 없으면 normal로 fallback
        //            d->pressedTex = INVALID_HANDLE_UINT;

        //            d->normalUV = { 0,0,1,1 };
        //            d->hoverUV = d->normalUV;
        //            d->pressedUV = d->normalUV;
        //        }
        //    }
        //}

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
    return S_OK;
}

void CMainApp::Update(_float fDT)
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

    SYS_CORE.Update_Engine(fDT);

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
