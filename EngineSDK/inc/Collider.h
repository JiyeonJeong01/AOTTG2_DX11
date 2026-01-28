#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class CRigidbody;

typedef struct tagCollision
{
	class CObject*		pCounterObject;			// 충돌 당한 오브젝트
	class Collider*		pCounterCollider;		// 충돌 당한 오브젝트의 콜라이더
	class Rigidbody*	pCounterRigidbody;

	_float3		vN;
	_float3		vPoint;
	_float3		vImpulse;
	_float3		vRelativeVel;
	float		fDepth;
}COLLISION;

class Collider : public CComponent
{
protected:
	Collider();
	~Collider() override;

public:
	uint_fast16_t	Get_ColliderID() const				{ return m_iId; }
	void			Set_ColliderID(uint_fast16_t iId)	{ m_iId = iId; }

public:
	// Getter/Setter
	const _float3&	Get_Offset() const					{ return m_vOffset; }
	void			Set_Offset(const _float3& vOffset)	{ m_vOffset = vOffset; }

	float			Get_Scale() const					{ return m_fScale; }
	void			Set_Scale(const float& fScale)		{ m_fScale = fScale; }

	COLLIDER_TYPE	Get_ColType() const					{ return m_eColType; }

protected:
	Rigidbody*		m_pRigidbody;
	uint_fast16_t	m_iId;

	COLLIDER_TYPE	m_eColType;
	_float3			m_vOffset;
	float			m_fScale = 1.f;


public:

private:
	void Free() override;
};

NS_END;