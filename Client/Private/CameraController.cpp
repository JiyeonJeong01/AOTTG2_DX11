#include "CameraController.h"

#include "Easing_Function.h"
#include "GameObject.h"

NS_BEGIN(Client)


void CCameraController::Awake(void* pCtx)
{
    m_fHeight = m_vOffsetToPlayer.y;

    const _float fXZLenSq =
        m_vOffsetToPlayer.x * m_vOffsetToPlayer.x +
        m_vOffsetToPlayer.z * m_vOffsetToPlayer.z;

    m_fDistance = sqrtf(fXZLenSq);

    if (m_fDistance < 0.001f)
        m_fDistance = 0.001f;

    m_fYawDegree = XMConvertToDegrees(atan2f(m_vOffsetToPlayer.x, -m_vOffsetToPlayer.z));
    m_fYawDegree = WrapAngleDeg(m_fYawDegree);
    m_fPitchDegree = XMConvertToDegrees(atan2f(m_vOffsetToPlayer.y, m_fDistance));

    m_fTargetYawDegree = m_fYawDegree;
    m_fTargetPitchDegree = m_fPitchDegree;
}

void CCameraController::Start(void* pCtx)
{
}

void CCameraController::Priority_Update(void* pCtx, _float fDT)
{
}

void CCameraController::Update(void* pCtx, _float fDT)
{
    /* 예외 처리 */
    {
        if (!m_goNormalCam && m_refCamera.Is_Valid())
        {
            m_goNormalCam = SYS_GAMEOBJECT.Get_Wrapper(m_refCamera.hObject);
            if (m_goNormalCam)
                m_trNormalCam = m_goNormalCam->Get_Component<CTransform>();
        }

        if (!m_goTarget && m_refTarget.Is_Valid())
        {
            m_goTarget = SYS_GAMEOBJECT.Get_Wrapper(m_refTarget.hObject);
            if (m_goTarget)
                m_trTarget = m_goTarget->Get_Component<CTransform>();
        }

        if (!m_trNormalCam.Is_Valid() || !m_trTarget.Is_Valid())
            return;

    }

    const _float fYawInputAlpha =
        CEasingFunction::SmoothDampAlpha(m_fInputResponseSharpness, fDT);

    const _float fAppliedYawInput =
        CEasingFunction::Lerp(0.f, m_fYawInputAccum, fYawInputAlpha);
    const _float fAppliedPitchInput =
        CEasingFunction::Lerp(0.f, m_fPitchInputAccum, fYawInputAlpha);

    m_fYawInputAccum -= fAppliedYawInput;
    m_fPitchInputAccum -= fAppliedPitchInput;

    m_fTargetYawDegree += fAppliedYawInput * m_fMouseSensor;
    m_fTargetPitchDegree += fAppliedPitchInput * m_fMouseSensor;

    if (m_fTargetPitchDegree > 75.f)
        m_fTargetPitchDegree = 75.f;
    else if (m_fTargetPitchDegree < -30.f)
        m_fTargetPitchDegree = -30.f;

    if (m_fTargetYawDegree > 360.f || m_fTargetYawDegree < -360.f)
        m_fTargetYawDegree = fmodf(m_fTargetYawDegree, 360.f);

    _float fYawDelta = WrapAngleDeg(m_fTargetYawDegree - m_fYawDegree);
    _float fWrappedTargetYaw = m_fYawDegree + fYawDelta;

    m_fYawDegree = CEasingFunction::DampedLerp(
        m_fYawDegree, fWrappedTargetYaw, m_fYawSharpness, fDT);
    m_fYawDegree = WrapAngleDeg(m_fYawDegree);

    m_fPitchDegree = CEasingFunction::DampedLerp(
        m_fPitchDegree, m_fTargetPitchDegree, m_fPitchSharpness, fDT);
}

void CCameraController::Late_Update(void* pCtx, _float fDT)
{
    if (!m_trNormalCam.Is_Valid() || !m_trTarget.Is_Valid())
        return;

    Follow_Target(fDT);
}

void CCameraController::Follow_Target(_float fDT)
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

    _float3 vDesiredCamPos{};
    vDesiredCamPos.x = vLookTargetPos.x + vOffset.x;
    vDesiredCamPos.y = vLookTargetPos.y + vOffset.y;
    vDesiredCamPos.z = vLookTargetPos.z + vOffset.z;

    if (vDesiredCamPos.y < fLimitY)
        vDesiredCamPos.y = fLimitY;

    if (!m_bCameraInitialized)
    {
        m_trNormalCam.Set_Position(Math::Load(vDesiredCamPos));
        m_vCurrentLookTargetPos = vLookTargetPos;
        m_bCameraInitialized = true;
    }
    else
    {
        _float3 vCurrentCamPos{};
        XMStoreFloat3(&vCurrentCamPos, m_trNormalCam.Get_StateXM(STATE::POSITION));

        vCurrentCamPos.x = CEasingFunction::DampedLerp(
            vCurrentCamPos.x, vDesiredCamPos.x, m_fFollowSharpness, fDT);
        vCurrentCamPos.y = CEasingFunction::DampedLerp(
            vCurrentCamPos.y, vDesiredCamPos.y, m_fFollowSharpness, fDT);
        vCurrentCamPos.z = CEasingFunction::DampedLerp(
            vCurrentCamPos.z, vDesiredCamPos.z, m_fFollowSharpness, fDT);

        m_vCurrentLookTargetPos.x = CEasingFunction::DampedLerp(
            m_vCurrentLookTargetPos.x, vLookTargetPos.x, m_fLookSharpness, fDT);
        m_vCurrentLookTargetPos.y = CEasingFunction::DampedLerp(
            m_vCurrentLookTargetPos.y, vLookTargetPos.y, m_fLookSharpness, fDT);
        m_vCurrentLookTargetPos.z = CEasingFunction::DampedLerp(
            m_vCurrentLookTargetPos.z, vLookTargetPos.z, m_fLookSharpness, fDT);

        m_trNormalCam.Set_Position(Math::Load(vCurrentCamPos));
    }

    m_trNormalCam.Look_At(Math::Load(m_vCurrentLookTargetPos));
}

_float CCameraController::WrapAngleDeg(_float fAngle)
{
    while (fAngle > 180.f)  fAngle -= 360.f;
    while (fAngle < -180.f) fAngle += 360.f;
    return fAngle;
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

void CCameraController::Add_Yaw_Input(_float fDegree)
{
    m_fYawInputAccum += fDegree;
}

void CCameraController::Add_Pitch_Input(_float fDegree)
{
    m_fPitchInputAccum += fDegree;
}

NS_END;
