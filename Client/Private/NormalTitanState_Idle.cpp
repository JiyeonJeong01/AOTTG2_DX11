#include "NormalTitanState_Idle.h"

#include "AnimationClip_Titan.h"
#include "NormalTitanStateMachine.h"
#include "NormalTitanStateMachine.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CNormalTitanState_Idle::CNormalTitanState_Idle(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CNormalTitanState_Idle::~CNormalTitanState_Idle()
{
}

HRESULT CNormalTitanState_Idle::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CNormalTitanState_Idle::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CNormalTitanState_Idle::Update(_float fDT)
{
    CTitanState::Update(fDT);
}

void CNormalTitanState_Idle::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    m_fElapsedIdleTime += fDT;

    //Decide_NextState();
}

void CNormalTitanState_Idle::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    const _uint iMaxDetail = To<_uint>(TITAN_IDLE::END);
    if (iDetailFlag >= iMaxDetail)
    {
        cout << "[TITAN_IDLE] invalid detail flag. fallback -> IDLE\n";
        iDetailFlag = To<_uint>(TITAN_IDLE::DEFAULT);
    }

    m_eIdleState = To<TITAN_IDLE>(iDetailFlag);
    m_fElapsedIdleTime = 0.f;

    switch (m_eIdleState)
    {
    case TITAN_IDLE::SIT:
        cout << "[TITAN_IDLE] ENTER SIT_IDLE\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::SIT_IDLE);
        *m_tRef.pPose = TITAN_POSE::SIT;
        break;

    case TITAN_IDLE::DEFAULT:
        cout << "[TITAN_IDLE] ENTER IDLE\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE);
        *m_tRef.pPose = TITAN_POSE::STAND;
        break;

    case TITAN_IDLE::DEFENSE:
        cout << "[TITAN_IDLE] ENTER IDLE_DEFENSE\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE_DEFENSE);
        *m_tRef.pPose = TITAN_POSE::STAND;
        break;

    default:
        cout << "[TITAN_IDLE] unknown detail. fallback -> IDLE\n";
        m_eIdleState = TITAN_IDLE::DEFAULT;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE);
        *m_tRef.pPose = TITAN_POSE::STAND;
        break;
    }
}
void CNormalTitanState_Idle::Exit()
{
    CTitanState::Exit();
}

void CNormalTitanState_Idle::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CNormalTitanState_Idle::Get_DetailState() const
{
    return To<_uint>(m_eIdleState);
}

void CNormalTitanState_Idle::Decide_NextState()
{
    /* 공통 유틸(Detect / Chase / Wander / Hurt / Dead 판정)은 추후 분리 예정 */

    if (m_fElapsedIdleTime >= m_fMaxIdleTime)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::MOVE), To<_uint>(TITAN_MOVE::WALK));
    }
}

void CNormalTitanState_Idle::Decide_NextAnim()
{
    /* idle 내부 변형 애니메이션 선택 로직이 필요해지면 여기서 처리 */
}

std::shared_ptr<CNormalTitanState_Idle> CNormalTitanState_Idle::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CNormalTitanState_Idle>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
