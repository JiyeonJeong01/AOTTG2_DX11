#include "TitanStateMachine.h"
#include "TitanState.h"

CTitanStateMachine::CTitanStateMachine()
{
}

CTitanStateMachine::~CTitanStateMachine()
{
}

void CTitanStateMachine::Cache_TitanInfos(const TITAN_CONTEXT& tContext)
{
    for (auto& pState : m_States)
        if (pState)
            pState->Cache_TitanContext(tContext);

    for (auto& pState : m_States)
        if (pState)
            pState->Setup_CachedTitanContext();
}

void CTitanStateMachine::Change_State(_uint iStateKey, _uint iDetailFlag)
{
    if (iStateKey >= m_States.size())
        return;

    if (!m_spCurState && m_spCurState->Get_State() == TITAN_STATE::DEAD)
        return;

    if (m_spCurState)
        m_spCurState->Exit();

    m_spCurState = m_States[iStateKey];
    m_spCurState->Enter(iDetailFlag);

    m_OnChanged_CurState.Invoke(m_spCurState);
}

std::shared_ptr<CTitanState> CTitanStateMachine::Sync_StateMachine() const
{
    return m_spCurState;
}
