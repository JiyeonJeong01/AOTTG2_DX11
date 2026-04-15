#include "CrawlerTitanStateMachine.h"

#include "CrawlerTitanState_Idle.h"
#include "CrawlerTitanState_Move.h"
#include "CrawlerTitanState_Chase.h"
#include "CrawlerTitanState_Hurt.h"
#include "CrawlerTitanState_Stunned.h"
#include "CrawlerTitanState_AttackEren.h"
#include "CrawlerTitanState_Dead.h"

CCrawlerTitanStateMachine::CCrawlerTitanStateMachine()
{
}

CCrawlerTitanStateMachine::~CCrawlerTitanStateMachine()
{
}

HRESULT CCrawlerTitanStateMachine::Initialize(CGameObject* goTitan, CTitan* scTitan)
{
    m_goTitan = goTitan;
    m_scTitan = scTitan;

    m_States.resize(To<_uint>(TITAN_STATE::END));
    m_States[To<_uint>(TITAN_STATE::IDLE)] = CCrawlerTitanState_Idle::Create(goTitan, scTitan, TITAN_STATE::IDLE);
    m_States[To<_uint>(TITAN_STATE::MOVE)] = CCrawlerTitanState_Move::Create(goTitan, scTitan, TITAN_STATE::MOVE);
    m_States[To<_uint>(TITAN_STATE::CHASE)] = CCrawlerTitanState_Chase::Create(goTitan, scTitan, TITAN_STATE::CHASE);
    m_States[To<_uint>(TITAN_STATE::HURT)] = CCrawlerTitanState_Hurt::Create(goTitan, scTitan, TITAN_STATE::HURT);
    m_States[To<_uint>(TITAN_STATE::STUNNED)] = CCrawlerTitanState_Stunned::Create(goTitan, scTitan, TITAN_STATE::STUNNED);
    m_States[To<_uint>(TITAN_STATE::ATTACK_EREN)] = CCrawlerTitanState_AttackEren::Create(goTitan, scTitan, TITAN_STATE::ATTACK_EREN);
    m_States[To<_uint>(TITAN_STATE::DEAD)] = CCrawlerTitanState_Dead::Create(goTitan, scTitan, TITAN_STATE::DEAD);

    m_spCurState = m_States[To<_uint>(TITAN_STATE::IDLE)];

    return S_OK;
}

void CCrawlerTitanStateMachine::Priority_Update(_float fDT)
{
    m_spCurState->Priority_Update(fDT);
}

void CCrawlerTitanStateMachine::Update(_float fDT)
{
    m_spCurState->Update(fDT);
}

void CCrawlerTitanStateMachine::Late_Update(_float fDT)
{
    m_spCurState->Late_Update(fDT);
}

std::unique_ptr<CCrawlerTitanStateMachine> CCrawlerTitanStateMachine::Create(CGameObject* goTitan, CTitan* scTitan)
{
    auto pInstance = std::make_unique<CCrawlerTitanStateMachine>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(goTitan, scTitan), nullptr, "Instance create failed.");
    return pInstance;
}
