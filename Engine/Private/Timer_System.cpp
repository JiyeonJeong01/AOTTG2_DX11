#include "Timer_System.h"

#include "Engine_Log.h"
#include "Timer.h"

NS_BEGIN(Engine)

CTimer_System::CTimer_System()
{

}

CTimer_System::~CTimer_System()
{
}

HRESULT CTimer_System::Initialize_System()
{
	m_pSystemTimer = CTimer::Create();
	m_pFrameTimer = CTimer::Create();

	return S_OK;
}

_float CTimer_System::Compute_SystemDT()
{
	if (nullptr == m_pSystemTimer)
		return 0.f;

	return m_pSystemTimer->Update_Timer();
}

_float CTimer_System::Compute_FrameDT()
{
	if (nullptr == m_pFrameTimer)
		return 0.f;

	m_fFrameDT = m_pFrameTimer->Update_Timer();
	return m_fFrameDT;
}

_float CTimer_System::Get_FrameDT() const
{
    return m_fFrameDT;
}


std::unique_ptr<CTimer_System> CTimer_System::Create()
{
    auto pInstance = make_unique<CTimer_System>();
	if (FAILED(pInstance->Initialize_System()))
	{
        _DEBUG_ERROR_BREAK("CTimer_System Create Failed");
        return nullptr;
	}
	return pInstance;
}


NS_END
