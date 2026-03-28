#include "PlayerState_Airborne.h"
#include "AnimationClip_Player.h"
#include "PlayerStateMachine.h"
#include "ODM_Gear.h"

using namespace ANIM_PLAYER;

CPlayerState_Airborne::CPlayerState_Airborne(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
    : CPlayerState(goPlayer, scPlayer)
{
}

CPlayerState_Airborne::~CPlayerState_Airborne()
{
}

HRESULT CPlayerState_Airborne::Initialize()
{
    return CPlayerState::Initialize();
}

void CPlayerState_Airborne::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);
}

void CPlayerState_Airborne::Update(_float fDT)
{



}

void CPlayerState_Airborne::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
}

void CPlayerState_Airborne::Decide_NextState()
{
}

void CPlayerState_Airborne::Enter()
{
    _uint iFlag = m_tRef.pGear->Get_UsingFlag();

    if ((iFlag & To<_uint>(SIDE::BOTH)) != 0)
    {
        /* Grapple Action (Left, Right, or Both) */
        m_tComponents.animator.Set_NextAnimationClip(AIR);
    }
    else
    {
        /* Falling or Normal Jump */
    }
}

std::shared_ptr<CPlayerState_Airborne> CPlayerState_Airborne::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
{
    auto pInstance = std::make_shared<CPlayerState_Airborne>(goPlayer, scPlayer);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
