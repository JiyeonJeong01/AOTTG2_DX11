#include "AbnormalTitanState_Idle.h"

#include "AnimationClip_Titan.h"
#include "AbnormalTitanStateMachine.h"
#include "AbnormalTitanStateMachine.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CAbnormalTitanState_Idle::CAbnormalTitanState_Idle(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Idle::~CAbnormalTitanState_Idle()
{
}

HRESULT CAbnormalTitanState_Idle::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Idle::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CAbnormalTitanState_Idle::Update(_float fDT)
{
    CTitanState::Update(fDT);
}

void CAbnormalTitanState_Idle::Late_Update(_float fDT)
{  
    CTitanState::Late_Update(fDT);

    m_fElapsedIdleTime += fDT;

    Decide_NextState();
}

void CAbnormalTitanState_Idle::Enter(_uint iDetailFlag)
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
void CAbnormalTitanState_Idle::Exit()
{
    CTitanState::Exit();
}

void CAbnormalTitanState_Idle::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    if (tContext.pSO)
        m_fMaxIdleTime = tContext.pSO->fMaxIdleTime;
}

void CAbnormalTitanState_Idle::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CAbnormalTitanState_Idle::Get_DetailState() const
{
    return To<_uint>(m_eIdleState);
}

void CAbnormalTitanState_Idle::Decide_NextState()
{
    if (m_fElapsedIdleTime >= m_fMaxIdleTime)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::MOVE), To<_uint>(TITAN_MOVE::WALK));
    }
}

void CAbnormalTitanState_Idle::Decide_NextAnim()
{

}

std::shared_ptr<CAbnormalTitanState_Idle> CAbnormalTitanState_Idle::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Idle>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
