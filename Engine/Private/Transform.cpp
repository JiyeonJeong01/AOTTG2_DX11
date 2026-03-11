#include "Transform.h"
#include "Engine_MathDX.h"
#include "Engine_Log.h"
#include "Engine_Math.h"

void CTransform::Translate(_fvector vWorldDir, SPACE eSpace)
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr.");

    _vector vMoveDir = vWorldDir;

    if (eSpace == SPACE::LOCAL)
    {
        /* Convert local direction to world direction */
        _vector vCurrentQuat = MathDX::Load(m_pData->vRotationQuat);
        vMoveDir = XMVector3Rotate(vMoveDir, vCurrentQuat);
    }

    if (XMVectorGetX(XMVector3LengthSq(vMoveDir)) < s_EPS)
        return;

    _vector vPos = MathDX::Load(m_pData->vPosition);
    vPos = XMVectorAdd(vPos, vMoveDir);
    MathDX::Store(m_pData->vPosition, vPos);

    m_pData->bDirty = true;
}

void CTransform::Rotate(_fvector vWorldAxis, _float fDegree, SPACE eSpace)
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr.");

    const _float fRadian = XMConvertToRadians(fDegree);

    _vector vCurrentQuat = MathDX::Load(m_pData->vRotationQuat);

    _vector vRotAxis = vWorldAxis;
    if (eSpace == SPACE::LOCAL)
        vRotAxis = XMVector3Rotate(vRotAxis, vCurrentQuat);

    if (XMVectorGetX(XMVector3LengthSq(vRotAxis)) < s_EPS)
        return;

    vRotAxis = XMVector3Normalize(vRotAxis);

    /* Define delta rotation in WORLD space */
    const _vector vDeltaQuat = XMQuaternionRotationAxis(vRotAxis, fRadian);

    vCurrentQuat = XMQuaternionMultiply(vDeltaQuat, vCurrentQuat); /* cq -> dq */

    MathDX::Store(m_pData->vRotationQuat, XMQuaternionNormalize(vCurrentQuat));

    m_pData->bDirty = true;
}

void CTransform::Scale(const _float3& vLocalDelta)
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr.");

    m_pData->vScale.x *= vLocalDelta.x;
    m_pData->vScale.y *= vLocalDelta.y;
    m_pData->vScale.z *= vLocalDelta.z;

    m_pData->bDirty = true;
}

void CTransform::Set_Rotation_Euler(_float3 vEulerDegree)
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr.");

    const _float fPitch = XMConvertToRadians(vEulerDegree.x);
    const _float fYaw = XMConvertToRadians(vEulerDegree.y);
    const _float fRoll = XMConvertToRadians(vEulerDegree.z);

    /* Converts Euler angles(p, y, r) into a quaternion format. */
    const _vector vQuat = XMQuaternionNormalize(XMQuaternionRotationRollPitchYaw(fPitch, fYaw, fRoll));
    MathDX::Store(m_pData->vRotationQuat, vQuat);

    m_pData->bDirty = true;
}

void CTransform::Look_At(_fvector vTargetPos)
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr.");

    const _vector vPos = MathDX::Load(m_pData->vPosition);

    _vector vLook = XMVectorSubtract(vTargetPos, vPos);
    if (XMVectorGetX(XMVector3LengthSq(vLook)) < s_EPS)
        return;

    vLook = XMVector3Normalize(vLook);

    /* Check if the Look Vector is parallel to the Up Vector. */
    _vector vTmpUp = MathDX::World_Up();
    const _float fDot1 = fabsf(XMVectorGetX(XMVector3Dot(vLook, XMVector3Normalize(vTmpUp))));
    if (fDot1 > 0.999f)
    {
        vTmpUp = MathDX::World_Look();
        const _float fDot2 = fabsf(XMVectorGetX(XMVector3Dot(vLook, XMVector3Normalize(vTmpUp))));
        if (fDot2 > 0.999f)
            vTmpUp = MathDX::World_Right();
    }

    _vector vRight = XMVector3Normalize(XMVector3Cross(vTmpUp, vLook));
    _vector vUp = XMVector3Normalize(XMVector3Cross(vLook, vRight));

    /* Build a pure rotation matrix */
    _matrix matRot = XMMatrixIdentity();
    matRot.r[0] = XMVectorSetW(vRight, 0.f);
    matRot.r[1] = XMVectorSetW(vUp, 0.f);
    matRot.r[2] = XMVectorSetW(vLook, 0.f);

    /* Convert rotation matrix to quaternion */
    const _vector vQuat = XMQuaternionNormalize(XMQuaternionRotationMatrix(matRot));
    MathDX::Store(m_pData->vRotationQuat, vQuat);

    m_pData->bDirty = true;
}

_float3 CTransform::Get_Rotation_Euler() const
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, {}, "m_pData is nullptr.");

    const _vector vQuat = XMQuaternionNormalize(MathDX::Load(m_pData->vRotationQuat));
    _float4 fQuat{};
    MathDX::Store(fQuat, vQuat);

    const _float fX = fQuat.x;
    const _float fY = fQuat.y;
    const _float fZ = fQuat.z;
    const _float fW = fQuat.w;

    _float fSnip = 2.f * (fW * fX - fZ * fY);
    fSnip = min(1.f, max(-1.f, fSnip));
    _float fPitch = asinf(fSnip);

    _float fSinyCosp = 2.f * (fW * fY + fX * fZ);
    _float fCosyCosp = 1.f - 2.f * (fX * fX + fY * fY);
    _float fYaw = atan2f(fSinyCosp, fCosyCosp);

    _float fSinrCosp = 2.f * (fW * fZ + fX * fY);
    _float fCosrCosp = 1.f - 2.f * (fY * fY + fZ * fZ);
    _float fRoll = atan2f(fSinrCosp, fCosrCosp);

    _float3 out{};
    out.x = XMConvertToDegrees(fPitch);
    out.y = XMConvertToDegrees(fYaw);
    out.z = XMConvertToDegrees(fRoll);

    return out;
}

_float4 CTransform::Get_Rotation_Quaternion() const
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, _float4{}, "m_pData is nullptr.");

    return m_pData->vRotationQuat;
}

_matrix CTransform::Get_WorldXM() const
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, {}, "m_pData is nullptr.");

    return MathDX::Load(m_pData->matWorld);
}


_vector CTransform::Get_StateXM(STATE eState) const
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, {}, "m_pData is nullptr.");

    return MathDX::Load(reinterpret_cast<const _float4*>(&m_pData->matWorld.m[SCAST(_uint, eState)][0]));
}

void CTransform::Set_Position(_fvector vPosition)
{
    Math::Store(m_pData->vPosition, vPosition);
    m_pData->bDirty = true;
}

_float3 CTransform::Get_Scale() const
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, _float3(1.f, 1.f, 1.f), "m_pData is nullptr.");

    return m_pData->vScale;
}

void CTransform::Set_Scale(const _float3& vScale)
{
    if (fabsf(vScale.x) < s_EPS || fabsf(vScale.y) < s_EPS || fabsf(vScale.z) < s_EPS)
        return;

    m_pData->vScale = vScale;
    m_pData->bDirty = true;
}

void CTransform::Set_Identity()
{
    IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr.");

    MathDX::Store(m_pData->vPosition, XMVectorZero());
    MathDX::Store(m_pData->vRotationQuat, XMQuaternionIdentity());
    m_pData->vScale = _float3(1.f, 1.f, 1.f);

    m_pData->bDirty = true;
}
