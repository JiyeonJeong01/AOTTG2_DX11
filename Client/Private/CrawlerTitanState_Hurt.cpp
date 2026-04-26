#include "CrawlerTitanState_Hurt.h"

#include "CrawlerTitan.h"
#include "AnimationClip_Titan.h"
#include "CrawlerTitanStateMachine.h"
#include "TargetSensor.h"

NS_BEGIN(Client)
using namespace ANIM_TITAN;

CCrawlerTitanState_Hurt::CCrawlerTitanState_Hurt(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CCrawlerTitanState_Hurt::~CCrawlerTitanState_Hurt()
{
}

HRESULT CCrawlerTitanState_Hurt::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goOwner is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CCrawlerTitanState_Hurt::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!m_bAcivated)
        return;
}

void CCrawlerTitanState_Hurt::Update(_float fDT)
{
    CTitanState::Update(fDT);

    if (!m_bAcivated)
        return;

    if (m_tComponents.animator->iAnimationClip == m_iHurtAnimClip
        && !m_tHurtFSX.bPlayed
        && m_tComponents.animator.Get_TrackPosition() >= m_tHurtFSX.fTrackPosition)
    {
        SYS_SOUND.PlaySFX(m_wstrSFX, CHANNEL_17, 0.72f);
        m_tHurtFSX.bPlayed = true;
    }

    Decide_NextAnim();
}

void CCrawlerTitanState_Hurt::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    if (!m_bAcivated)
        return;

    Decide_NextState();
}

void CCrawlerTitanState_Hurt::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    cout << "[CRAWLER_HURT] ENTER\n";

    m_iHurtAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_tHurtFSX.bPlayed = false;

    if (iDetailFlag >= To<_uint>(TITAN_HURT::END))
    {
        m_eHurtState = TITAN_HURT::END;
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
        return;
    }

    m_eHurtState = To<TITAN_HURT>(iDetailFlag);
    *m_tRef.pPose = TITAN_POSE::CRAWL;

    m_iHurtAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::CRAWLER_HITEYES);
    if (m_iHurtAnimClip == INVALID_ANIM_CLIP_INDEX)
    {
        cout << "[CRAWLER_HURT] Invalid Hurt Anim : " << ANIM_TITAN::CRAWLER_HITEYES << "\n";

        m_eHurtState = TITAN_HURT::END;
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
        return;
    }

    m_tComponents.animator.Set_NextAnimationClip(m_iHurtAnimClip);
}

void CCrawlerTitanState_Hurt::Exit()
{
    m_eHurtState = TITAN_HURT::END;
    m_iHurtAnimClip = INVALID_ANIM_CLIP_INDEX;

    CTitanState::Exit();
}

void CCrawlerTitanState_Hurt::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    m_wstrSFX = L"Titan_Hurt" + std::to_wstring(tContext.pSO->iHurtSound);
    m_tHurtFSX.iAnimIndex = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::CRAWLER_HITEYES);
    m_tHurtFSX.fTrackPosition = 34.f;
}

void CCrawlerTitanState_Hurt::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CCrawlerTitanState_Hurt::On_AnimFinished, this);
}

_uint CCrawlerTitanState_Hurt::Get_DetailState() const
{
    return To<_uint>(m_eHurtState);
}

void CCrawlerTitanState_Hurt::Decide_NextState()
{
    if (m_eHurtState != TITAN_HURT::END)
        return;

    CGameObject* pTarget = nullptr;
    if (m_tRef.pSensor)
        pTarget = m_tRef.pSensor->Get_Target();

    if (pTarget && pTarget->Is_Valid())
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE), 0);
        return;
    }

    m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
}

void CCrawlerTitanState_Hurt::Decide_NextAnim()
{
    if (m_eHurtState == TITAN_HURT::END)
        return;

    if (m_iHurtAnimClip == INVALID_ANIM_CLIP_INDEX)
        return;
}

void CCrawlerTitanState_Hurt::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex != m_iHurtAnimClip)
        return;

    cout << "[CRAWLER_HURT] Hurt Animation Finished\n";

    m_eHurtState = TITAN_HURT::END;
}

std::shared_ptr<CCrawlerTitanState_Hurt> CCrawlerTitanState_Hurt::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CCrawlerTitanState_Hurt>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
