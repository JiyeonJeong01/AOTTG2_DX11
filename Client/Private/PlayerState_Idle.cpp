#include "PlayerState_Idle.h"

#include "AnimationClip_Player.h"
#include "GroundChecker.h"
#include "PlayerStateMachine.h"

#include "ODM_Gear.h"   

using namespace ANIM_PLAYER;

CPlayerState_Idle::CPlayerState_Idle(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_Idle::~CPlayerState_Idle()
{
}

HRESULT CPlayerState_Idle::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr.");

    return S_OK;
}

void CPlayerState_Idle::Priority_Update(_float fDT)
{
    Control_Camera();
    LookTo_InputDir(fDT);
    Try_Grappling();
}

void CPlayerState_Idle::Update(_float fDT)
{
    CPlayerState::Update(fDT);
}

void CPlayerState_Idle::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    Decide_NextState();
}

void CPlayerState_Idle::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    cout << "[IDLE] ENTER \n";

    m_tComponents.animator.Set_NextAnimationClip(IDLE_F);
}

void CPlayerState_Idle::Exit()
{
    CPlayerState::Exit();
}

void CPlayerState_Idle::Setup_CachedPlayerInfos()
{
    CPlayerState::Setup_CachedPlayerInfos();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_Idle::On_AnimFinished, this);
}

void CPlayerState_Idle::Decide_NextState()
{
    /* -> JUMP */
    if (m_tInputCmd.bBoostPressed
        && (m_tRef.pGroundChecker && m_tRef.pGroundChecker->Get_OnWalkable())
        && m_tRef.pFSM)
    {
        cout << "[IDLE] -> JUMP::JUMP_BEGIN\n";

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::JUMP), To<_uint>(JUMP::JUMP_BEGIN));
        return;
    }

    /* -> GROUNDED_ATTAK */
    if (m_tInputCmd.bNormalAttackPressed)
    {
        cout << "[IDLE] -> GROUNDED_ATTACK::ATK\n";

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_ATTACK), To<_uint>(GROUNDED_ATTACK::ATK));
        return;
    }

    /* -> GROUNDED_MOVE */
    if (!XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero()))
    {
        cout << "[IDLE] -> GROUNDED_MOVE::JUMP_BEGIN\n";

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::RUN));
        return;
    }
}

void CPlayerState_Idle::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::JUMP])
        On_DashLandFinished(tData);
}

void CPlayerState_Idle::On_DashLandFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::IDLE_F);
}

std::shared_ptr<CPlayerState_Idle> CPlayerState_Idle::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Idle>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
