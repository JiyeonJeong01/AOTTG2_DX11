#pragma once
#include "Base.h"
#include "Engine_Math.h"

NS_BEGIN(Engine)

class CGameObject;
class CTransform;



class ENGINE_DLL CComponent : public CBase
{
protected :
	CComponent(COMPONENT_TYPE eComType);
	virtual ~CComponent() = default;

public :

	virtual HRESULT	Init();
	virtual void	Render(const _float4x4& matView, const _float4x4& matProj);

	CGameObject*		Get_Owner() const		{ return m_pOwner; }
	CTransform*			Get_Transform() const	{ return m_pTransform; }
	COMPONENT_TYPE		Get_Type() const		{ return m_eComType; }

protected :
	uint32_t m_iDataIndex = { 0 };
	uint32_t Get_DataIndex() const { return m_iDataIndex; }

private :
	CGameObject*		m_pOwner = { nullptr };
	CTransform*			m_pTransform = { nullptr };
	COMPONENT_TYPE		m_eComType = { COMPONENT_TYPE::END };

public :
	virtual void Free();
};

NS_END