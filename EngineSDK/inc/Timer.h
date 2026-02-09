#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTimer
{
public:
	CTimer();
	~CTimer();

public:
	HRESULT		Initialize();
	_float		Update_Timer();

private:
	LARGE_INTEGER		m_FrameTime = {};
	LARGE_INTEGER		m_LastTime = {};
	LARGE_INTEGER		m_CpuTick = {};

	_float				m_fTimeDelta = {};

public:
	static std::unique_ptr<CTimer> Create();

};

NS_END
