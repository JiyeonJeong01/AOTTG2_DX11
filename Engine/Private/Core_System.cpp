#include "Core_System.h"

/* --- main --- */
#include "GameObject_System.h"
#include "Component_System.h"
#include "Input_System.h"
#include "Logger.h"
#include "Asset_Registry.h"
#include "Resource_System.h"
#include "Event_System.h"
#include "CRender_System.h"
#include "Editor_System.h"
#include "GameInstance.h"

/* --- sub --- */
#include "Graphic_Device.h"
#include "Timer_System.h"
#include "Physics_Processor.h"

/* --- --- */
#include "Scene.h"

/* --- event --- */
#include "Scene_Handler.h"
#include "WindowResize_Event.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CCore_System)

CCore_System::CCore_System()
{

}

CCore_System::~CCore_System()
{
    SYS_COMPONENT.DestroyInstance();
    SYS_GAMEOBJECT.DestroyInstance();
    SYS_ASSET.DestroyInstance();
    SYS_INPUT.DestroyInstance();
    SYS_LOG.DestroyInstance();
    SYS_EVENT.DestroyInstance();
    SYS_RENDER.DestroyInstance();
    SYS_RESOURCE.DestroyInstance();
}

HRESULT CCore_System::Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice,
	ID3D11DeviceContext** ppContext)
{
    _uint iWidth = EngineDesc.iViewportSize.first;
    _uint iHeight = EngineDesc.iViewportSize.second;

    /* --- Graphic Device ---*/
    m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd, EngineDesc.eWinMode, iWidth, iHeight, ppDevice, ppContext);
    IF_NULL_RETURN_MSG_BREAK(m_pGraphic_Device, E_FAIL, "CGraphic_Device create failed");

    m_pDevice = *ppDevice;
    m_pContext = *ppContext;

    /* --- Timer ---*/
    m_pTimer_Handler = CTimer_Handler::Create();
    IF_NULL_RETURN_MSG_BREAK(m_pTimer_Handler, E_FAIL, "CTimer_Handler create failed");

    /* --- Scene ---*/
    m_pScene_Handler = CScene_Handler::Create();
    IF_NULL_RETURN_MSG_BREAK(m_pScene_Handler, E_FAIL, "CScene_Handler create failed");

    /* --- Log System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_LOG.Initialize(), E_FAIL, "Log System failed Initialize");

    /* --- Resource System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_RESOURCE.Initialize(*ppDevice, *ppContext), E_FAIL, "Resource System failed Initialize");

    /* --- Asset System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_ASSET.Initialize(ProjectConfig::PATH + ProjectConfig::ROOT), E_FAIL, "Asset System failed Initialize");

    /* --- Component System ---*/
    IF_FAIL_RETURN_MSG_BREAK(SYS_COMPONENT.Initialize(m_pDevice, m_pContext), E_FAIL, "Component System failed Initialize");

    /* --- Object System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_GAMEOBJECT.Initialize(), E_FAIL, "Object System failed Initialize");

    /* --- Input System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_INPUT.Initialize(EngineDesc.hWnd, EngineDesc.hInst), E_FAIL, "Input System failed Initialize");

    /* --- Event System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_EVENT.Initialize(), E_FAIL, "Event System failed Initialize");

    /* --- Render System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_RENDER.Initialize(m_pDevice, m_pContext, iWidth, iHeight), E_FAIL, "Renderer System failed Initialize");

    /* --- Editor_System --- */
    IF_FAIL_RETURN_MSG_BREAK(SYS_EDITOR.Initialize(ProjectConfig::PATH + ProjectConfig::ROOT), E_FAIL, "Editor System failed Initialize");

    /* --- GameInstance --- */
    IF_FAIL_RETURN_MSG_BREAK(GAME_INSTANCE.Initialize(m_pDevice, m_pContext), E_FAIL, "GAME_INSTANCE failed Initialize");

    /* --- Register event --- */
    SYS_EVENT.Subscribe(EVENT_TYPE::On_Window_Resize, &CCore_System::On_Resize, this);

	return S_OK;
}

void CCore_System::Update_Editor_Engine(_float fDT)
{
    CScene* pScene = m_pScene_Handler->Get_CurrentScene();
    if (!pScene) return;

    SYS_INPUT.Update_System();
    SYS_RENDER.Priority_Update();

    SYS_EDITOR.Update(fDT);

    switch (pScene->Get_State())
    {
    case SCENE_STATE::EDIT :
    case SCENE_STATE::PAUSE : /* 스텝 허용 */
        SYS_COMPONENT.Update_Debug();
        return;
    case SCENE_STATE::PLAY :
        {
            Update_RuntimeEngine(fDT, pScene);
            Fixed_Update(m_FIXED_DT);
            GAME_INSTANCE.Test_LineRibbonMesh();

            //m_fTimeAcc += fDT;
            //if (m_fTimeAcc >= m_FIXED_DT)
            //{
            //    Fixed_Update(m_FIXED_DT);

            //    m_fTimeAcc = 0.f;
            //}
        }
        return;
    }
}

void CCore_System::Update_Game_Engine(_float fDT)
{
    CScene* pScene = nullptr;
    //CScene* pScene = m_pScene_Handler->Get_CurrentScene();
    //if (!pScene) return;

    SYS_INPUT.Update_System();
    SYS_RENDER.Priority_Update();
    Update_RuntimeEngine(fDT, pScene);

    m_fTimeAcc += fDT;
    if (m_fTimeAcc >= m_FIXED_DT)
    {
        Fixed_Update(m_FIXED_DT);

        m_fTimeAcc = 0.f;
    }

}

