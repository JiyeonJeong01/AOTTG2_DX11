#pragma once
//#include "Component.h"
//
//NS_BEGIN(Engine)
//
//class ENGINE_DLL CTransform final : public CComponent
//{
//private :
//	CTransform();
//
//public :
//	HRESULT Init() override;
//	void	Start() override;
//	void	Update(float fDT) override;
//	void	Late_Update(float fDT) override;
//
//	void	Translate(const _float3& vDeltaPos)
//	{
//		// m_vPosition += vDeltaPos;
//	}
//	void	Rotate(AXIS_TYPE eAxis, const float& fAngle);
//	void	Rotate(const _float3& vAxis, const float& fDegree);
//
//	// getter/setter
//	void			Set_Scale(const _float3& vScale)				{ m_vScale = vScale; }
//	const			_float3& Get_Scale()							{ return m_vScale; }
//
//	void			Set_Rotation(float fX, float fY, float fZ);
//	void			Set_Rotation(const _float3& vAngle);
//	_float3			Get_RotationAxis(AXIS_TYPE eAxis) const noexcept;
//	const _float4&	Get_RotationQuat()								{ return m_vQuaternion; }
//	_matrix			Get_RotationMat();
//
//	void			Set_Position(float fX, float fY, float fZ)		{ m_vPosition = _float3(fX, fY, fZ); }
//	void			Set_Position(const _float3& vPosition)			{ m_vPosition = vPosition; }
//	const _float3&	Get_Position()									{ return m_vPosition; }
//
//	_float4x4*		Get_WorldMatrix()								{ return &m_matWorld; }
//
//	void Get_Info(AXIS_TYPE eAxis, _float3* pAxis) { memcpy(pAxis, &m_matWorld.m[SCAST(_int, eAxis)][0], sizeof(_float3)); }
//
//
//private:
//	_float3		m_vScale {};
//	_float3		m_vEulerDeg {};
//	_float4		m_vQuaternion {};
//	_float3		m_vPosition {};
//
//	array<_float3, SCAST(size_t, AXIS_TYPE::END)> m_vRotation{};
//	_float4x4	m_matWorld {};
//
//public:
//	static CTransform* Create();
//	void Free() override;
//};
//
//NS_END
