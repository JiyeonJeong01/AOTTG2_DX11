#include "AbnormalTitanState_Move.h"

#include <NavMesh.h>

#include "AnimationClip_Titan.h"
#include "AbnormalTitanStateMachine.h"

#include "RandomUtil.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CAbnormalTitanState_Move::CAbnormalTitanState_Move(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Move::~CAbnormalTitanState_Move()
{
}

HRESULT CAbnormalTitanState_Move::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Move::Priority_Update(_float fDT)
{
    if (!m_bAcivated)
        return;

    const _vector vMoveDir = Get_WanderMoveDir();
    if (!XMVector3Equal(vMoveDir, XMVectorZero()))
        Look_To(vMoveDir, fDT);
}

void CAbnormalTitanState_Move::Update(_float fDT)
{
    CTitanState::Update(fDT);

    m_fElapsedMoveTime += fDT;

    //Move(fDT);
}

void CAbnormalTitanState_Move::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    Decide_NextState();
}

void CAbnormalTitanState_Move::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

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
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::RUN_ABNORMAL_3);
    }
}

void CAbnormalTitanState_Move::Exit()
{
    CTitanState::Exit();
}

void CAbnormalTitanState_Move::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CAbnormalTitanState_Move::Get_DetailState() const
{
    return To<_uint>(m_eMoveState);
}

void CAbnormalTitanState_Move::Decide_NextState()
{
    /* 공통 유틸(Detect / Chase / Hurt / Dead 판정)은 추후 분리 예정 */
    if (m_fElapsedMoveTime >= m_fMaxMoveTime)
    {
        _int iNextAnim = CRandomUtil::Get_Int(0, To<_int>(TITAN_IDLE::END) - 1);
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), iNextAnim);
        return;
    }
}

void CAbnormalTitanState_Move::Decide_NextAnim()
{
}

void CAbnormalTitanState_Move::Move(_float fDT)
{
    const _vector vMoveDir = Get_WanderMoveDir();
    GroundedMove(vMoveDir, fDT);
}

std::shared_ptr<CAbnormalTitanState_Move> CAbnormalTitanState_Move::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Move>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
