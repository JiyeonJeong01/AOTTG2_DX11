#include "PlayerState_Jump.h"
#include "AnimationClip_Player.h"

using namespace ANIM_PLAYER;

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
}

void CPlayerState_Jump::Decide_NextState()
{
}

void CPlayerState_Jump::Enter(_uint iDetailFlag)
{
    m_tComponents.animator.Set_NextAnimationClip(JUMP);
}

std::shared_ptr<CPlayerState_Jump> CPlayerState_Jump::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Jump>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
