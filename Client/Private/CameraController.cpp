#include "CameraController.h"
#include "GameObject.h"

NS_BEGIN(Client)


void CCameraController::Awake(void* pCtx)
{
    //m_goNormalCam = SYS_GAMEOBJECT.Get_Wrapper(m_refCamera.hObject);
    //IF_NULL_RETURN_MSG_BREAK(m_goNormalCam, , "m_goNormalCam is nullptr");
    //m_trNormalCam = m_goNormalCam->Get_Component<CTransform>();

    //m_goTarget = SYS_GAMEOBJECT.Get_Wrapper(m_refTarget.hObject);
    //IF_NULL_RETURN_MSG_BREAK(m_goTarget, , "m_goTarget is nullptr");
    //m_trTarget = m_goTarget->Get_Component<CTransform>();

    m_fHeight = m_vOffsetToPlayer.y;

    const _float fXZLenSq =
        m_vOffsetToPlayer.x * m_vOffsetToPlayer.x +
        m_vOffsetToPlayer.z * m_vOffsetToPlayer.z;

    m_fDistance = sqrtf(fXZLenSq);

    if (m_fDistance < 0.001f)
        m_fDistance = 0.001f;

    m_fYawDegree = XMConvertToDegrees(atan2f(m_vOffsetToPlayer.x, -m_vOffsetToPlayer.z));
    m_fPitchDegree = XMConvertToDegrees(atan2f(m_vOffsetToPlayer.y, m_fDistance));
}

void CCameraController::Start(void* pCtx)
{
}

void CCameraController::Priority_Update(void* pCtx, _float fDT)
{
}

void CCameraController::Update(void* pCtx, _float fDT)
{
    if (!m_goNormalCam && m_refCamera.Is_Valid())
    {
        m_goNormalCam = SYS_GAMEOBJECT.Get_Wrapper(m_refCamera.hObject);
        if (m_goNormalCam)
            m_trNormalCam = m_goNormalCam->Get_Component<CTransform>();
    }

    if (!m_goTarget && m_refCamera.Is_Valid())
    {
        m_goTarget = SYS_GAMEOBJECT.Get_Wrapper(m_refTarget.hObject);
        if (m_goTarget)
            m_trTarget = m_goTarget->Get_Component<CTransform>();

    }
}

void CCameraController::Late_Update(void* pCtx, _float fDT)
{
    if (!m_trNormalCam.Is_Valid() || !m_trTarget.Is_Valid())
        return;

    Follow_Target();
}

void CCameraController::Follow_Target()
{
    _float3 vTargetPos{};
    XMStoreFloat3(&vTargetPos, m_trTarget.Get_StateXM(STATE::POSITION));

    _float3 vLookTargetPos = vTargetPos;
    vLookTargetPos.y += m_fHeight;

    _float fLimitY = vTargetPos.y - 1.f;

    const _float fYawRad = XMConvertToRadians(m_fYawDegree);
    const _float fPitchRad = XMConvertToRadians(m_fPitchDegree);

    const _float fCosYaw = cosf(fYawRad);
    const _float fSinYaw = sinf(fYawRad);
    const _float fCosPitch = cosf(fPitchRad);
    const _float fSinPitch = sinf(fPitchRad);

    _float3 vOffset{};
    vOffset.x = m_fDistance * fCosPitch * fSinYaw;
    vOffset.y = m_fDistance * fSinPitch;
    vOffset.z = -m_fDistance * fCosPitch * fCosYaw;

    _vector vNewPos = Math::Load(vLookTargetPos) + Math::Load(vOffset);

    vNewPos = XMVectorSet(
        Math::Get_X(vNewPos),
        fmaxf(fLimitY, Math::Get_Y(vNewPos)),
        Math::Get_Z(vNewPos),
        1.f);

    m_trNormalCam.Set_Position(vNewPos);
    m_trNormalCam.Look_At(Math::Load(vLookTargetPos));
}

void CCameraController::Pitch(_float fDegree)
{
    m_fPitchDegree += fDegree * m_fMouseSensor;

    if (m_fPitchDegree > 75.f)
        m_fPitchDegree = 75.f;
    else if (m_fPitchDegree < -30.f)
        m_fPitchDegree = -30.f;
}

void CCameraController::Yaw(_float fDegree)
{
    m_fYawDegree += fDegree * m_fMouseSensor;

    if (m_fYawDegree > 360.f || m_fYawDegree < -360.f)
        m_fYawDegree = fmodf(m_fYawDegree, 360.f);
}

NS_END;
