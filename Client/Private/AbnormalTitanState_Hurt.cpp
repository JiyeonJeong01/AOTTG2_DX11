#include "AbnormalTitanState_Hurt.h"

#include "AbnormalTitan.h"
#include "AnimationClip_Titan.h"
#include "AbnormalTitanStateMachine.h"
#include "TargetSensor.h"

NS_BEGIN(Client)
using namespace ANIM_TITAN;

CAbnormalTitanState_Hurt::CAbnormalTitanState_Hurt(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Hurt::~CAbnormalTitanState_Hurt()
{
}

HRESULT CAbnormalTitanState_Hurt::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goOwner is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Hurt::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!m_bAcivated)
        return;
}

void CAbnormalTitanState_Hurt::Update(_float fDT)
{
    CTitanState::Update(fDT);

    if (!m_bAcivated)
        return;

    if (m_bWaitRecovery)
    {
        m_fRecoveryElapsed += fDT;
        if (m_fRecoveryElapsed >= m_fRecoveryTime)
        {
            m_fRecoveryElapsed = m_fRecoveryTime;
            m_bWaitRecovery = false;
            m_eHurtState = TITAN_HURT::END;
        }
    }

    if (m_tComponents.animator->iAnimationClip == m_iHurtAnimClip
        && !m_tHurtFSX.bPlayed
        && m_tComponents.animator.Get_TrackPosition() >= m_tHurtFSX.fTrackPosition)
    {
        SYS_SOUND.PlaySFX(m_wstrSFX, CHANNEL_17, 0.82f);
        m_tHurtFSX.bPlayed = true;
    }

    Decide_NextAnim();
}

void CAbnormalTitanState_Hurt::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    if (!m_bAcivated)
        return;

    Decide_NextState();
}

void CAbnormalTitanState_Hurt::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    cout << "[TITAN_HURT] ENTER\n";

    m_iHurtAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_bWaitRecovery = false;
    m_fRecoveryElapsed = 0.f;
    m_bLegDownFinished = false;
    m_tHurtFSX.bPlayed = false;

    /* fallback은 모두 IDLE로 */
    if (iDetailFlag >= To<_uint>(TITAN_HURT::END))
    {
        m_eHurtState = TITAN_HURT::END;
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
        return;
    }

    m_eHurtState = To<TITAN_HURT>(iDetailFlag);

    const char* pAnimName = Get_HurtAnimName(m_eHurtState);
    if (nullptr == pAnimName)
    {
        m_eHurtState = TITAN_HURT::END;
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
        return;
    }

    m_iHurtAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(pAnimName);
    if (m_iHurtAnimClip == INVALID_ANIM_CLIP_INDEX)
    {
        cout << "[TITAN_HURT] Invalid Hurt Anim : " << pAnimName << "\n";

        m_eHurtState = TITAN_HURT::END;
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
        return;
    }

    m_tComponents.animator.Set_NextAnimationClip(m_iHurtAnimClip);
}

void CAbnormalTitanState_Hurt::Exit()
{
    m_eHurtState = TITAN_HURT::END;
    m_iHurtAnimClip = INVALID_ANIM_CLIP_INDEX;

    m_bWaitRecovery = false;
    m_fRecoveryElapsed = 0.f;
    m_bLegDownFinished = false;

    CTitanState::Exit();
}

void CAbnormalTitanState_Hurt::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    m_wstrSFX = L"Titan_Hurt" + std::to_wstring(tContext.pSO->iHurtSound);
    m_tHurtFSX.iAnimIndex = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::SIT_HIT_EYE);
    m_tHurtFSX.fTrackPosition = 34.f;
}

void CAbnormalTitanState_Hurt::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CAbnormalTitanState_Hurt::On_AnimFinished, this);
}

_uint CAbnormalTitanState_Hurt::Get_DetailState() const
{
    return To<_uint>(m_eHurtState);
}

