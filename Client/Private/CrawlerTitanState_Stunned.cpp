#include "CrawlerTitanState_Stunned.h"

#include "AnimationClip_Titan.h"
#include "CrawlerTitanStateMachine.h"
#include "TargetSensor.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CCrawlerTitanState_Stunned::CCrawlerTitanState_Stunned(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CCrawlerTitanState_Stunned::~CCrawlerTitanState_Stunned()
{
}

HRESULT CCrawlerTitanState_Stunned::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CCrawlerTitanState_Stunned::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CCrawlerTitanState_Stunned::Update(_float fDT)
{
    CTitanState::Update(fDT);
}

void CCrawlerTitanState_Stunned::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    m_fElapsedStunnedTime += fDT;
    Decide_NextState();
}

void CCrawlerTitanState_Stunned::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    (*m_tRef.m_pStunnedAcc)++;

    if (*m_tRef.m_pStunnedAcc > m_pStats->iMaxStunned)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::DEAD), 0);
        return;
    }

    m_iPrevState = iDetailFlag;
    m_fElapsedStunnedTime = 0.f;

    *m_tRef.pPose = TITAN_POSE::CRAWL;

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::CRAWLER_HUNT_DOWN);
}

void CCrawlerTitanState_Stunned::Exit()
{
    CTitanState::Exit();
}

void CCrawlerTitanState_Stunned::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

void CCrawlerTitanState_Stunned::Decide_NextState()
{
    if (m_fElapsedStunnedTime < m_fMaxStunnedTime)
        return;

    _float3 vStunnedVel = m_tComponents.rigidbody.Get_LinearVel();
    const _float fStunnedVel = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&vStunnedVel)));

    if (fStunnedVel < 10.f)
    {
        if (m_tRef.pSensor)
        {
            auto* goTarget = m_tRef.pSensor->Get_Target();
            if (goTarget)
            {
                if (goTarget->Is_ExactMask(O_EREN))
                {
                    m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::ATTACK_EREN));
                    return;
                }
                else
                {
                    m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
                    return;
                }
            }
        }

        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
    }
}

std::shared_ptr<CCrawlerTitanState_Stunned> CCrawlerTitanState_Stunned::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CCrawlerTitanState_Stunned>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
