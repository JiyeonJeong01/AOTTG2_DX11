#include "PlayerStateMachine.h"

#include "PlayerState_Grounded.h"

CPlayerStateMachine::CPlayerStateMachine()
{
}

CPlayerStateMachine::~CPlayerStateMachine()
{
}

HRESULT CPlayerStateMachine::Initialize(CGameObject* goPlayer, CPlayer* scPlayer)
{
    m_goPlayer = goPlayer;
    m_scPlayer = scPlayer;

    m_States.resize(To<_uint>(PLAYER_STATE::END));
    m_spCurState = m_States[To<_uint>(PLAYER_STATE::IDLE)] = CPlayerState_Grounded::Create(goPlayer, scPlayer);

    return S_OK;
}

void CPlayerStateMachine::Priority_Update(_float fDT)
{
    m_spCurState->Priority_Update(fDT);
}

void CPlayerStateMachine::Update(_float fDT)
{
    m_spCurState->Update(fDT);
}

void CPlayerStateMachine::Late_Update(_float fDT)
{
    m_spCurState->Late_Update(fDT);
}

void CPlayerStateMachine::Change_State(_uint iStateKey)
{
    if (iStateKey >= m_States.size())
        return;

    m_spCurState = m_States[iStateKey];
    m_OnChanged_CurState.Invoke(m_spCurState);
}

void CPlayerStateMachine::Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd)
{
    m_spCurState->Update_PlayerInput(tInputCmd);
}

std::unique_ptr<CPlayerStateMachine> CPlayerStateMachine::Create(CGameObject* goPlayer, CPlayer* scPlayer)
{
    auto pInstance = std::make_unique<CPlayerStateMachine>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(goPlayer, scPlayer), nullptr, "Instance create failed.");
    return pInstance;
}
