#include "PlayerState_AirborneAttack.h"
#include "AnimationClip_Player.h"

#include "PlayerStateMachine.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"
#include "Player_SkillController.h"
#include "ThrownBlade.h"
#include "HitBox.h"

CPlayerState_AirborneAttack::CPlayerState_AirborneAttack(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_AirborneAttack::~CPlayerState_AirborneAttack()
{
}

HRESULT CPlayerState_AirborneAttack::Initialize()
{

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
    if (m_bAnimFinished) /* 애니메이션 재생이 아닐 때에만 회전 반영 */
        LookTo_InputDir(fDT);
    Try_Grappling();
    Finish_Grappling();
    Handle_Trail(fDT, WIDTH_TYPE::NORMAL);

    Handle_SpeedLines(fDT);
}

void CPlayerState_AirborneAttack::Update(_float fDT)
{
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::NORMAL)
    {
        Update_BladeHitBox(fDT);
        return;
    }
    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_H)
    {
        Spin_Horizontal(fDT);
        Update_BladeHitBox(fDT);
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
        Update_BladeHitBox(fDT);
        return;
    }
    Handle_BladeTrail(fDT, WIDTH_TYPE::NONE);
}

void CPlayerState_AirborneAttack::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
    Decide_NextState();
}

void CPlayerState_AirborneAttack::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    if (!m_pThrownBlade)
        m_pThrownBlade = m_goPlayer->Get_Script_InChildren<CThrownBlade>();

    Set_InitialValue();
    m_bBladeHitBoxStarted = false;

    auto it = m_tRef.pAllHitBoxes->find(PLAYER_BLADE_ATTACK);
    if (it != m_tRef.pAllHitBoxes->end())
    {
        m_pBladeHitBox = it->second;
        m_pBladeHitBox->Set_Active(false);
    }
    else
    {
        m_pBladeHitBox = nullptr;
    }

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

        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SPECIAL_LEVI); /* SPECIAL_LEVI = SpinH 시작 동작으로 사용 */
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

        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SPECIAL_PETRA);
        m_bAnimFinished = false;
        m_bKeepAttack = true;

        return;
    }
    cout << "[AIRBORNE_ATTACK] 지정되지 않는 상태\n";
}

void CPlayerState_AirborneAttack::Exit()
{
    CPlayerState::Exit();

    if (m_pBladeHitBox)
        m_pBladeHitBox->Set_Active(false);

    m_bBladeHitBoxStarted = false;
}

void CPlayerState_AirborneAttack::Cache_PlayerContext(const PLAYER_CONTEXT& tContext)
{
    CPlayerState::Cache_PlayerContext(tContext);

    m_pHitBox = tContext.pHitBox;
    IF_NULL_RETURN_MSG_BREAK(m_pHitBox, , "m_pHitBox is nullptr");
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
    else if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::SPECIAL_LEVI]) /* ATTACK1 = SpinH 시작 동작으로 사용 */
        On_SpinH_Finished(tData);
    else if (m_eAirborneAttackState == AIRBORNE_ATTACK::THROW && iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::SPECIAL_PETRA])
        On_Throw_Finished(tData);
    else if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_V && iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::SPECIAL_PETRA])
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
    m_bAnimFinished = true;
    m_bKeepAttack = false;

    if (m_iSpinH_LoopCnt >= m_iSpinH_TotalLoopCnt)
        m_bSpinH_Completed = true;

    m_tComponents.animator.Set_PlaySpeed(m_fOriginAnimPlaySpeed);
    m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));

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
    m_bAnimFinished = true;
    m_bKeepAttack = false;

    if (m_iSpinV_LoopCnt >= m_iSpinV_TotalLoopCnt)
        m_bSpinV_Completed = true;

    m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));

    cout << " => [AIRBORNE_ATTACK] On_SpinV_Finished\n";
}

