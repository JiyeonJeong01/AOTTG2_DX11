#include "PlayerState_AirborneMove.h"
#include "AnimationClip_Player.h"
#include "PlayerStateMachine.h"
#include "ODM_Gear.h"

CPlayerState_AirborneMove::CPlayerState_AirborneMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
    : CPlayerState(goPlayer, scPlayer)
{
}

CPlayerState_AirborneMove::~CPlayerState_AirborneMove()
{
}

HRESULT CPlayerState_AirborneMove::Initialize()
{
    return CPlayerState::Initialize();
}

void CPlayerState_AirborneMove::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);
}

void CPlayerState_AirborneMove::Update(_float fDT)
{
    _uint iFlag = m_tRef.pGear->Get_UsingFlag();

    /* 그래플링 끝 */
    if (m_tInputCmd.bLeftAnchorHeld == false && m_tInputCmd.bRightAnchorHeld == false)
    {
        m_eState = AIRBORNE_STATE::AIR_FALL;
        m_tRef.pGear->Finish_Grappling();
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);
    }

}

void CPlayerState_AirborneMove::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
}

void CPlayerState_AirborneMove::Decide_NextState()
{
}

void CPlayerState_AirborneMove::Enter(_uint iDetailFlag)
{
    LOG_INFO("[ ENTER PLAYERSTATE_AIRBORNEMOVE ]");
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

std::shared_ptr<CPlayerState_AirborneMove> CPlayerState_AirborneMove::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
{
    auto pInstance = std::make_shared<CPlayerState_AirborneMove>(goPlayer, scPlayer);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
