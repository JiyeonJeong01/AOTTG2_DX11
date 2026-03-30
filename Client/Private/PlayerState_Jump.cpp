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
    _bool bFalling = m_tComponents.animator.Get_CurAnimaionClipIdx() == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::AIR_FALL);
    if (m_tRef.pGroundChecker->Get_OnWalkable() && bFalling)
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
    /* 상승 후 하락 */
    if (m_eJumpState != JUMP::FALL
        && m_tComponents.rigidbody.Get_LinearVel().y < 0.f)
    {
        m_eJumpState = JUMP::FALL;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);
    }
}

void CPlayerState_Jump::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::JUMP])
        On_JumpFinished(tData);
}

void CPlayerState_Jump::On_JumpFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RISE);
}


std::shared_ptr<CPlayerState_Jump> CPlayerState_Jump::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Jump>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
