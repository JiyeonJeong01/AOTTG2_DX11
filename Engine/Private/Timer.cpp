#include "Timer.h"

CTimer::CTimer()
	: m_fTimeDelta(0.f)
{
	ZeroMemory(&m_LastTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_FrameTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_CpuTick, sizeof(LARGE_INTEGER));
}

HRESULT CTimer::Ready_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);	
	QueryPerformanceCounter(&m_LastTime);	

	QueryPerformanceFrequency(&m_CpuTick);	

	return S_OK;
}

_float CTimer::Update_Timer()
{
	QueryPerformanceCounter(&m_FrameTime);			

	m_fTimeDelta = (m_FrameTime.QuadPart - m_LastTime.QuadPart)
		/ static_cast<_float>(m_CpuTick.QuadPart);

	m_LastTime = m_FrameTime;

	return m_fTimeDelta;
}

CTimer* CTimer::Create()
{
	CTimer* pInstance = new CTimer;

	if (FAILED(pInstance->Ready_Timer()))
	{
		Engine::Safe_Release(pInstance);
		return nullptr;
	}

	return pInstance;
}

void CTimer::Free()
{
	__super::Free();

}

