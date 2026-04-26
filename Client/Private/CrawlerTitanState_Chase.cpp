#include "CrawlerTitanState_Chase.h"

#include "CrawlerTitan.h"
#include "AnimationClip_Titan.h"
#include "CrawlerTitanStateMachine.h"
#include "TargetSensor.h"
#include "HitBox.h"

NS_BEGIN(Client)
using namespace ANIM_TITAN;

CCrawlerTitanState_Chase::CCrawlerTitanState_Chase(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CCrawlerTitanState_Chase::~CCrawlerTitanState_Chase()
{
}

HRESULT CCrawlerTitanState_Chase::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goOwner is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    m_fOriginalRotationSharpness = m_fRotateSharpness;

    return S_OK;
}

void CCrawlerTitanState_Chase::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!m_bAcivated)
        return;

    if (!m_tRef.pSensor)
        return;

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_vChaseDir = tInfo.vDirXZ;
    m_vDetectDir = tInfo.vDir;
    m_fChaseDist = tInfo.fDist;

    if (m_fChaseDist > 12.f)
        m_fRotateSharpness = m_fFastRotationSharpness;
    else
        m_fRotateSharpness = m_fOriginalRotationSharpness;
}

void CCrawlerTitanState_Chase::Update(_float fDT)
{
    CTitanState::Update(fDT);

    if (!m_tRef.pSensor)
        return;

    m_goTarget = m_tRef.pSensor->Get_Target();
    if (!m_goTarget)
        return;

    _vector vMoveDir = XMLoadFloat3(&m_vChaseDir);
    if (!XMVector3Equal(vMoveDir, XMVectorZero()))
    {
        vMoveDir = XMVector3Normalize(vMoveDir);

        Look_To(vMoveDir, fDT);

        _vector vLook = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::LOOK) * -1.f);
        _float fDot = XMVectorGetX(XMVector3Dot(vLook, vMoveDir));

        if (m_fChaseDist > m_fStopMoveDist && fDot > 0.2f)
        {
            _float fMoveScale = 1.f;

            if (m_fChaseDist < m_fSlowDownStartDist)
            {
                fMoveScale = (m_fChaseDist - m_fStopMoveDist) / (m_fSlowDownStartDist - m_fStopMoveDist);
                fMoveScale = max(0.f, min(fMoveScale, 1.f));
            }

            GroundedMove(vMoveDir * fMoveScale, fDT);
        }
    }

    Decide_NextAnim();
}

void CCrawlerTitanState_Chase::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    Decide_NextState();
}

void CCrawlerTitanState_Chase::Enter(_uint iDetailFlag)
{
    UNREFERENCED_PARAMETER(iDetailFlag);

    CTitanState::Enter(iDetailFlag);

    *m_tRef.pPose = TITAN_POSE::CRAWL;

    cout << "[CRAWLER_CHASE] ENTER\n";

    Try_CacheHitBox();
    Set_ChaseHitBoxActive(true);

    if (m_tRef.pSensor)
    {
        const auto& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
        m_fChaseDist = tInfo.fDist;
        m_goTarget = m_tRef.pSensor->Get_Target();
    }

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::CRAWLER_RUN_NEW);

    if (!m_bSFXPlayed)
    {
        m_bSFXPlayed = true;
        SYS_SOUND.PlaySFX(m_wstrSFX, CHANNEL_18, 0.6f);
    }
}

void CCrawlerTitanState_Chase::Exit()
{
    Set_ChaseHitBoxActive(false);
    m_fRotateSharpness = m_fOriginalRotationSharpness;

    CTitanState::Exit();
}

void CCrawlerTitanState_Chase::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    m_fStayAttackErenDist = tContext.fStayAttackErenDist;
    m_wstrSFX = L"Titan_Grunt" + std::to_wstring(tContext.pSO->iHurtSound);
}

void CCrawlerTitanState_Chase::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CCrawlerTitanState_Chase::Get_DetailState() const
{
    return To<_uint>(m_eChaseState);
}

void CCrawlerTitanState_Chase::Decide_NextState()
{
    if (!m_tRef.pSensor)
        return;

    CGameObject* goTarget = m_tRef.pSensor->Get_Target();
    if (!goTarget)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
        return;
    }

    if (goTarget->Is_ExactMask(O_EREN))
    {
        if (m_fStayAttackErenDist > m_fChaseDist)
        {
            m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::ATTACK_EREN));
            return;
        }
    }
}

void CCrawlerTitanState_Chase::Decide_NextAnim()
{
    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::CRAWLER_RUN_NEW);
}

void CCrawlerTitanState_Chase::Try_CacheHitBox()
{
    if (m_pHitBoxL != nullptr && m_pHitBoxR != nullptr)
        return;

    if (m_tRef.pAllHitBoxes == nullptr)
        return;

    auto itL = m_tRef.pAllHitBoxes->find(TITAN_PUNCH_ATTACK_L);
    if (itL != m_tRef.pAllHitBoxes->end())
    {
        m_pHitBoxL = itL->second;
        m_pHitBoxL->Set_Active(false);
    }
    else
    {
        m_pHitBoxL = nullptr;
    }

    auto itR = m_tRef.pAllHitBoxes->find(TITAN_PUNCH_ATTACK_R);
    if (itR != m_tRef.pAllHitBoxes->end())
    {
        m_pHitBoxR = itR->second;
        m_pHitBoxR->Set_Active(false);
    }
    else
    {
        m_pHitBoxR = nullptr;
    }
}

void CCrawlerTitanState_Chase::Set_ChaseHitBoxActive(_bool bActive)
{
    if (m_pHitBoxL)
        m_pHitBoxL->Set_Active(bActive);

    if (m_pHitBoxR)
        m_pHitBoxR->Set_Active(bActive);
}


std::shared_ptr<CCrawlerTitanState_Chase> CCrawlerTitanState_Chase::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CCrawlerTitanState_Chase>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
