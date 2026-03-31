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
    Control_Camera();
    LookTo_InputDir(fDT);
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
    Try_Grappling();
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

void CPlayerState_Jump::Setup_CachedPlayerInfos()
{
    CPlayerState::Setup_CachedPlayerInfos();

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
        _float3 vVelocity = m_tComponents.rigidbody.Get_LinearVel();
        _float fVelSq = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&vVelocity)));
        const _float fThreshold = 25.f;

        if (fVelSq < 25.f)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::DASH_LAND));
        else
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::SLIDE));
    }
}

void CPlayerState_Jump::Decide_NextAnim()
{
    /* 점프 후, 가속 대쉬 */
    _bool bBoost = m_tInputCmd.bBoostHeld;

    _bool bJumpBegin = m_tComponents.animator->iAnimationClip == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::JUMP];
    if (m_eJumpState != JUMP::DASH && bBoost && !bJumpBegin)
    {
        m_eJumpState = JUMP::DASH;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR);
        return;
    }

    /* 점프 대쉬 중단 */
    if (m_eJumpState == JUMP::DASH && !bBoost)
    {
        m_eJumpState = JUMP::FALL;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);
        return;
    }

    /* 상승 후 하락 */
    if (m_eJumpState != JUMP::FALL
        && m_tComponents.rigidbody.Get_LinearVel().y < 0.f)
    {
        m_eJumpState = JUMP::FALL;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);

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

    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RISE);
    m_eJumpState = JUMP::RISE;
}

void CPlayerState_Jump::Jump_Dash(_float fDT)
{
    if (m_eJumpState != JUMP::DASH)
        return;

    _float3 vDashForce = m_tComponents.rigidbody.Get_LinearVel();

    vDashForce.x *= m_pStats->fJumpDash * fDT;
    vDashForce.z *= m_pStats->fJumpDash * fDT;

    m_tComponents.rigidbody.Add_Force(vDashForce);
}

std::shared_ptr<CPlayerState_Jump> CPlayerState_Jump::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Jump>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
