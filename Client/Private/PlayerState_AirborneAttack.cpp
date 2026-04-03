#include "PlayerState_AirborneAttack.h"
#include "AnimationClip_Player.h"

#include "PlayerStateMachine.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"
#include "Player_SkillController.h"
#include "ThrownBlade.h"

CPlayerState_AirborneAttack::CPlayerState_AirborneAttack(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_AirborneAttack::~CPlayerState_AirborneAttack()
{
}

HRESULT CPlayerState_AirborneAttack::Initialize()
{
    m_pThrownBlade = m_goPlayer->Get_Script_InChildren<CThrownBlade>();
    IF_NULL_RETURN_MSG_BREAK(m_pThrownBlade, E_FAIL, "m_pThrownBlade is nullptr");

    return CPlayerState::Initialize();
}

void CPlayerState_AirborneAttack::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_AirborneAttack::On_AnimFinished, this);
}

void CPlayerState_AirborneAttack::Priority_Update(_float fDT)
{
    Control_Camera();
    if (!m_bSpinH_Force && !m_bSpinV_Force)
        LookTo_InputDir(fDT);
    Try_Grappling();
    Finish_Grappling();
}

void CPlayerState_AirborneAttack::Update(_float fDT)
{
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::NORMAL)
    {
        return;
    }
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_H)
    {
        Spin_Horizontal(fDT);
        return;
    }
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::THROW)
    {
        Throw_Blade(fDT);
        return;
    }
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_V)
    {
        Spin_Vertical(fDT);
        return;
    }
}

void CPlayerState_AirborneAttack::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
    Decide_NextState();
}

void CPlayerState_AirborneAttack::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    Set_InitialValue();

    if (iDetailFlag < To<_uint>(AIRBORNE_ATTACK::END))
        m_eAirborneAttackState = To<AIRBORNE_ATTACK>(iDetailFlag);

    /* 약공격 */
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::NORMAL)
    {
        cout << "[AIRBORNE_ATTACK] NOMAL\n";
        /* 왼쪽 훅 사용 중 */
        if (m_tRef.pGear->Get_UsingFlag() & To<_uint>(SIDE::LEFT))
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::ATTACK_1_HOOK_L1);
        /* 오른쪽 훅 사용 중 */
        else if (m_tRef.pGear->Get_UsingFlag() & To<_uint>(SIDE::RIGHT))
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::ATTACK_1_HOOK_R1);
        /* 정면 */
        else
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::ATTACK_2);/* ATTACK2 = AIRBORNE_ATTACK::NORMAL 동작으로 사용 */

        return;
    }

    /* 공격을 정하지 않고 진입한 경우 */
    Decide_State_If_Needed();

    /* 강공격 1. 스핀 수평 */
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_H)
    {
        cout << "[AIRBORNE_ATTACK] SPIN_H\n";

        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::ATTACK_1); /* ATTACK1 = SpinH 시작 동작으로 사용 */
        m_bAnimFinished = false;
        m_bKeepAttack = true;

        return;
    }

    /* 강공격 2. 칼날 던지기 */
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::THROW)
    {
        cout << "[AIRBORNE_ATTACK] THROW\n";

        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SPECIAL_PETRA);
        m_bAnimFinished = false;
        m_bKeepAttack = true;

        return;
    }

    /* 강공격 3. 스핀 수직 */
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_V)
    {
        cout << "[AIRBORNE_ATTACK] SPIN_V\n";

        _vector vRight = m_tComponents.transform.Get_StateXM(STATE::RIGHT);
        XMStoreFloat3(&m_vSpinV_Axis, vRight);

        const _float fLenSq = m_vSpinV_Axis.x * m_vSpinV_Axis.x
            + m_vSpinV_Axis.y * m_vSpinV_Axis.y
            + m_vSpinV_Axis.z * m_vSpinV_Axis.z;

        if (fLenSq > 0.0001f)
        {
            const _float fInvLen = 1.f / sqrtf(fLenSq);
            m_vSpinV_Axis.x *= fInvLen;
            m_vSpinV_Axis.y *= fInvLen;
            m_vSpinV_Axis.z *= fInvLen;
        }
        else
        {
            m_vSpinV_Axis = { 1.f, 0.f, 0.f };
        }

        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::ATTACK_3_1);
        m_bAnimFinished = false;
        m_bKeepAttack = true;

        return;
    }
    cout << "[AIRBORNE_ATTACK] 지정되지 않는 상태\n";
}

void CPlayerState_AirborneAttack::Exit()
{
    CPlayerState::Exit();
}

void CPlayerState_AirborneAttack::Decide_NextState()
{
    /* -> GROUNDED_MOVE(착지/슬라이딩) */
    if (m_bAnimFinished && !m_bKeepAttack && m_tRef.pGroundChecker->Get_OnWalkable())
    {
        cout << "[AIRBORNE_ATTACK] -> GROUNDED_MOVE\n";
        const float THREASHOLD = 10.f;

        _float3 fLinearVel = m_tComponents.rigidbody.Get_LinearVel();

        const _float fLinearVelSq = fLinearVel.x * fLinearVel.x + fLinearVel.z * fLinearVel.z;

        if (fLinearVelSq > THREASHOLD)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::SLIDE));
        else
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::DASH_LAND));

        return;
    }

    /* -> AIRBORNE_MOVE(착지/슬라이딩) */
    if (m_bAnimFinished && !m_bKeepAttack)
    {
        cout << "[AIRBORNE_ATTACK] -> AIRBORNE_MOVE\n";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR));
        return;
    }

}

