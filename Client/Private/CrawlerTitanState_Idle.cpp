#include "CrawlerTitanState_Idle.h"

#include "AnimationClip_Titan.h"
#include "CrawlerTitanStateMachine.h"

#include "TargetSensor.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CCrawlerTitanState_Idle::CCrawlerTitanState_Idle(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CCrawlerTitanState_Idle::~CCrawlerTitanState_Idle()
{
}

HRESULT CCrawlerTitanState_Idle::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CCrawlerTitanState_Idle::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CCrawlerTitanState_Idle::Update(_float fDT)
{
    CTitanState::Update(fDT);
}

void CCrawlerTitanState_Idle::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    m_fElapsedIdleTime += fDT;

    Decide_NextState();
}

void CCrawlerTitanState_Idle::Enter(_uint iDetailFlag)
{
    UNREFERENCED_PARAMETER(iDetailFlag);

    CTitanState::Enter(iDetailFlag);

    m_fElapsedIdleTime = 0.f;

    cout << "[CRAWLER_IDLE] ENTER CRAWLER_IDLE\n";
    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::CRAWLER_IDLE_NEW);
    *m_tRef.pPose = TITAN_POSE::CRAWL;
}

void CCrawlerTitanState_Idle::Exit()
{
    CTitanState::Exit();
}

void CCrawlerTitanState_Idle::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    if (tContext.pSO)
        m_fMaxIdleTime = tContext.pSO->fMaxIdleTime;
}

void CCrawlerTitanState_Idle::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CCrawlerTitanState_Idle::Get_DetailState() const
{
    return 0;
}

void CCrawlerTitanState_Idle::Decide_NextState()
{
    if (m_tRef.pSensor && m_tRef.pSensor->Has_Target())
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
        return;
    }

    if (m_fElapsedIdleTime >= m_fMaxIdleTime)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::MOVE));
    }
}

void CCrawlerTitanState_Idle::Decide_NextAnim()
{
}

std::shared_ptr<CCrawlerTitanState_Idle> CCrawlerTitanState_Idle::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CCrawlerTitanState_Idle>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