void CCore_System::Fixed_Update(_float fDT)
{
    SYS_COMPONENT.FixedUpdate(fDT);
}

void CCore_System::Request_Step(_float fDT, CScene* pScene)
{
    if (pScene->Get_State() == SCENE_STATE::PAUSE)
        Update_RuntimeEngine(fDT, pScene);
}

HRESULT CCore_System::Draw()
{
    /* 디버깅 정보 등 필요 시 아래 로직 추가 */
    //CScene* pScene = m_pScene_Handler->Get_CurrentScene();
    //if (pScene)
    //    pScene->Render();

    SYS_RENDER.Render();
    SYS_COMPONENT.Render();

    return S_OK;
}

void CCore_System::Clear_Resources(_uint iLevelIndex)
{
}

void CCore_System::On_Resize(EVENT_DATA& eData)
{
    assert(eData.eType == EVENT_TYPE::On_Window_Resize);

    auto& eResizeData = SCAST(RESIZE_EVENT_DATA&, eData);
    m_pGraphic_Device->On_Resize(eResizeData.iWidth, eResizeData.iHeight);
}

void CCore_System::Share_GraphicDevice(ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
    if (ppDevice)    
        *ppDevice = m_pDevice;
    if (ppContext)
        *ppContext = m_pContext;
}

void CCore_System::Share_SceneSRV(ID3D11ShaderResourceView** ppSRV)
{
    if (!ppSRV) return;
    *ppSRV = m_pGraphic_Device ? m_pGraphic_Device->Get_SceneSRV() : nullptr;
}

HRESULT CCore_System::Ready_SceneRenderTarget(_uint iWidth, _uint iHeight)
{
    return m_pGraphic_Device->Ensure_SceneRenderTarget(iWidth, iHeight);
}

void CCore_System::Bind_DefaultRTV()
{
    m_pGraphic_Device->Bind_DefaultRTV();
}

void CCore_System::Bind_SceneRTV()
{
    m_pGraphic_Device->Bind_SceneRTV();
}

HRESULT CCore_System::Clear_Default_Buffers(const _float4* pClearColor) const
{
	if (FAILED(m_pGraphic_Device->Clear_Default_RTV(pClearColor)))
		return E_FAIL;

	if (FAILED(m_pGraphic_Device->Clear_Default_DSV()))
		return E_FAIL;

	return S_OK;
}

HRESULT CCore_System::Clear_Scene_Buffers(const _float4* pClearColor) const
{
    if (FAILED(m_pGraphic_Device->Clear_Scene_RTV(pClearColor)))
        return E_FAIL;

    if (FAILED(m_pGraphic_Device->Clear_Scene_DSV()))
        return E_FAIL;

    return S_OK;
}

HRESULT CCore_System::Present() const
{
	return m_pGraphic_Device->Present();
}

_float CCore_System::Compute_SystemDT() const
{
	return m_pTimer_Handler->Compute_SystemDT();
}
_float CCore_System::Compute_FrameDT() const
{
	return m_pTimer_Handler->Compute_FrameDT();
}

_float CCore_System::Get_FrameDT() const
{
    return m_pTimer_Handler->Get_FrameDT();
}

HRESULT CCore_System::Register_Scenes(const ASSET_GUID& tGUID, const std::filesystem::path& scenePath)
{
    return m_pScene_Handler->Register_Scenes(tGUID, scenePath);
}

HRESULT CCore_System::Open_EditScene(const std::string& sceneName)
{
    ASSET_GUID outGuUID{};
    m_pScene_Handler->Find_GUID_By_Name(sceneName, outGuUID);
    return m_pScene_Handler->Open_EditScene(outGuUID);
}

HRESULT CCore_System::Open_EditScene(const ASSET_GUID& tGUID)
{
    return m_pScene_Handler->Open_EditScene(tGUID);
}

HRESULT CCore_System::Change_Scene(const std::string& sceneName)
{
    ASSET_GUID outGuUID{};
    m_pScene_Handler->Find_GUID_By_Name(sceneName, outGuUID);
    return m_pScene_Handler->Change_Scene(outGuUID);
}

HRESULT CCore_System::Change_Scene(const ASSET_GUID& tGUID)
{
    return m_pScene_Handler->Change_Scene(tGUID);
}

HRESULT CCore_System::Save_CurrentScene(const std::filesystem::path& path)
{
    return m_pScene_Handler->Save_CurrentScene(path);
}

CScene* CCore_System::Get_CurrentScene()
{
    return m_pScene_Handler->Get_CurrentScene();
}

void CCore_System::Set_CurrentScene(std::unique_ptr<CScene> pScene)
{
    m_pScene_Handler->Set_CurrentScene(std::move(pScene));
}

_bool CCore_System::Restart()
{
    return m_pScene_Handler->Restart();
}

void CCore_System::Set_DebugRender(DEBUG_DRAW eDraw)
{
    CPhysics_Processor* pPhysics = SYS_COMPONENT.Bind_Processor<CPhysics_Processor>();
    IF_NULL_RETURN_MSG_BREAK(pPhysics, , "pPhysics is nullptr");

    pPhysics->Set_DrawMode(eDraw);
}


void CCore_System::Update_RuntimeEngine(_float fDT, CScene* pScene)
{
    /* 씬 업데이트 : 씬의 데이터, 씬 상태(PLAY, STOP 등), 씬 이벤트 등 처리 */
    // pScene->Update(fDT);

    /* 컴포넌트 업데이트 */
    SYS_COMPONENT.Update(fDT);

    /* 게임 오브젝트 Pending 로직 */
    SYS_GAMEOBJECT.Flush_PendingDestroy();
}

NS_END
