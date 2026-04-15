#include "CrawlerTitanState_Move.h"
#include "AnimationClip_Titan.h"
#include "CrawlerTitanStateMachine.h"
#include "NavMesh.h"
#include "TargetSensor.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CCrawlerTitanState_Move::CCrawlerTitanState_Move(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CCrawlerTitanState_Move::~CCrawlerTitanState_Move()
{
}

HRESULT CCrawlerTitanState_Move::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    memset(m_szMoveAnimName, 0, sizeof(m_szMoveAnimName));
    m_fOriginalRotationSharpness = m_fRotateSharpness;
    return S_OK;
}

void CCrawlerTitanState_Move::Priority_Update(_float fDT)
{
    if (!m_bAcivated)
        return;

    _vector vMoveDir = Get_PatrolMoveDir();
    if (XMVector3Equal(vMoveDir, XMVectorZero()))
        vMoveDir = { 0.f, 0.f, 1.f, 0.f };

    Look_To(vMoveDir, fDT);
    GroundedMove(vMoveDir, fDT);
}

void CCrawlerTitanState_Move::Update(_float fDT)
{
    CTitanState::Update(fDT);

    m_fElapsedMoveTime += fDT;
}

void CCrawlerTitanState_Move::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    Decide_NextState();
}

void CCrawlerTitanState_Move::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);
    m_fRotateSharpness = m_fFastRotationSharpness;

    if (!m_pPatrol)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), To<_uint>(TITAN_IDLE::DEFAULT));
        return;
    }

    if (m_tRef.pNav)
        m_tRef.pNav->Set_TargetPosition(m_tComponents.transform->vPosition, m_pPatrol->Get_CurPatrolPos());

    if (iDetailFlag >= To<_uint>(TITAN_MOVE::END))
    {
        cout << "[CRAWLER_MOVE] invalid detail flag. fallback -> WALK\n";
        iDetailFlag = To<_uint>(TITAN_MOVE::WALK);
    }

    m_eMoveState = To<TITAN_MOVE>(iDetailFlag);
    *m_tRef.pPose = TITAN_POSE::CRAWL;

    m_fElapsedMoveTime = 0.f;

    if (m_szMoveAnimName[0] == '\0')
        strncpy_s(m_szMoveAnimName, ANIM_TITAN::CRAWLER_RUN_NEW, _TRUNCATE);

    if (iDetailFlag == To<_uint>(TITAN_MOVE::WALK))
    {
        cout << "[CRAWLER_MOVE] ENTER CRAWLER_RUN_NEW\n";
        m_tComponents.animator.Set_NextAnimationClip(m_szMoveAnimName);
    }
}

void CCrawlerTitanState_Move::Exit()
{
    m_fRotateSharpness = m_fOriginalRotationSharpness;

    CTitanState::Exit();
}

void CCrawlerTitanState_Move::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    if (tContext.pSO)
    {
        m_fMaxMoveTime = tContext.pSO->fMaxMoveTime;
    }

    strncpy_s(m_szMoveAnimName, ANIM_TITAN::CRAWLER_RUN_NEW, _TRUNCATE);
}

void CCrawlerTitanState_Move::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CCrawlerTitanState_Move::Get_DetailState() const
{
    return To<_uint>(m_eMoveState);
}

void CCrawlerTitanState_Move::Decide_NextState()
{
    if (m_tRef.pSensor && m_tRef.pSensor->Has_Target())
        return;

    if (m_fElapsedMoveTime >= m_fMaxMoveTime)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), To<_uint>(TITAN_IDLE::DEFAULT));
        return;
    }
}

void CCrawlerTitanState_Move::Decide_NextAnim()
{
}

std::shared_ptr<CCrawlerTitanState_Move> CCrawlerTitanState_Move::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CCrawlerTitanState_Move>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
