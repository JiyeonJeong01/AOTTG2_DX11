#include "PlayerState_AirborneMove.h"
#include "AnimationClip_Player.h"
#include "PlayerStateMachine.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"

CPlayerState_AirborneMove::CPlayerState_AirborneMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_AirborneMove::~CPlayerState_AirborneMove()
{
}

HRESULT CPlayerState_AirborneMove::Initialize()
{
    return CPlayerState::Initialize();
}

void CPlayerState_AirborneMove::Setup_CachedPlayerInfos()
{
    CPlayerState::Setup_CachedPlayerInfos();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_AirborneMove::On_AnimFinished, this);
}

void CPlayerState_AirborneMove::Priority_Update(_float fDT)
{
    Control_Camera();
    LookTo_InputDir(fDT);
}

void CPlayerState_AirborneMove::Update(_float fDT)
{
    _uint iFlag = m_tRef.pGear->Get_UsingFlag();

    /* 그래플링 끝 */
    if (m_eState != AIRBORNE_STATE::AIR_FALL
        && (m_tInputCmd.bLeftAnchorHeld == false && m_tInputCmd.bRightAnchorHeld == false))
    {
        m_eState = AIRBORNE_STATE::AIR_FALL;
        m_tRef.pGear->Finish_Grappling();
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);
    }
}

void CPlayerState_AirborneMove::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
    Decide_NextState();
}

void CPlayerState_AirborneMove::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    _uint iFlag = m_tRef.pGear->Get_UsingFlag();

    if ((iFlag & To<_uint>(SIDE::BOTH)) != 0)
    {
        /* Grapple Action (Left, Right, or Both) */
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR);
    }
    else
    {
        /* Falling or Normal Jump */
    }
}

void CPlayerState_AirborneMove::Exit()
{
    CPlayerState::Exit();
}

void CPlayerState_AirborneMove::Decide_NextState()
{
    if (m_eState == AIRBORNE_STATE::AIR_FALL && m_tRef.pGroundChecker->Get_OnWalkable())
    {
        const float THREASHOLD = 10.f;

        _float3 fLinearVel = m_tComponents.rigidbody.Get_LinearVel();

        const _float fLinearVelSq = fLinearVel.x * fLinearVel.x + fLinearVel.z * fLinearVel.z;

        if (fLinearVelSq > THREASHOLD)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::SLIDE));
        else
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::DASH_LAND));
    }
}

void CPlayerState_AirborneMove::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::AIR])
        On_AirFinished(tData);
    else if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::DASH_LAND])
        On_DashLandFinished(tData);
}

void CPlayerState_AirborneMove::On_AirFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if ((m_tRef.pGear->Get_UsingFlag() & To<_uint>(SIDE::BOTH)) == (To<_uint>(SIDE::BOTH)))
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RIGHT);
    else if (m_tRef.pGear->Get_UsingFlag() & To<_uint>(SIDE::LEFT))
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_LEFT);
    else if (m_tRef.pGear->Get_UsingFlag() & To<_uint>(SIDE::RIGHT))
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RIGHT);
}

void CPlayerState_AirborneMove::On_DashLandFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero()))
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::DASH_LAND);
    }
    else
    {
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE));
    }
}

std::shared_ptr<CPlayerState_AirborneMove> CPlayerState_AirborneMove::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_AirborneMove>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
