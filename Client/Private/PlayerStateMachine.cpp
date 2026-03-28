#include "PlayerStateMachine.h"

#include "PlayerState_Idle.h"
#include "PlayerState_Jump.h"
#include "PlayerState_Grounded.h"
#include "PlayerState_Airborne.h"

#include "ODM_Gear.h"

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

    /* { IDLE = 0, MOVE, JUMP, AIRBORNE, HOOK, ATTACK, SHOOT, RELOAD, DODGE, RESUPPLY, GRABBED, EMOTE, END } */
    m_States[To<_uint>(PLAYER_STATE::IDLE)] = CPlayerState_Idle::Create(goPlayer, scPlayer);
    m_States[To<_uint>(PLAYER_STATE::JUMP)] = CPlayerState_Jump::Create(goPlayer, scPlayer);
    m_States[To<_uint>(PLAYER_STATE::AIRBORNE)] = CPlayerState_Airborne::Create(goPlayer, scPlayer);

    m_spCurState = m_States[To<_uint>(PLAYER_STATE::IDLE)];

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

void CPlayerStateMachine::Cache_PlayerInfos(const PLAYER_COMPONENTS& tComponents, const PLAYER_RUNTIME_REF& tRef)
{
    for (auto& pState : m_States)
        if (pState)
            pState->Cache_PlayerInfos(tComponents, tRef);

    /* 그래플링/앵커 고정 성공 시 Airborne 상태로 전환하는 이벤트 등록 */
    tRef.pGear->Subscribe_On_Success_Anchored(&CPlayerStateMachine::Change_State, this);
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
