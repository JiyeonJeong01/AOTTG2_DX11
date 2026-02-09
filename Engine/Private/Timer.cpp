#include "Timer.h"

#include "Engine_Log.h"

NS_BEGIN(Engine)
    CTimer::CTimer()
	: m_fTimeDelta(0.f)
{
	ZeroMemory(&m_LastTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_FrameTime, sizeof(LARGE_INTEGER));
	ZeroMemory(&m_CpuTick, sizeof(LARGE_INTEGER));
}

CTimer::~CTimer()
{

}

HRESULT CTimer::Initialize()
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

std::unique_ptr<CTimer> CTimer::Create()
{
	auto pInstance = std::make_unique<CTimer>();

	if (FAILED(pInstance->Initialize()))
	{
        _DEBUG_ERROR_BREAK("Create instance failed");
		return nullptr;
	}

	return pInstance;
}

NS_END
