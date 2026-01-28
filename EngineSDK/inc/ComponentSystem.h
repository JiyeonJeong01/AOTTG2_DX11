#pragma once
#include "Base.h"
#include "ComponentPool.h"

NS_BEGIN(Engine)
	class ENGINE_DLL CComponentSystem abstract : public CBase
{
protected :
	CComponentSystem() {};
	~CComponentSystem() override = default;

public :
	//virtual HRESULT	Init() = 0;
	virtual void	Update(_float fDT) = 0;
	//virtual void	Late_Update(_float fDT) = 0;

	virtual CComponent*	Create_Component(COMPONENT_HANDLE& outHandle) = 0;
	//virtual void		Remove_Component(COMPONENT_HANDLE hHandle) = 0;

protected :
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	/* TODO 확장 */
	/* DirtyList 혹은 Dirty Flag를 사용하게 할 것인지 고민해보자. */
};

NS_END