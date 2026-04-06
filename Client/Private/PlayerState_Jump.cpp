#include "PlayerState_Jump.h"
#include "AnimationClip_Player.h"
#include "GroundChecker.h"
#include "PlayerStateMachine.h"

CPlayerState_Jump::CPlayerState_Jump(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_Jump::~CPlayerState_Jump()
{
}

HRESULT CPlayerState_Jump::Initialize()
{
    return CPlayerState::Initialize();
}

void CPlayerState_Jump::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);

    Control_Camera();
    LookTo_InputDir(fDT);
    Try_Grappling();
    Finish_Grappling();
}

void CPlayerState_Jump::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    Jump_Dash(fDT);
}

void CPlayerState_Jump::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    Decide_NextAnim();
    Decide_NextState();
}

void CPlayerState_Jump::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    m_eJumpState = To<JUMP>(iDetailFlag);
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::JUMP);

    /* 위로 가속 */
    _float3 vWorldUp = { 0.f, m_pStats->fJump, 0.f };
    m_tComponents.rigidbody.Add_LinearImpulse(vWorldUp);
}

void CPlayerState_Jump::Exit()
{
    CPlayerState::Exit();
}

void CPlayerState_Jump::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_Jump::On_AnimFinished, this);
}

void CPlayerState_Jump::Decide_NextState()
{
    /* AIRBORNE_ATTACK */
    if (m_tInputCmd.bNormalAttackPressed)
    {
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_ATTACK), To<_uint>(AIRBORNE_ATTACK::NORMAL));
        return;
    }

    /* -> GROUNDED_MOVE */
    _bool bFalling = m_tComponents.animator.Get_CurAnimaionClipIdx() == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::AIR_FALL);
    _bool bJumpDash = m_tComponents.animator.Get_CurAnimaionClipIdx() == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::AIR);
    if (m_tRef.pGroundChecker->Get_OnWalkable() && (bFalling || bJumpDash))
    {
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::RUN));
        return;
    }

    _bool bReload = m_tInputCmd.bReloadBlade;
    if (bReload)
    {
        cout << "[AIRBORNE_MOVE] -> RELOAD::AIR\n";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::RELOAD), To<_uint>(RELOAD::AIR));
        return;
    }
}
void CPlayerState_Jump::Decide_NextAnim()
{
    /* 점프 후, 가속 대쉬 */
    _bool bBoost = m_tInputCmd.bBoostHeld;

    const _uint iCurClip = m_tComponents.animator.Get_CurAnimaionClipIdx();
    const _uint iNextClip = m_tComponents.animator->iNextAnimationClip;

    const _uint iJumpClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::JUMP);
    const _uint iAirRiseClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::AIR_RISE);
    const _uint iAirClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::AIR);
    const _uint iAirFallClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::AIR_FALL);

    _bool bJumpBegin = iCurClip == iJumpClip;
    _bool bAirRisePlaying = iCurClip == iAirRiseClip;
    _bool bAirRiseReserved = iNextClip == iAirRiseClip;
    _bool bAirPlaying = iCurClip == iAirClip;
    _bool bAirReserved = iNextClip == iAirClip;
    _bool bAirFallPlaying = iCurClip == iAirFallClip;
    _bool bAirFallReserved = iNextClip == iAirFallClip;

    /* 상승 후 하락 */
    if (m_tComponents.rigidbody.Get_LinearVel().y < 0.f)
    {
        if (m_eJumpState != JUMP::FALL)
        {
            m_eJumpState = JUMP::FALL;
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);
        }
        else if (!bAirFallPlaying && !bAirFallReserved)
        {
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);
        }

        return;
    }

    /* 점프 후, 가속 대쉬 */
    /* FALL 상태에서는 AIR로 되돌아가지 않는다 */
    /* AIR_RISE 재생 중이거나 예약 중이면 AIR로 덮어쓰지 않는다 */
    if (m_eJumpState != JUMP::DASH
        && m_eJumpState != JUMP::FALL
        && bBoost
        && !bJumpBegin
        && !bAirRisePlaying
        && !bAirRiseReserved
        && !bAirPlaying
        && !bAirReserved)
    {
        m_eJumpState = JUMP::DASH;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR);
        return;
    }

    /* 점프 대쉬 중단 */
    if (m_eJumpState == JUMP::DASH && !bBoost)
    {
        if (!bAirRisePlaying && !bAirRiseReserved)
            m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RISE);

        m_eJumpState = JUMP::RISE;
        return;
    }
}

void CPlayerState_Jump::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[JUMP] On_AnimFinished\n";

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::JUMP])
        On_JumpFinished(tData);
}

void CPlayerState_Jump::On_JumpFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    cout << " => [JUMP] On_JumpFinished\n";
    cout << "[JUMP] -> AIR_RISE\n";
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RISE);
    m_eJumpState = JUMP::RISE;
}

void CPlayerState_Jump::Jump_Dash(_float fDT)
{
    if (m_eJumpState != JUMP::DASH)
        return;

    _float3 vDashForce = m_tComponents.rigidbody.Get_LinearVel();

    vDashForce.x *= m_pStats->fJumpDashH * fDT;
    vDashForce.y = m_pStats->fJumpDashV;
    vDashForce.z *= m_pStats->fJumpDashH * fDT;

    m_tComponents.rigidbody.Add_Force(vDashForce);
}

std::shared_ptr<CPlayerState_Jump> CPlayerState_Jump::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Jump>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
