#include "PlayerStateMachine.h"

#include "PlayerState_Idle.h"
#include "PlayerState_Jump.h"
#include "PlayerState_GroundedMove.h"
#include "PlayerState_AirborneMove.h"
#include "PlayerState_GroundedAttack.h"
#include "PlayerState_AirborneAttack.h"

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

    /* { IDLE = 0, GROUNDED_MOVE, JUMP, AIRBORNE_MOVE, HOOK, GROUNDED_ATTACK, AIRBORNE_ATTACK, SHOOT, RELOAD, DODGE, RESUPPLY, GRABBED, EMOTE, END } */
    m_States[To<_uint>(PLAYER_STATE::IDLE)] = CPlayerState_Idle::Create(goPlayer, scPlayer, PLAYER_STATE::IDLE);
    m_States[To<_uint>(PLAYER_STATE::GROUNDED_MOVE)] = CPlayerState_GroundedMove::Create(goPlayer, scPlayer, PLAYER_STATE::GROUNDED_MOVE);
    m_States[To<_uint>(PLAYER_STATE::JUMP)] = CPlayerState_Jump::Create(goPlayer, scPlayer, PLAYER_STATE::JUMP);
    m_States[To<_uint>(PLAYER_STATE::AIRBORNE_MOVE)] = CPlayerState_AirborneMove::Create(goPlayer, scPlayer, PLAYER_STATE::AIRBORNE_MOVE);
    m_States[To<_uint>(PLAYER_STATE::GROUNDED_ATTACK)] = CPlayerState_GroundedAttack::Create(goPlayer, scPlayer, PLAYER_STATE::GROUNDED_ATTACK);
    m_States[To<_uint>(PLAYER_STATE::AIRBORNE_ATTACK)] = CPlayerState_AirborneAttack::Create(goPlayer, scPlayer, PLAYER_STATE::AIRBORNE_ATTACK);

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

void CPlayerStateMachine::Cache_PlayerInfos(const PLAYER_CONTEXT& tContext)
{
    for (auto& pState : m_States)
        if (pState)
            pState->Cache_PlayerContext(tContext);

    /* 그래플링/앵커 고정 성공 시 Airborne 상태로 전환하는 이벤트 등록 */
    if (tContext.tRef.pGear)
        tContext.tRef.pGear->Subscribe_On_Success_Anchored(&CPlayerStateMachine::Change_State, this);

    for (auto& pState : m_States)
        if (pState)
            pState->Setup_CachedPlayerInfos();
}


void CPlayerStateMachine::Change_State(_uint iStateKey, _uint iDetailFlag)
{
    if (iStateKey >= m_States.size())
        return;

    m_spCurState->Exit();

    m_spCurState = m_States[iStateKey];

    /* NOTE !! 입력 상태 최신화 필수 */
    m_spCurState->Update_PlayerInput(m_tInputCmd);
    m_spCurState->Enter(iDetailFlag);

    m_OnChanged_CurState.Invoke(m_spCurState);
}

void CPlayerStateMachine::Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd)
{
    m_tInputCmd = tInputCmd;
    m_spCurState->Update_PlayerInput(tInputCmd);
}

std::unique_ptr<CPlayerStateMachine> CPlayerStateMachine::Create(CGameObject* goPlayer, CPlayer* scPlayer)
{
    auto pInstance = std::make_unique<CPlayerStateMachine>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(goPlayer, scPlayer), nullptr, "Instance create failed.");
    return pInstance;
}