void CPlayerState_AirborneAttack::Update_BladeHitBox(_float fDT)
{
    if (!m_pBladeHitBox)
        return;
    Handle_BladeTrail(fDT, WIDTH_TYPE::THIN);

    /* 히트박스 on */
    if (!m_bBladeHitBoxStarted)
    {
        if (Can_Start_BladeHitBox())
        {
            m_pBladeHitBox->Set_Active(true);
            m_bBladeHitBoxStarted = true;
        }
        return;
    }

    /* 히트박스 off */
    if (m_pBladeHitBox->Get_Active() && Can_End_BladeHitBox())
    {
        m_pBladeHitBox->Set_Active(false);
    }

}

void CPlayerState_AirborneAttack::Spin_Horizontal(_float fDT)
{
    if (m_bSpinH_Completed)
        return;

    /* 애니메이션 재생 속도 강제 조정 */
    if (m_tComponents.animator.Get_TrackPosition() > m_fSpinH_LoopStart_TrackPosition)
    {
        m_tComponents.animator.Set_PlaySpeed(m_fSpinH_Speed);
    }

    /* 애니메이션 특정 구간 강제 반복 */
    if (!m_bSpinH_Looping
        && m_iSpinH_LoopCnt < m_iSpinH_TotalLoopCnt
        && m_tComponents.animator.Get_TrackPosition() > m_fSpinH_LoopEnd_TrackPosition)
    {
        m_tComponents.animator.Set_TrackPosition(m_fSpinH_LoopStart_TrackPosition);

        m_iSpinH_LoopCnt++;
    }
}

void CPlayerState_AirborneAttack::Spin_Vertical(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (m_bSpinV_Completed)
        return;

    /* 애니메이션 특정 구간 강제 반복 */
    if (!m_bSpinV_Looping
        && m_iSpinV_LoopCnt < m_iSpinV_TotalLoopCnt
        && m_tComponents.animator.Get_TrackPosition() > m_fSpinV_LoopEnd_TrackPosition)
    {
        m_tComponents.animator.Set_TrackPosition(m_fSpinV_LoopStart_TrackPosition);

        m_iSpinV_LoopCnt++;
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

            _vector vLook = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::LOOK));
            _vector vUp = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::UP));
            _vector vStartPos = vPos + vLook * m_vThrowOffset.z + vUp * m_vThrowOffset.y;

            _float3 vDir = GAME_INSTANCE.Cam_Look();
            if (m_pThrownBlade)
                m_pThrownBlade->Start_Throw(vStartPos, XMLoadFloat3(&vDir));
        }
    }
}

void CPlayerState_AirborneAttack::Set_InitialValue()
{
    m_bAnimFinished = false;
    m_bKeepAttack = false;

    m_bSpinH_Completed = false;
    m_bSpinH_Looping = false;
    m_iSpinH_LoopCnt = 0;

    m_bSpinV_Completed = false;
    m_bSpinV_Looping = false;
    m_iSpinV_LoopCnt = 0;

    m_bThrowNow = false;
    m_bThrewAlready = false;
    m_fThrow_WaitElapsedTime = 0.f;
}

_bool CPlayerState_AirborneAttack::Can_Start_BladeHitBox() const
{
    if (!m_pBladeHitBox)
        return false;

    if (m_bBladeHitBoxStarted)
        return false;

    if (!m_pBlade->Can_ConsumeBladeAtk())
        return false;

    const _float fTrackPosition = m_tComponents.animator->fTrackPosition;

    if (m_eAirborneAttackState == AIRBORNE_ATTACK::NORMAL)
        return fTrackPosition >= m_fNormal_HitBoxStartTrackPos;

    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_H)
        return fTrackPosition >= m_fSpinH_HitBoxStartTrackPos;

    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_V)
        return fTrackPosition >= m_fSpinV_HitBoxStartTrackPos;

    return false;
}

_bool CPlayerState_AirborneAttack::Can_End_BladeHitBox() const
{
    if (!m_pBladeHitBox)
        return true;

    if (!m_bBladeHitBoxStarted)
        return false;

    if (m_eAirborneAttackState == AIRBORNE_ATTACK::NORMAL)
        return m_bAnimFinished;

    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_H)
        return m_bSpinH_Completed;

    if (m_eAirborneAttackState == AIRBORNE_ATTACK::SPIN_V)
        return m_bSpinV_Completed;

    return true;
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
