#include "MainApp.h"
#include "Core_System.h"
#include "GameObject.h"

/* ====== Component Test ====== */
#include "TestComponentA.h"
#include "TestComponentB.h"
#include "Tester.h"
#include "Prototype_System.h"
#include "Component_Spec.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Resource_System.h"


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
        Engine::CGameObject* pObj = SYS_GAMEOBJECT.Create_GameObject();
        pObj->Add_Component<CTestComponentA>(COMPONENT_TYPE::TEST_A);
        pObj->Add_Component<CTestComponentB>(COMPONENT_TYPE::TEST_B);

        pObj->Add_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
        auto mr = pObj->Get_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
        auto* d = mr._Data();
        d->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::SHADER_VTXCOL); // 임시: material=shader
        d->hMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_CUBE);      // 또는 CreateCubeMesh()
        d->layer = RENDER_LAYER::NONBLEND;
        d->flags = RF_NONE;

        auto comA = pObj->Get_Component<CTestComponentA>(COMPONENT_TYPE::TEST_A);
        auto comB = pObj->Get_Component<CTestComponentB>(COMPONENT_TYPE::TEST_B);
        auto comsA = pObj->Get_Components<CTestComponentA>(COMPONENT_TYPE::TEST_A);
        auto comsB = pObj->Get_Components<CTestComponentB>(COMPONENT_TYPE::TEST_B);
    }

    // Somewhere test code
    PROTOTYPE_SPEC spec{};
    spec.strName = "TEST_PROTO";

    // bundle 채우기 (예시)
    COMPONENT_SPEC_BUNDLE bundle{};
    m_pTestA = std::make_unique<TEST_A_SPEC>();
    m_pTestA->vData[0] = { 0, 0, 0 };
    m_pTestA->vData[1] = { 1.f, 1.f, 1.f };
    m_pTestA->vData[2] = { 2.f, 2.f, 2.f };
    m_pTestA->vData[3] = { 3.f, 3.f, 3.f };

    bundle.components.push_back(std::move(m_pTestA));
    spec.tComponentBundle = std::move(bundle);

    HRESULT hr = CPrototype_System::GetInstance().Register_Prototype(testGUID = ASSET_GUID::New_GUID(), std::move(spec));
    if (hr > 0)
        LOG_INFO("Prototype 등록됨");

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
        Engine::CGameObject* pObj = CPrototype_System::GetInstance().Clone(testGUID);
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
