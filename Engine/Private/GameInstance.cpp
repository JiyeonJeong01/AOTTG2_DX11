#include "GameInstance.h"

#include "Graphic_Device.h"

#include "GameObject_System.h"
#include "Component_System.h"

#include "TimerSystem.h"
#include "Logger.h"

IMPLEMENT_SINGLETON(CGameInstance)

CGameInstance::CGameInstance()
{

}

HRESULT CGameInstance::Initialize_Engine(const ENGINE_DESC& EngineDesc, ID3D11Device** ppDevice,
	ID3D11DeviceContext** ppContext)
{
	m_pGraphic_Device = CGraphic_Device::Create(EngineDesc.hWnd,
		EngineDesc.eWinMode,
		EngineDesc.iViewportSize.first,
		EngineDesc.iViewportSize.second,
		ppDevice,
		ppContext);
	if (nullptr == m_pGraphic_Device)
		return E_FAIL;

	m_pTimerSystem = CTimerSystem::Create();
	if (nullptr == m_pTimerSystem)
		return E_FAIL;


	if(FAILED(SYS_COM->Init()))
	{
		MSG_BOX("Failed Component System Initialize");
		return E_FAIL;
	}

	SYS_LOG->Ready_Logger();

	return S_OK;
}

void CGameInstance::Update_Engine(_float fTimeDelta)
{

	SYS_COM->Update(fTimeDelta);
}

HRESULT CGameInstance::Draw()
{
    return S_OK;
}

void CGameInstance::Clear_Resources(_uint iLevelIndex)
{
}

HRESULT CGameInstance::Clear_Buffers(const _float4* pClearColor) const
{
	if (FAILED(m_pGraphic_Device->Clear_BackBuffer_View(pClearColor)))
		return E_FAIL;

	if (FAILED(m_pGraphic_Device->Clear_DepthStencil_View()))
		return E_FAIL;

	return S_OK;
}

HRESULT CGameInstance::Present() const
{
	return m_pGraphic_Device->Present();
}

_float CGameInstance::Compute_SystemDT() const
{
	return m_pTimerSystem->Compute_SystemDT();
}
_float CGameInstance::Compute_FrameDT() const
{
	return m_pTimerSystem->Compute_FrameDT();
}

HRESULT CGameInstance::Change_Scene(_uint iNewLevelIndex, CLevel* pNewLevel)
{
    return S_OK;
}

void CGameInstance::Free()
{
	__super::Free();
}
