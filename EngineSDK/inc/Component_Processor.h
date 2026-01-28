  #pragma once
#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CComponent_Processor abstract : public CBase
{
protected :
	CComponent_Processor() {};
	~CComponent_Processor() override = default;

public :
	virtual HRESULT	Init() { return S_OK; };
	virtual void	Update(_float fDT) { };
	virtual void	LateUpdate(_float fDT) { };

	virtual COMPONENT_HANDLE Create_Component_Data() { return COMPONENT_HANDLE{}; }
	virtual void Remove_Component(COMPONENT_HANDLE hHandle) {};

protected :
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	/* TODO 확장 */
	/* DirtyList 혹은 Dirty Flag를 사용하게 할 것인지 고민해보자. */
};

NS_END