#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTimer : public CBase
{
private:
	CTimer();
	virtual ~CTimer() = default;


public:
	HRESULT		Ready_Timer();
	_float		Update_Timer();

private:
	LARGE_INTEGER		m_FrameTime = {};
	LARGE_INTEGER		m_LastTime = {};
	LARGE_INTEGER		m_CpuTick = {};

	_float				m_fTimeDelta = {};

public:
	static CTimer* Create();

public:
	virtual void	Free();

};

NS_END
