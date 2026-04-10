#include "AbnormalTitanStateMachine.h"

#include "AbnormalTitanState_Idle.h"
#include "AbnormalTitanState_Move.h"
#include "AbnormalTitanState_Chase.h"
#include "AbnormalTitanState_Grab.h"
#include "AbnormalTitanState_Hurt.h"
#include "AbnormalTitanState_Stunned.h"
#include "AbnormalTitanState_AttackEren.h"
#include "AbnormalTitanState_Dead.h"

CAbnormalTitanStateMachine::CAbnormalTitanStateMachine()
{
}

CAbnormalTitanStateMachine::~CAbnormalTitanStateMachine()
{
}

HRESULT CAbnormalTitanStateMachine::Initialize(CGameObject* goTitan, CTitan* scTitan)
{
    m_goTitan = goTitan;
    m_scTitan = scTitan;

    m_States.resize(To<_uint>(TITAN_STATE::END));
    m_States[To<_uint>(TITAN_STATE::IDLE)] = CAbnormalTitanState_Idle::Create(goTitan, scTitan, TITAN_STATE::IDLE);
    m_States[To<_uint>(TITAN_STATE::MOVE)] = CAbnormalTitanState_Move::Create(goTitan, scTitan, TITAN_STATE::MOVE);
    m_States[To<_uint>(TITAN_STATE::CHASE)] = CAbnormalTitanState_Chase::Create(goTitan, scTitan, TITAN_STATE::CHASE);
    m_States[To<_uint>(TITAN_STATE::GRAB)] = CAbnormalTitanState_Grab::Create(goTitan, scTitan, TITAN_STATE::GRAB);
    m_States[To<_uint>(TITAN_STATE::HURT)] = CAbnormalTitanState_Hurt::Create(goTitan, scTitan, TITAN_STATE::HURT);
    m_States[To<_uint>(TITAN_STATE::STUNNED)] = CAbnormalTitanState_Stunned::Create(goTitan, scTitan, TITAN_STATE::STUNNED);
    m_States[To<_uint>(TITAN_STATE::ATTACK_EREN)] = CAbnormalTitanState_AttackEren::Create(goTitan, scTitan, TITAN_STATE::ATTACK_EREN);
    m_States[To<_uint>(TITAN_STATE::DEAD)] = CAbnormalTitanState_Dead::Create(goTitan, scTitan, TITAN_STATE::DEAD);

    m_spCurState = m_States[To<_uint>(TITAN_STATE::IDLE)];

    return S_OK;
}

void CAbnormalTitanStateMachine::Priority_Update(_float fDT)
{
    m_spCurState->Priority_Update(fDT);
}

void CAbnormalTitanStateMachine::Update(_float fDT)
{
    m_spCurState->Update(fDT);
}

void CAbnormalTitanStateMachine::Late_Update(_float fDT)
{
    m_spCurState->Late_Update(fDT);
}

std::unique_ptr<CAbnormalTitanStateMachine> CAbnormalTitanStateMachine::Create(CGameObject* goTitan, CTitan* scTitan)
{
    auto pInstance = std::make_unique<CAbnormalTitanStateMachine>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(goTitan, scTitan), nullptr, "Instance create failed.");
    return pInstance;
}
