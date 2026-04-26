#include "AbnormalTitanState_Dead.h"

#include "AnimationClip_Titan.h"

CAbnormalTitanState_Dead::CAbnormalTitanState_Dead(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Dead::~CAbnormalTitanState_Dead()
{
}

HRESULT CAbnormalTitanState_Dead::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Dead::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CAbnormalTitanState_Dead::Update(_float fDT)
{
    CTitanState::Update(fDT);


}

void CAbnormalTitanState_Dead::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);
}

void CAbnormalTitanState_Dead::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    if (*m_tRef.pPose == TITAN_POSE::STAND)
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::DIE_FRONT);
    else if (*m_tRef.pPose == TITAN_POSE::SIT)
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::SIT_DIE);
    else if (*m_tRef.pPose == TITAN_POSE::CRAWL)
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::CRAWLER_DIE);

    m_scTitan->Set_Dead();

    //m_tComponents.collider.Set_Enable(false);
}

void CAbnormalTitanState_Dead::Exit()
{
    CTitanState::Exit();
}

void CAbnormalTitanState_Dead::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CAbnormalTitanState_Dead::Get_DetailState() const
{
    return 0;
}

void CAbnormalTitanState_Dead::Decide_NextState()
{
}

void CAbnormalTitanState_Dead::Decide_NextAnim()
{
}

std::shared_ptr<CAbnormalTitanState_Dead> CAbnormalTitanState_Dead::Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Dead>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}