void CAbnormalTitanState_Hurt::Decide_NextState()
{
    if (m_eHurtState != TITAN_HURT::END)
        return;

    /* 타겟이 있는 경우 마저 쫓기 */
    CGameObject* pTarget = m_tRef.pSensor->Get_Target();
    if (pTarget && pTarget->Is_Valid())
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE), 0);
        return;
    }

    m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), 0);
}

void CAbnormalTitanState_Hurt::Decide_NextAnim()
{
    if (m_eHurtState == TITAN_HURT::END)
        return;

    if (m_iHurtAnimClip == INVALID_ANIM_CLIP_INDEX)
        return;

    if (m_bWaitRecovery)
    {
        if (Is_ArmHurt(m_eHurtState))
            m_tComponents.animator.Set_NextAnimationClip(IDLE);
        else if (Is_LegHurt(m_eHurtState) && m_bLegDownFinished)
            m_tComponents.animator.Set_NextAnimationClip(SIT_HIT_EYE);
    }
}

void CAbnormalTitanState_Hurt::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex != m_iHurtAnimClip)
        return;

    cout << "[TITAN_HURT] Hurt Animation Finished\n";

    if (Is_EyeHurt(m_eHurtState))
    {
        m_eHurtState = TITAN_HURT::END;
        return;
    }

    if (Is_ArmHurt(m_eHurtState))
    {
        m_bWaitRecovery = true;
        m_fRecoveryElapsed = 0.f;

        m_iHurtAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(IDLE);
        if (m_iHurtAnimClip != INVALID_ANIM_CLIP_INDEX)
            m_tComponents.animator.Set_NextAnimationClip(m_iHurtAnimClip);

        return;
    }

    if (Is_LegHurt(m_eHurtState))
    {
        m_bLegDownFinished = true;
        m_bWaitRecovery = true;
        m_fRecoveryElapsed = 0.f;

        m_iHurtAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(SIT_HIT_EYE);
        if (m_iHurtAnimClip != INVALID_ANIM_CLIP_INDEX)
            m_tComponents.animator.Set_NextAnimationClip(m_iHurtAnimClip);

        return;
    }
}

const char* CAbnormalTitanState_Hurt::Get_HurtAnimName(TITAN_HURT eHurt) const
{
    switch (eHurt)
    {
    case TITAN_HURT::STAND_EYE:
        return HIT_EYE;

    case TITAN_HURT::STAND_ARM_L:
        return ARM_HURT_L;

    case TITAN_HURT::STAND_ARM_R:
        return ARM_HURT_R;

    case TITAN_HURT::STAND_LEG_L:
        return SIT_HUNT_DOWN;

    case TITAN_HURT::STAND_LEG_R:
        return SIT_HUNT_DOWN;

    case TITAN_HURT::SIT_EYE:
        return SIT_HIT_EYE;

    case TITAN_HURT::CRAWL_EYE:
        return CRAWLER_HITEYES;

    default:
        break;
    }

    return nullptr;
}

_bool CAbnormalTitanState_Hurt::Is_LegHurt(TITAN_HURT eHurt) const
{
    return eHurt == TITAN_HURT::STAND_LEG_L ||
        eHurt == TITAN_HURT::STAND_LEG_R;
}

_bool CAbnormalTitanState_Hurt::Is_ArmHurt(TITAN_HURT eHurt) const
{
    return eHurt == TITAN_HURT::STAND_ARM_L ||
        eHurt == TITAN_HURT::STAND_ARM_R;
}

_bool CAbnormalTitanState_Hurt::Is_EyeHurt(TITAN_HURT eHurt) const
{
    return eHurt == TITAN_HURT::STAND_EYE ||
        eHurt == TITAN_HURT::SIT_EYE ||
        eHurt == TITAN_HURT::CRAWL_EYE;
}

std::shared_ptr<CAbnormalTitanState_Hurt> CAbnormalTitanState_Hurt::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Hurt>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
