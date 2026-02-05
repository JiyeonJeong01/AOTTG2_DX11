#include "Core_System.h"

#include "Graphic_Device.h"

#include "GameObject_System.h"
#include "Component_System.h"

#include "Timer_System.h"
#include "Logger.h"
#include "Asset_Registry.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CCore_System)

CCore_System::CCore_System()
{

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

    /* --- Timer ---*/
    {
        m_pTimerSystem = CTimer_System::Create();
        if (nullptr == m_pTimerSystem)
            return E_FAIL;
    }

    /* --- Component System ---*/
    {
        if (FAILED(SYS_COMPONENT->Initialize()))
        {
            MSG_BOX("Component System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Object System --- */
    {
        if (FAILED(SYS_GAMEOBJECT->Initialize()))
        {
            MSG_BOX("Object System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Log System --- */
    {
        if (FAILED(SYS_LOG->Initialize()))
        {
            MSG_BOX("Log System failed Initialize");
            return E_FAIL;
        }
    }

    /* --- Resource System --- */
    {
        if (FAILED(SYS_RESOURCE->Initialize(ProjectConfig::PATH + ProjectConfig::ROOT)))
        {
            MSG_BOX("Resource System failed Initialize");
            return E_FAIL;
        }
    }

    m_pDevice = *ppDevice;
    m_pContext = *ppContext;

	return S_OK;
}

void CCore_System::Update_Engine(_float fTimeDelta)
{
	SYS_COMPONENT->Update(fTimeDelta);
}

HRESULT CCore_System::Draw()
{
    return S_OK;
}

void CCore_System::Clear_Resources(_uint iLevelIndex)
{
}

void CCore_System::Share_GraphicDevice(ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
    *ppDevice = m_pDevice;
    *ppContext = m_pContext;
}

HRESULT CCore_System::Clear_Buffers(const _float4* pClearColor) const
{
	if (FAILED(m_pGraphic_Device->Clear_BackBuffer_View(pClearColor)))
		return E_FAIL;

	if (FAILED(m_pGraphic_Device->Clear_DepthStencil_View()))
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

void CCore_System::Free()
{
	__super::Free();
}

NS_END
