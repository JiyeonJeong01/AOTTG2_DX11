#include "TitanState.h"

HRESULT CTitanState::Initialize()
{
    return S_OK;
}

void CTitanState::Priority_Update(_float fDT)
{
}

void CTitanState::Update(_float fDT)
{
}

void CTitanState::Late_Update(_float fDT)
{
}

void CTitanState::Enter(_uint iDetailFlag)
{
    m_bAcivated = true;
}

void CTitanState::Exit()
{
    m_bAcivated = false;
}

void CTitanState::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    m_tComponents = tContext.tComponents;
    m_tRef = tContext.tRef;
    m_pStats = tContext.pStats;
}

void CTitanState::Setup_CachedTitanContext()
{  
}

_vector CTitanState::Get_WanderMoveDir()
{
    return { 1.f, 0.f, 0.f };
}

void CTitanState::GroundedMove(_fvector vDir, float fDT)
{
    const _float fMaxSpeed = m_pStats->fMaxSpeed;
    const _float fCurSpeed = m_pStats->fCurSpeed;

    /* 플레이어의 현재 속도 */
    _float3 vLinearVel = m_tComponents.rigidbody.Get_LinearVel();

    _float3 vMoveDir{};
    XMStoreFloat3(&vMoveDir, vDir);

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

void CTitanState::Look_To(_fvector vDir, _float fDT)
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

    _vector vTargetLook = vDir;
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

void CTitanState::Detect_Human()
{
}

void CTitanState::Chase_Human()
{
}

TITAN_STATE CTitanState::Get_State() const
{
    return m_eState;
}

const char* CTitanState::Get_StateName() const
{
    return m_szStateName;
}
