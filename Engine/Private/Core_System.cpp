#include "Core_System.h"

/* --- main --- */
#include "GameObject_System.h"
#include "Component_System.h"
#include "Input_System.h"
#include "Logger.h"
#include "Asset_Registry.h"
#include "Resource_System.h"
#include "Event_System.h"

/* --- sub --- */
#include "Graphic_Device.h"
#include "Timer_System.h"

/* --- event --- */
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
    SYS_LOG.DestroyInstance();
    SYS_INPUT.DestroyInstance();
    SYS_EVENT.DestroyInstance();
}

HRESULT CCore_System::Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice,
	ID3D11DeviceContext** ppContext)
{
    /* --- Device --- */
    {
        m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd,
           EngineDesc.eWinMode,
           EngineDesc.iViewportSize.first,
           EngineDesc.iViewportSize.second,
           ppDevice,
           ppContext);
        if (nullptr == m_pGraphic_Device)
            return E_FAIL;
    }

    m_pDevice = *ppDevice;
    m_pContext = *ppContext;

    /* --- Timer ---*/
    {
        m_pTimerSystem = CTimer_System::Create();
        if (nullptr == m_pTimerSystem)
            return E_FAIL;
    }

    /* --- Component System ---*/
    {
        if (FAILED(SYS_COMPONENT.Initialize(m_pDevice, m_pContext)))
        {
            MSG_BOX("Component System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Object System --- */
    {
        if (FAILED(SYS_GAMEOBJECT.Initialize()))
        {
            MSG_BOX("Object System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Log System --- */
    {
        if (FAILED(SYS_LOG.Initialize()))
        {
            MSG_BOX("Log System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Asset System --- */
    {
        if (FAILED(SYS_ASSET.Initialize(ProjectConfig::PATH + ProjectConfig::ROOT)))
        {
            MSG_BOX("Resource System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Resource System --- */
    {
        if (FAILED(SYS_RESOURCE.Initialize(*ppDevice, *ppContext)))
        {
            MSG_BOX("Resource System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Input System --- */
    {
        if (FAILED(SYS_INPUT.Initialize(EngineDesc.hWnd, EngineDesc.hInst)))
        {
            MSG_BOX("Input System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Event System --- */
    {
        if (FAILED(SYS_EVENT.Initialize()))
        {
            MSG_BOX("Event System failed Initialize");
            return E_FAIL;
        }
    }



    /* Register event */
    SYS_EVENT.Subscribe(EVENT_TYPE::On_Window_Resize, &CCore_System::On_Resize, this);

	return S_OK;
}

void CCore_System::Update_Engine(_float fTimeDelta)
{
	SYS_COMPONENT.Update(fTimeDelta);
}

HRESULT CCore_System::Draw()
{
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
	return m_pTimerSystem->Compute_SystemDT();
}
_float CCore_System::Compute_FrameDT() const
{
	return m_pTimerSystem->Compute_FrameDT();
}

HRESULT CCore_System::Change_Scene(_uint iNewLevelIndex, CLevel* pNewLevel)
{
    return S_OK;
}

NS_END
