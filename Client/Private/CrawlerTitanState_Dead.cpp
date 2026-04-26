#include "CrawlerTitanState_Dead.h"

#include "AnimationClip_Titan.h"

NS_BEGIN(Client)

CCrawlerTitanState_Dead::CCrawlerTitanState_Dead(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CCrawlerTitanState_Dead::~CCrawlerTitanState_Dead()
{
}

HRESULT CCrawlerTitanState_Dead::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CCrawlerTitanState_Dead::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CCrawlerTitanState_Dead::Update(_float fDT)
{
    CTitanState::Update(fDT);
}

void CCrawlerTitanState_Dead::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);
}

void CCrawlerTitanState_Dead::Enter(_uint iDetailFlag)
{
    UNREFERENCED_PARAMETER(iDetailFlag);

    CTitanState::Enter(iDetailFlag);

    *m_tRef.pPose = TITAN_POSE::CRAWL;
    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::CRAWLER_DIE);

    m_scTitan->Set_Dead();
    //m_tComponents.collider.Set_Enable(false);
}

void CCrawlerTitanState_Dead::Exit()
{
    CTitanState::Exit();
}

void CCrawlerTitanState_Dead::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CCrawlerTitanState_Dead::Get_DetailState() const
{
    return 0;
}

void CCrawlerTitanState_Dead::Decide_NextState()
{
}

void CCrawlerTitanState_Dead::Decide_NextAnim()
{
}

std::shared_ptr<CCrawlerTitanState_Dead> CCrawlerTitanState_Dead::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CCrawlerTitanState_Dead>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
