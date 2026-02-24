#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CTimer;
class CTimer_Handler final
{
public:
	CTimer_Handler();
	~CTimer_Handler();
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
	static unique_ptr<CTimer_Handler> Create();
};

NS_END
