#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CTimer;
class CTimerSystem : public CBase
{
private:
	CTimerSystem();
	virtual ~CTimerSystem() = default;
public :
	HRESULT			Ready_System();

public:
	_float			Compute_SystemDT();
	_float			Compute_FrameDT();
	_float			Get_FrameDT() const { return m_fFrameDT; }

private:
	CTimer*			m_pSystemTimer = { nullptr };
	CTimer*			m_pFrameTimer = { nullptr };
	float			m_fFrameDT = {};

public:
	static CTimerSystem* Create();
	virtual void		Free();
};

NS_END