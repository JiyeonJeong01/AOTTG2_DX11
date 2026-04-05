#include "NormalTitanState_Dead.h"

#include "AnimationClip_Titan.h"

CNormalTitanState_Dead::CNormalTitanState_Dead(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CNormalTitanState_Dead::~CNormalTitanState_Dead()
{
}

HRESULT CNormalTitanState_Dead::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CNormalTitanState_Dead::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CNormalTitanState_Dead::Update(_float fDT)
{
    CTitanState::Update(fDT);
}

void CNormalTitanState_Dead::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);
}

void CNormalTitanState_Dead::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::DIE_FRONT);
}

void CNormalTitanState_Dead::Exit()
{
    CTitanState::Exit();
}

void CNormalTitanState_Dead::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

void CNormalTitanState_Dead::Decide_NextState()
{
}

void CNormalTitanState_Dead::Decide_NextAnim()
{
}

std::shared_ptr<CNormalTitanState_Dead> CNormalTitanState_Dead::Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CNormalTitanState_Dead>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}
