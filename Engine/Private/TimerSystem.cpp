#include "TimerSystem.h"

#include "Engine_Log.h"
#include "Timer.h"

CTimerSystem::CTimerSystem()
{

}

HRESULT CTimerSystem::Ready_System()
{
	m_pSystemTimer = CTimer::Create();
	m_pFrameTimer = CTimer::Create();

	return S_OK;
}

_float CTimerSystem::Compute_SystemDT()
{
	if (nullptr == m_pSystemTimer)
		return 0.f;

	return m_pSystemTimer->Update_Timer();
}

_float CTimerSystem::Compute_FrameDT()
{
	if (nullptr == m_pFrameTimer)
		return 0.f;

	m_fFrameDT = m_pFrameTimer->Update_Timer();
	return m_fFrameDT;
}

CTimerSystem* CTimerSystem::Create()
{
	CTimerSystem* pInstance = new CTimerSystem;
	if (FAILED(pInstance->Ready_System()))
	{
		Safe_Release(pInstance);
		ENGINE_LOG_ERROR("CTimerSystem Create Failed");
	}
	return pInstance;
}

void CTimerSystem::Free()
{
	__super::Free();

	Safe_Release(m_pSystemTimer);
	Safe_Release(m_pFrameTimer);
}
