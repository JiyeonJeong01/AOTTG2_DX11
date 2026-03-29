#include "PlayerState_Idle.h"

#include "AnimationClip_Player.h"
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

void CPlayerState_Idle::Decide_NextState()
{
    if (m_tInputCmd.bBoostPressed
        /* && (m_tRef.pGroundChecker && m_tRef.pGroundChecker->Is_OnGround()) */
        && m_pFSM)
    {
        m_pFSM->Change_State(To<_uint>(PLAYER_STATE::JUMP));
    }

    if (m_tRef.pGear)
    {
        /* 앵커 고정 가능한지 판단 */
        if (m_tInputCmd.bLeftAnchorPressed)
        {
            m_tRef.pGear->Try_Grappling(SIDE::LEFT);
        }
        if (m_tInputCmd.bLeftAnchorPressed)
        {
            m_tRef.pGear->Try_Grappling(SIDE::RIGHT);
        }
    }

    if (!XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero()))
    {
        m_pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE));
    }
}

void CPlayerState_Idle::Enter(_uint iDetailFlag)
{
    m_tComponents.animator.Set_NextAnimationClip(IDLE_F);
}

std::shared_ptr<CPlayerState_Idle> CPlayerState_Idle::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Idle>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
