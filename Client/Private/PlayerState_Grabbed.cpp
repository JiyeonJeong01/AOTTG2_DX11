#include "PlayerState_Grabbed.h"

#include "AnimationClip_Player.h"

CPlayerState_Grabbed::CPlayerState_Grabbed(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_Grabbed::~CPlayerState_Grabbed()
{
}

HRESULT CPlayerState_Grabbed::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_scPlayer, E_FAIL, "m_scPlayer is nullptr");

    return S_OK;
}

void CPlayerState_Grabbed::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    
}

void CPlayerState_Grabbed::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);

    Handle_Trail(fDT, WIDTH_TYPE::NONE);
}

void CPlayerState_Grabbed::Update(_float fDT)
{
    CPlayerState::Update(fDT);
}

void CPlayerState_Grabbed::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
}

void CPlayerState_Grabbed::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::GRABBED);

    m_tComponents.collider.Set_Enable(false);
    m_tComponents.rigidbody.Set_Enable(false);
    m_tComponents.springJoint.Set_Enable(false);
}

void CPlayerState_Grabbed::Exit()
{
    CPlayerState::Exit();

    m_tComponents.collider.Set_Enable(true);
    m_tComponents.rigidbody.Set_Enable(true);
    m_tComponents.springJoint.Set_Enable(true);
}

void CPlayerState_Grabbed::Decide_NextState()
{
}

std::shared_ptr<CPlayerState_Grabbed> CPlayerState_Grabbed::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Grabbed>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
