#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CTimer;
class CTimer_System final
{
public:
	CTimer_System();
	~CTimer_System();
public :
	HRESULT			Initialize_System();

public:
	_float			Compute_SystemDT();
	_float			Compute_FrameDT();
    _float			Get_FrameDT() const;

private:
	std::unique_ptr<CTimer> m_pSystemTimer{ };
	std::unique_ptr<CTimer> m_pFrameTimer{ };
	_float			        m_fFrameDT{ };

public:
	static unique_ptr<CTimer_System> Create();
};

NS_END