void CPlayerState_AirborneAttack::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[AIRBORNE_ATTACK] On_AnimFinished\n";

    _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::ATTACK_1_HOOK_L1]
        || iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::ATTACK_1_HOOK_R1]
        || iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::ATTACK_2])
        On_NomalFinished(tData);
    else if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::ATTACK_1]) /* ATTACK1 = SpinH 시작 동작으로 사용 */
        On_SpinH_Finished(tData);
    else if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::SPECIAL_PETRA])
        On_Throw_Finished(tData);
    else if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::ATTACK_3_1])
        On_SpinV_Finished(tData);
}

void CPlayerState_AirborneAttack::On_NomalFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_bAnimFinished = true;
    m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));

    cout << " => [AIRBORNE_ATTACK] On_NomalFinished -> AIRBORNE\n";
}

void CPlayerState_AirborneAttack::On_SpinH_Finished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_SpinH_Elapsed_Degree = 0.f;
    m_bAnimFinished = true;
    m_bKeepAttack = true;

    static _int iCnt = 0;
    cout << " => [AIRBORNE_ATTACK] On_SpinH_Finished\n";
}

void CPlayerState_AirborneAttack::On_Throw_Finished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_bAnimFinished = true;
    m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));

    cout << " => [AIRBORNE_ATTACK] On_Throw_Finished\n";
}

void CPlayerState_AirborneAttack::On_SpinV_Finished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_SpinV_Elapsed_Degree = 0.f;
    m_bAnimFinished = true;
    m_bKeepAttack = true;

    cout << " => [AIRBORNE_ATTACK] On_SpinV_Finished\n";
}

void CPlayerState_AirborneAttack::Spin_Horizontal(_float fDT)
{
    m_fSpinH_WaitElapsedTime += fDT;

    m_bSpinH_Force = m_fSpinH_WaitElapsedTime >= m_fSpinH_WaitTotalTime;
         
    if (m_bSpinH_Force)
    {
        _float fRot = m_SpinH_Degree_PerSec * fDT;
        m_tComponents.transform.Rotate({ 0.f, 1.f, 0.f, 1.f }, fRot);
        m_SpinH_Elapsed_Degree += fRot;

        /* 회전 끝 */
        if (m_SpinH_Elapsed_Degree > m_SpinH_Total_Degree)
        {
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));
            m_bSpinH_Force = false;
            m_bKeepAttack = false;
        }
    }
}

void CPlayerState_AirborneAttack::Spin_Vertical(_float fDT)
{
    m_fSpinV_WaitElapsedTime += fDT;

    m_bSpinV_Force = m_fSpinV_WaitElapsedTime >= m_fSpinV_WaitTotalTime;

    if (m_bSpinV_Force)
    {
        _float fRot = m_SpinV_Degree_PerSec * fDT;

        m_tComponents.transform.Rotate(
            { m_vSpinV_Axis.x, m_vSpinV_Axis.y, m_vSpinV_Axis.z, 1.f },
            fRot, SPACE::WORLD);

        m_SpinV_Elapsed_Degree += fRot;

        if (m_SpinV_Elapsed_Degree > m_SpinV_Total_Degree)
        {
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));
            m_bSpinV_Force = false;
            m_bKeepAttack = false;
        }
    }
}

void CPlayerState_AirborneAttack::Throw_Blade(_float fDT)
{
    m_fThrow_WaitElapsedTime += fDT;

    m_bThrowNow = m_fThrow_WaitElapsedTime >= m_fThrow_WaitTotalTime;
    if (m_bThrowNow)
    {
        if (!m_bThrewAlready)
        {
            m_bThrewAlready = true;
            _vector vPos = XMLoadFloat3(&m_tComponents.transform->vPosition);
            _float3 vDir = GAME_INSTANCE.Cam_Look();
            m_pThrownBlade->Start_Throw(vPos, XMLoadFloat3(&vDir));
        }
    }
}

void CPlayerState_AirborneAttack::Set_InitialValue()
{
    m_bAnimFinished = false;
    m_bKeepAttack = false;

    m_SpinH_Elapsed_Degree = 0.f;
    m_fSpinH_WaitElapsedTime = 0.f;
    m_bSpinH_Force = false;

    m_SpinV_Elapsed_Degree = 0.f;
    m_fSpinV_WaitElapsedTime = 0.f;
    m_bSpinV_Force = false;
    m_vSpinV_Axis = { 1.f, 0.f, 0.f };

    m_bThrowNow = false;
    m_bThrewAlready = false;
    m_fThrow_WaitElapsedTime = 0.f;
}

void CPlayerState_AirborneAttack::Decide_State_If_Needed()
{
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::STRONG)
    {
        SKILL_TYPE eSkill = m_pSkillController->Get_CurSkillType();

        switch (eSkill)
        {
        case SKILL_TYPE::SPIN_H:
            m_eAirborneAttackState = AIRBORNE_ATTACK::SPIN_H;
            break;
        case SKILL_TYPE::THROW:
            m_eAirborneAttackState = AIRBORNE_ATTACK::THROW;
            break;
        case SKILL_TYPE::SPIN_V:
            m_eAirborneAttackState = AIRBORNE_ATTACK::SPIN_V;
            break;
        }
    }
}

std::shared_ptr<CPlayerState_AirborneAttack> CPlayerState_AirborneAttack::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_AirborneAttack>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
