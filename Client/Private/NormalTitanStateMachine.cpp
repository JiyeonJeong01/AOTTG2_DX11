#include "NormalTitanStateMachine.h"

#include "NormalTitanState_Idle.h"
#include "NormalTitanState_Move.h"
#include "NormalTitanState_Chase.h"
#include "NormalTitanState_Grab.h"
#include "NormalTitanState_Hurt.h"
#include "NormalTitanState_Dead.h"

CNormalTitanStateMachine::CNormalTitanStateMachine()
{
}

CNormalTitanStateMachine::~CNormalTitanStateMachine()
{
}

HRESULT CNormalTitanStateMachine::Initialize(CGameObject* goTitan, CTitan* scTitan)
{
    m_goTitan = goTitan;
    m_scTitan = scTitan;

    m_States.resize(To<_uint>(TITAN_STATE::END));
    m_States[To<_uint>(TITAN_STATE::IDLE)] = CNormalTitanState_Idle::Create(goTitan, scTitan, TITAN_STATE::IDLE);
    m_States[To<_uint>(TITAN_STATE::MOVE)] = CNormalTitanState_Move::Create(goTitan, scTitan, TITAN_STATE::MOVE);
    m_States[To<_uint>(TITAN_STATE::CHASE)] = CNormalTitanState_Chase::Create(goTitan, scTitan, TITAN_STATE::CHASE);
    m_States[To<_uint>(TITAN_STATE::GRAB)] = CNormalTitanState_Grab::Create(goTitan, scTitan, TITAN_STATE::GRAB);
    m_States[To<_uint>(TITAN_STATE::HURT)] = CNormalTitanState_Hurt::Create(goTitan, scTitan, TITAN_STATE::HURT);
    m_States[To<_uint>(TITAN_STATE::DEAD)] = CNormalTitanState_Dead::Create(goTitan, scTitan, TITAN_STATE::DEAD);

    m_spCurState = m_States[To<_uint>(TITAN_STATE::IDLE)];

    return S_OK;
}

void CNormalTitanStateMachine::Priority_Update(_float fDT)
{
    m_spCurState->Priority_Update(fDT);
}

void CNormalTitanStateMachine::Update(_float fDT)
{
    m_spCurState->Update(fDT);
}

void CNormalTitanStateMachine::Late_Update(_float fDT)
{
    m_spCurState->Late_Update(fDT);
}

void CNormalTitanStateMachine::Cache_TitanInfos(const TITAN_CONTEXT& tContext)
{
    for (auto& pState : m_States)
        if (pState)
            pState->Cache_TitanContext(tContext);

    for (auto& pState : m_States)
        if (pState)
            pState->Setup_CachedTitanContext();
}

void CNormalTitanStateMachine::Change_State(_uint iStateKey, _uint iDetailFlag)
{
    if (iStateKey >= m_States.size())
        return;

    m_spCurState->Exit();

    m_spCurState = m_States[iStateKey];
    m_spCurState->Enter(iDetailFlag);

    m_OnChanged_CurState.Invoke(m_spCurState);
}

std::unique_ptr<CNormalTitanStateMachine> CNormalTitanStateMachine::Create(CGameObject* goTitan, CTitan* scTitan)
{
    auto pInstance = std::make_unique<CNormalTitanStateMachine>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(goTitan, scTitan), nullptr, "Instance create failed.");
    return pInstance;
}
