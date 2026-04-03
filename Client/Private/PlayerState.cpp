#include "PlayerState.h"
#include "CameraController.h"
#include "Easing_Function.h"
#include "ODM_Gear.h"

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

void CPlayerState::Enter(_uint iDetailFlag)
{
    m_bAcivated = true;
}

void CPlayerState::Exit()
{
    m_bAcivated = false;
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

void CPlayerState::Setup_CachedPlayerContext()
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

    while (fNewYaw > XM_PI)  fNewYaw -= XM_2PI;
    while (fNewYaw < -XM_PI) fNewYaw += XM_2PI;

    m_fCurrentYaw = fNewYaw;

    _vector qRot = XMQuaternionRotationRollPitchYaw(0.f, fNewYaw, 0.f);
    m_tComponents.transform.Set_Rotation_Quaternion(qRot);
}

void CPlayerState::Try_Grappling()
{
    if (m_tInputCmd.bLeftAnchorPressed)
    {
        /* 앵커 고정 가능한지 판단 */
        m_tRef.pGear->Try_Grappling(SIDE::LEFT);
        return;
    }
    if (m_tInputCmd.bRightAnchorPressed)
    {
        /* 앵커 고정 가능한지 판단 */
        m_tRef.pGear->Try_Grappling(SIDE::RIGHT);
        return;
    }
}

void CPlayerState::Finish_Grappling()
{
    _bool bLeftHook = m_tInputCmd.bLeftAnchorHeld || m_tInputCmd.bLeftAnchorPressed;;
    _bool bRightHook = m_tInputCmd.bRightAnchorHeld || m_tInputCmd.bRightAnchorPressed;

    if (!bLeftHook)
    {
        m_tRef.pGear->Finish_Grappling(SIDE::LEFT);
    }
    if (!bRightHook)
    {
        m_tRef.pGear->Finish_Grappling(SIDE::RIGHT);
    }
}

void CPlayerState::GroundedMove(_float fDT)
{
    const _float fMaxSpeed = m_pStats->fMaxSpeed;
    const _float fCurSpeed = m_pStats->fCurSpeed;

    /* 플레이어의 현재 속도 */
    _float3 vLinearVel = m_tComponents.rigidbody.Get_LinearVel();

    _float3 vMoveDir{};
    vMoveDir.x = m_tInputCmd.vMove.x;
    vMoveDir.z = m_tInputCmd.vMove.z;

    const _float fMoveLenSq = vMoveDir.x * vMoveDir.x + vMoveDir.z * vMoveDir.z;

    /* 입력 없음 */
    if (fMoveLenSq <= 0.f)
        return;

    _float3 vHorizontalVel{};
    vHorizontalVel.x = vLinearVel.x;
    vHorizontalVel.z = vLinearVel.z;

    const _float fHorizontalSpeedSq =
        vHorizontalVel.x * vHorizontalVel.x +
        vHorizontalVel.z * vHorizontalVel.z;

    /* 최대 속도 제한 */
    if (fHorizontalSpeedSq < fMaxSpeed * fMaxSpeed)
    {
        _float3 vForce{};
        vForce.x = vMoveDir.x * fCurSpeed * fCurSpeed;
        vForce.z = vMoveDir.z * fCurSpeed * fCurSpeed;

        m_tComponents.rigidbody.Add_Force(vForce);
    }
}

void CPlayerState::Cache_PlayerContext(const PLAYER_CONTEXT& tContext)
{
    m_tComponents = tContext.tComponents;
    m_tRef = tContext.tRef;
    m_pStats = tContext.pStats;
    m_pSkillController = tContext.pSkillController;
}

PLAYER_STATE CPlayerState::Get_State() const
{
    return m_eState;
}

const char* CPlayerState::Get_StateName() const
{
    return m_szStateName;
}
