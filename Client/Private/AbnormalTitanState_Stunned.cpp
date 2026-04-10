#include "AbnormalTitanState_Stunned.h"

#include "NormalTitanState_Stunned.h"

#include "AnimationClip_Titan.h"
#include "NormalTitanStateMachine.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CAbnormalTitanState_Stunned::CAbnormalTitanState_Stunned(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Stunned::~CAbnormalTitanState_Stunned()
{
}

HRESULT CAbnormalTitanState_Stunned::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Stunned::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);
}

void CAbnormalTitanState_Stunned::Update(_float fDT)
{
    CTitanState::Update(fDT);
}

void CAbnormalTitanState_Stunned::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    m_fElapsedStunnedTime += fDT;
    Decide_NextState();
}

void CAbnormalTitanState_Stunned::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    (*m_tRef.m_pStunnedAcc)++;

    if (*m_tRef.m_pStunnedAcc > m_pStats->iMaxStunned)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::DEAD), 0);
        return;
    }

    m_iPrevState = To<_uint>(iDetailFlag);
    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::TITAN_FALLING);
}

void CAbnormalTitanState_Stunned::Exit()
{
    CTitanState::Exit();
}

void CAbnormalTitanState_Stunned::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

void CAbnormalTitanState_Stunned::Decide_NextState()
{
    if (m_fElapsedStunnedTime < m_fMaxStunnedTime)
        return;

    _float3 vStunnedVel = m_tComponents.rigidbody.Get_LinearVel();
    const _float fStunnedVel = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&vStunnedVel)));

    if (fStunnedVel < 10.f)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::ATTACK_EREN));
    }
}

std::shared_ptr<CAbnormalTitanState_Stunned> CAbnormalTitanState_Stunned::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Stunned>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
