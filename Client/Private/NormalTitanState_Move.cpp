#include "NormalTitanState_Move.h"

#include "AnimationClip_Titan.h"
#include "NormalTitanStateMachine.h"

#include "NavMesh.h"
#include "RandomUtil.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CNormalTitanState_Move::CNormalTitanState_Move(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CNormalTitanState_Move::~CNormalTitanState_Move()
{
}

HRESULT CNormalTitanState_Move::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CNormalTitanState_Move::Priority_Update(_float fDT)
{
    if (!m_bAcivated)
        return;

    _vector vMoveDir = Get_PatrolMoveDir();
    if (XMVector3Equal(vMoveDir, XMVectorZero()))
        vMoveDir = { 0.f, 0.f, 1.f, 0.f };

    Look_To(vMoveDir, fDT);
    GroundedMove(vMoveDir, fDT);
}

void CNormalTitanState_Move::Update(_float fDT)
{
    CTitanState::Update(fDT);

    m_fElapsedMoveTime += fDT;
}

void CNormalTitanState_Move::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    Decide_NextState();
}

void CNormalTitanState_Move::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);
    /* 정찰할 위치를 navigation 목표로 설정하기 */
    if (!m_pPatrol)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), To<_uint>(TITAN_IDLE::DEFAULT));
        return;
    }

    m_tRef.pNav->Set_TargetPosition(m_tComponents.transform->vPosition, m_pPatrol->Get_CurPatrolPos());

    if (iDetailFlag >= To<_uint>(TITAN_MOVE::END))
    {
        cout << "[TITAN_MOVE] invalid detail flag. fallback -> WALK\n";
        iDetailFlag = To<_uint>(TITAN_MOVE::WALK);
    }

    m_eMoveState = To<TITAN_MOVE>(iDetailFlag);
    *m_tRef.pPose = TITAN_POSE::STAND;

    m_fElapsedMoveTime = 0.f;

    if (iDetailFlag == To<_uint>(TITAN_MOVE::WALK))
    {
        cout << "[TITAN_MOVE] ENTER WALK\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::RUN_WALK);
    }
}

void CNormalTitanState_Move::Exit()
{
    CTitanState::Exit();
}

void CNormalTitanState_Move::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CNormalTitanState_Move::Get_DetailState() const
{
    return To<_uint>(m_eMoveState);
}

void CNormalTitanState_Move::Decide_NextState()
{
    if (m_fElapsedMoveTime >= m_fMaxMoveTime)
    {
        _int iNextAnim = CRandomUtil::Get_Int(0, To<_int>(TITAN_IDLE::END) - 1);
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), iNextAnim);
        return;
    }
}

void CNormalTitanState_Move::Decide_NextAnim()
{
}

std::shared_ptr<CNormalTitanState_Move> CNormalTitanState_Move::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CNormalTitanState_Move>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
