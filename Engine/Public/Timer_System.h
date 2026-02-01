#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CTimer;
class CTimer_System : public CBase
{
private:
	CTimer_System();
	virtual ~CTimer_System() = default;
public :
	HRESULT			Initialize_System();

public:
	_float			Compute_SystemDT();
	_float			Compute_FrameDT();
    _float			Get_FrameDT() const;

private:
	CTimer*			m_pSystemTimer = { nullptr };
	CTimer*			m_pFrameTimer = { nullptr };
	float			m_fFrameDT = {};

public:
	static CTimer_System* Create();
	virtual void		Free();
};

NS_END
