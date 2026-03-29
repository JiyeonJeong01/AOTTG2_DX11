#include "PlayerState.h"
#include "CameraController.h"
#include "Easing_Function.h"

HRESULT CPlayerState::Initialize()
{


    return S_OK;
}

void CPlayerState::Priority_Update(_float fDT)
{


}

void CPlayerState::Update(_float fDT)
{
}

void CPlayerState::Late_Update(_float fDT)
{
}

void CPlayerState::Control_Camera()
{
    if (m_tRef.pCameraController)
    {
        m_tRef.pCameraController->Add_Yaw_Input(m_tInputCmd.vMouseDelta.x);
        m_tRef.pCameraController->Add_Pitch_Input(m_tInputCmd.vMouseDelta.y);
    }
}

void CPlayerState::Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd)
{
    m_tInputCmd = tInputCmd;
}

void CPlayerState::Setup_CachedPlayerInfos()
{

}

void CPlayerState::LookTo_InputDir(_float fDT)
{
    if (!m_bYawInitialized)
    {
        _vector vInitLook = m_tComponents.transform.Get_StateXM(STATE::LOOK);
        vInitLook = XMVectorSetY(vInitLook, 0.f);

        const _float fEps = 1e-4f;
        if (XMVectorGetX(XMVector3LengthSq(vInitLook)) < fEps)
            vInitLook = XMVectorSet(0.f, 0.f, -1.f, 0.f);
        else
            vInitLook = XMVector3Normalize(vInitLook);

        m_fCurrentYaw = atan2f(XMVectorGetX(vInitLook), XMVectorGetZ(vInitLook));
        m_bYawInitialized = true;
    }

    _vector vTargetLook = XMLoadFloat3(&m_tInputCmd.vLook);
    vTargetLook *= -1.f;
    vTargetLook = XMVectorSetY(vTargetLook, 0.f);

    const _float fEps = 1e-4f;
    if (XMVectorGetX(XMVector3LengthSq(vTargetLook)) < fEps)
        return;

    vTargetLook = XMVector3Normalize(vTargetLook);

    _float fTargetYaw = atan2f(XMVectorGetX(vTargetLook), XMVectorGetZ(vTargetLook));

    _float fDeltaYaw = fTargetYaw - m_fCurrentYaw;
    while (fDeltaYaw > XM_PI)  fDeltaYaw -= XM_2PI;
    while (fDeltaYaw < -XM_PI) fDeltaYaw += XM_2PI;

    const _float fT = 1.f - expf(-m_fRotateSharpness * fDT);
    _float fNewYaw = m_fCurrentYaw + fDeltaYaw * fT;

    m_fCurrentYaw = fNewYaw;

    _vector qRot = XMQuaternionRotationRollPitchYaw(0.f, fNewYaw, 0.f);
    m_tComponents.transform.Set_Rotation_Quaternion(qRot);
}

//void CPlayerState::LookTo_InputDir(_float fDT)
//{
//    /* 목표 방향 보정 */
//    _vector vTargetLook = XMLoadFloat3(&m_tInputCmd.vLook);
//    vTargetLook *= -1.f;
//    vTargetLook = XMVectorSetY(vTargetLook, 0.f);
//
//    const _float fEps = 1e-4f;
//    if (XMVectorGetX(XMVector3LengthSq(vTargetLook)) < fEps)
//        return;
//
//    vTargetLook = XMVector3Normalize(vTargetLook);
//
//    /* 현재 방향 */
//    _vector vCurLook = m_tComponents.transform.Get_StateXM(STATE::LOOK);
//    vCurLook = XMVectorSetY(vCurLook, 0.f);
//
//    if (XMVectorGetX(XMVector3LengthSq(vCurLook)) < fEps)
//        vCurLook = vTargetLook;
//    else
//        vCurLook = XMVector3Normalize(vCurLook);
//
//    /* 방향 벡터를 yaw 각도로 전환하기 */
//    _float fCurYaw = atan2f(XMVectorGetX(vCurLook), XMVectorGetZ(vCurLook));
//    _float fTargetYaw = atan2f(XMVectorGetX(vTargetLook), XMVectorGetZ(vTargetLook));
//
//    /* 회전할 yaw 각도 */
//    _float fDeltaYaw = fTargetYaw - fCurYaw;
//
//    /* 최단거리로 보정하기(-180 ~ 180) :-340 -> 20 */
//    while (fDeltaYaw > XM_PI)  fDeltaYaw -= XM_2PI;
//    while (fDeltaYaw < -XM_PI) fDeltaYaw += XM_2PI;
//
//    const _float fT = 1.f - expf(-m_fRotateSharpness * fDT);
//
//    _float fNewYaw = fCurYaw + fDeltaYaw * fT;
//
//    LOG_INFO("curYaw=%.3f, targetYaw=%.3f, deltaYaw=%.3f, newYaw=%.3f",
//        XMConvertToDegrees(fCurYaw),
//        XMConvertToDegrees(fTargetYaw),
//        XMConvertToDegrees(fDeltaYaw),
//        XMConvertToDegrees(fNewYaw));
//
//    _vector qRot = XMQuaternionRotationRollPitchYaw(0.f, fNewYaw, 0.f);
//    m_tComponents.transform.Set_Rotation_Quaternion(qRot);
//
//    _float3 vRot3 = m_tComponents.transform.Get_Rotation_Euler();
//
//    LOG_INFO("AFTER : x=%.3f, y=%.3f, z=%.3f", vRot3.x, vRot3.y, vRot3.z);
//
//    m_vPrevLook = m_tInputCmd.vLook;
//}

void CPlayerState::Cache_PlayerInfos(const PLAYER_COMPONENTS& tComponents, const PLAYER_RUNTIME_REF& tRef, PLAYER_INFO* pInfo)
{
    m_tComponents = tComponents;
    m_tRef = tRef;
    m_pFSM = tRef.pFSM;
    m_pInfo = pInfo;
}

PLAYER_STATE CPlayerState::Get_State() const
{
    return m_eState;
}

const char* CPlayerState::Get_StateName() const
{
    return m_szStateName;
}
