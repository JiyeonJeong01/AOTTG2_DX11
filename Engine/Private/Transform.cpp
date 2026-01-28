//#include "Transform.h"
//#include "Engine_Log.h"
//
//CTransform::CTransform()
//	: CComponent(COMPONENT_TYPE::TRANSFORM)
//{
//}
//
//HRESULT CTransform::Init()
//{
//	CComponent::Init();
//
//    return S_OK;
//}
//
//void CTransform::Start()
//{
//	CComponent::Start();
//
//	const _matrix matW = Math::Load(m_matWorld);
//
//	Math::Set_IdentityM(m_matWorld);
//	for (int i = 0; i < SCAST(_int, AXIS_TYPE::END); ++i)
//		Math::Store(m_vRotation[i], matW.r[i]);
//}
//
//void CTransform::Update(float fDT)
//{
//    Math::Set_IdentityM(m_matWorld);
//
//    // Rotation (Quaternion -> Matrix)
//    const _vector q = XMVectorSet(m_vQuaternion.x, m_vQuaternion.y, m_vQuaternion.z, m_vQuaternion.w);
//    const _matrix R = Math::RotationQuaternionM(q);
//
//    for (int i = 0; i < SCAST(_int, AXIS_TYPE::END); ++i)
//        Math::Store(m_vRotation[i], R.r[i]);
//
//    // Scale 
//    for (int i = 0; i < SCAST(_int, AXIS_TYPE::END); ++i)
//    {
//        const float scale = (i == 0 ? m_vScale.x : i == 1 ? m_vScale.y : m_vScale.z);
//
//        m_vRotation[i].x *= scale;
//        m_vRotation[i].y *= scale;
//        m_vRotation[i].z *= scale;
//    }
//
//    // World compose
//    _matrix W = Math::Load(m_matWorld);
//
//    for (int i = 0; i < SCAST(_int, AXIS_TYPE::END); ++i)
//        W.r[i] = XMVectorSet(m_vRotation[i].x, m_vRotation[i].y, m_vRotation[i].z, 0.f);
//
//    W.r[3] = XMVectorSet(m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.f);
//
//    Math::Store(m_matWorld, W);
//}
//
//void CTransform::Late_Update(float fDT)
//{
//	CComponent::Late_Update(fDT);
//}
//
//void CTransform::Rotate(AXIS_TYPE eAxis, const float& fDegree)
//{
//    _float3 axis{};
//
//    switch (eAxis)
//    {
//    case AXIS_TYPE::X: axis = { 1.f, 0.f, 0.f }; break;
//    case AXIS_TYPE::Y: axis = { 0.f, 1.f, 0.f }; break;
//    case AXIS_TYPE::Z: axis = { 0.f, 0.f, 1.f }; break;
//    default: return;
//    }
//
//    Rotate(axis, fDegree);
//}
//
//void CTransform::Rotate(const _float3& vAxis, const float& fDegree)
//{
//    const float rad = XMConvertToRadians(fDegree);
//
//    const _vector axis = Math::Load(vAxis);
//
//    _vector qCur = XMVectorSet(m_vQuaternion.x, m_vQuaternion.y, m_vQuaternion.z, m_vQuaternion.w);
//	const _vector qDelta = XMQuaternionRotationAxis(axis, rad);
//
//    // qCur = qCur * qDelta
//    qCur = XMQuaternionMultiply(qCur, qDelta);
//	qCur = XMQuaternionNormalize(qCur);
//
//    Math::Store(m_vQuaternion, qCur);
//
//    // TODO : Quaternion to Euler
//    m_vEulerDeg = Math::QuaternionToEulerDeg(qCur);
//}
//
//void CTransform::Set_Rotation(float fX, float fY, float fZ)
//{
//    m_vEulerDeg = { fX, fY, fZ };         // degree
//    m_vQuaternion = Math::EulerDegToQuaternion(m_vEulerDeg);
//}
//
//void CTransform::Set_Rotation(const _float3& vAngle)
//{
//    m_vEulerDeg = vAngle;                  // degree
//    m_vQuaternion = Math::EulerDegToQuaternion(m_vEulerDeg);
//}
//
//_float3 CTransform::Get_RotationAxis(AXIS_TYPE eAxis) const noexcept
//{
//    // 저장용 quaternion -> 연산용
//    const _vector q = Math::Load(m_vQuaternion);
//
//    // 회전 행렬 생성
//    const _matrix R = Math::RotationQuaternionM(q);
//
//    switch (eAxis)
//    {
//    case AXIS_TYPE::X: // Right
//        return Math::Store(XMVector3Normalize(R.r[0]));
//
//    case AXIS_TYPE::Y: // Up
//        return Math::Store(XMVector3Normalize(R.r[1]));
//
//    case AXIS_TYPE::Z: // Look
//        return Math::Store(XMVector3Normalize(R.r[2]));
//
//    default:
//        return Math::Zero3();
//    }
//}
//
//_matrix CTransform::Get_RotationMat()
//{
//    _vector q = Math::Load(m_vQuaternion);
//
//    q = XMQuaternionNormalize(q);
//
//    return Math::RotationQuaternionM(q);
//}
//
//CTransform* CTransform::Create()
//{
//    CTransform* pInstance = new CTransform();
//    if (FAILED(pInstance->Init()))
//    {
//        Safe_Release(pInstance);
//        ERROR_BREAK("Create Transform Failed");
//    }
//    return pInstance;
//}
//
//void CTransform::Free()
//{
//	CComponent::Free();
//}
