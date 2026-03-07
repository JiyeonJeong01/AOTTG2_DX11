#include "Rigidbody.h"
#include "GameObject.h"
#include "Transform.h"

void CRigidbody::Set_Shape(SHAPE eShape)
{
    if (!m_pData)
        return;

    if (m_pData->eShape != SHAPE::END)
        return; /* 다른 타입으로 도중에 변경 불가 */

    m_pData->eShape = eShape;
}

SHAPE CRigidbody::Get_Shape() const
{
    if (!m_pData)
        return SHAPE::END;

    return m_pData->eShape;
}

void CRigidbody::Set_BodyType(BODY_TYPE eBodyType)
{
    if (!m_pData)
        return;

    m_pData->eBodyType = eBodyType;
    m_pData->bDirtyMass = true;
    m_pData->bDirtyInertia = true;
    m_pData->bDirtyWorldInertia = true;
}

BODY_TYPE CRigidbody::Get_BodyType() const
{
    if (!m_pData)
        return BODY_TYPE::STATIC;

    return m_pData->eBodyType;
}

void CRigidbody::Set_Mass(_float fMass)
{
    if (!m_pData)
        return;

    if (fMass < 0.f)
        fMass = 0.f;

    m_pData->fMass = fMass;
}

_float CRigidbody::Get_Mass() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fMass;
}

void CRigidbody::Set_Drag(_float fDrag)
{
    if (!m_pData)
        return;

    if (fDrag < 0.f)
        fDrag = 0.f;

    m_pData->fDrag = fDrag;
}

_float CRigidbody::Get_Drag() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fDrag;
}

void CRigidbody::Set_AngularDrag(_float fAngularDrag)
{
    if (!m_pData)
        return;

    if (fAngularDrag < 0.f)
        fAngularDrag = 0.f;

    m_pData->fAngularDrag = fAngularDrag;
}

_float CRigidbody::Get_AngularDrag() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fAngularDrag;
}

void CRigidbody::Set_Restitution(_float fRestitution)
{
    if (!m_pData)
        return;

    if (fRestitution < 0.f)
        fRestitution = 0.f;

    m_pData->fRestitution = fRestitution;
}

_float CRigidbody::Get_Restitution() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fRestitution;
}

void CRigidbody::Set_Friction(_float fFriction)
{
    if (!m_pData)
        return;

    if (fFriction < 0.f)
        fFriction = 0.f;

    m_pData->fFriction = fFriction;
}

_float CRigidbody::Get_Friction() const
{
    if (!m_pData)
        return 0.f;

    return m_pData->fFriction;
}

void CRigidbody::Set_Gravity(_bool bGravity)
{
    if (!m_pData)
        return;

    m_pData->bGravity = bGravity;
}

_bool CRigidbody::Get_Gravity() const
{
    if (!m_pData)
        return false;

    return m_pData->bGravity;
}

void CRigidbody::Set_COM(const _float3& vCOM)
{
    if (!m_pData)
        return;

    m_pData->vCOM = vCOM;
}

_float3 CRigidbody::Get_COM() const
{
    if (!m_pData)
        return _float3{ 0.f, 0.f, 0.f };

    return m_pData->vCOM;
}

void CRigidbody::Set_LinearVel(const _float3& vLinearVel)
{
    if (!m_pData)
        return;

    if (m_pData->eBodyType != BODY_TYPE::DYNAMIC)
        return;

    m_pData->vLinearVel = vLinearVel;
}

_float3 CRigidbody::Get_LinearVel() const
{
    if (!m_pData)
        return _float3{ 0.f, 0.f, 0.f };

    return m_pData->vLinearVel;
}

void CRigidbody::Set_AngularVel(const _float3& vAngularVel)
{
    if (!m_pData)
        return;

    if (m_pData->eBodyType != BODY_TYPE::DYNAMIC)
        return;

    m_pData->vAngularVel = vAngularVel;
}

_float3 CRigidbody::Get_AngularVel() const
{
    if (!m_pData)
        return _float3{ 0.f, 0.f, 0.f };

    return m_pData->vAngularVel;
}

void CRigidbody::Set_RotationLock(const AXIS_MASK& tRotationLock)
{
    if (!m_pData)
        return;

    m_pData->tRotationLock = tRotationLock;
}

AXIS_MASK CRigidbody::Get_RotationLock() const
{
    if (!m_pData)
        return AXIS_MASK{};

    return m_pData->tRotationLock;
}

void CRigidbody::Set_PositionLock(const AXIS_MASK& tPositionLock)
{
    if (!m_pData)
        return;

    m_pData->tPositionLock = tPositionLock;
}

AXIS_MASK CRigidbody::Get_PositionLock() const
{
    if (!m_pData)
        return AXIS_MASK{};

    return m_pData->tPositionLock;
}

void CRigidbody::Translate(const _float3& vDeltaPos)
{
    if (!m_pData || !Find_Transform())
        return;

    /* STATIC BODY는 절대 움직이지 않는다 */
    if (m_pData->eBodyType == BODY_TYPE::STATIC)
        return;

    const _vector vPosition = Math::Load(m_pTrData->vPosition);
    const _vector vDelta = Math::Load(vDeltaPos);
    Math::Store(m_pTrData->vPosition, vPosition + vDelta);

    const _vector vCOM = Math::Load(m_pData->vCOM);
    Math::Store(m_pData->vCOM, vCOM + vDelta);
    Math::Store(m_pData->vWorldCOM, Math::Load(m_pTrData->vPosition));

    m_pData->bDirtyWorldInertia = true;
}

void CRigidbody::Add_LinearImpulse(const _float3& vImpulse)
{
    if (!m_pData || !Find_Transform())
        return;

    if (m_pData->eBodyType != BODY_TYPE::DYNAMIC)
        return;

    const _vector vLinearVel = Math::Load(m_pData->vLinearVel);
    const _vector vImpulseVec = Math::Load(vImpulse);

    Math::Store(m_pData->vLinearVel, vLinearVel + vImpulseVec * m_pData->fInvMass);
}

void CRigidbody::Add_Force(const _float3& vForce)
{
    if (!m_pData || !Find_Transform())
        return;

    if (m_pData->eBodyType != BODY_TYPE::DYNAMIC)
        return;

    const _vector vForceAccum = Math::Load(m_pData->vForceAccum);
    const _vector vForceVec = Math::Load(vForce);

    Math::Store(m_pData->vForceAccum, vForceAccum + vForceVec);
}

void CRigidbody::Add_Torque(const _float3& vTorque)
{
    if (!m_pData || !Find_Transform())
        return;

    if (m_pData->eBodyType != BODY_TYPE::DYNAMIC)
        return;

    const _vector vTorqueAccum = Math::Load(m_pData->vTorqueAccum);
    const _vector vTorqueVec = Math::Load(vTorque);

    Math::Store(m_pData->vTorqueAccum, vTorqueAccum + vTorqueVec);
}

TRANSFORM_DATA* CRigidbody::Find_Transform()
{
    if (m_pTrData) return m_pTrData;

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(m_pData->hObject);
    IF_NULL_RETURN_MSG_BREAK(pObj, nullptr, "pObj is nullptr");

    m_pTrData = pObj->Get_Component<CTransform>()._Data();
    IF_NULL_RETURN_MSG_BREAK(m_pTrData, nullptr, "m_pTrData is nullptr");

    return m_pTrData;
}
