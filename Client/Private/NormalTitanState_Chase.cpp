#include "NormalTitanState_Chase.h"

#include "NormalTitan.h"
#include "AnimationClip_Titan.h"
#include "NormalTitanStateMachine.h"
#include "TargetSensor.h"
#include "TitanBound_Controller.h"

NS_BEGIN(Client)
    using namespace ANIM_TITAN;

CNormalTitanState_Chase::CNormalTitanState_Chase(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CNormalTitanState_Chase::~CNormalTitanState_Chase()
{
}

HRESULT CNormalTitanState_Chase::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CNormalTitanState_Chase::Priority_Update(_float fDT)
{
    if (!m_bAcivated)
        return;

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_vChaseDir = tInfo.vDirXZ;
    m_fChaseDist = tInfo.fDist;
}

void CNormalTitanState_Chase::Update(_float fDT)
{
    CTitanState::Update(fDT);

    if (m_fGrabAnimCooldownElapsed < m_fGrabAnimCooldown)
    {
        m_fGrabAnimCooldownElapsed += fDT;

        if (m_fGrabAnimCooldownElapsed >= m_fGrabAnimCooldown)
        {
            m_fGrabAnimCooldownElapsed = m_fGrabAnimCooldown;

            if (m_tRef.pBoundCtlr && !m_bGrabAnimPlaying)
                m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(true);
        }
    }

    if (!m_bGrabAnimPlaying)
        Try_PlayTriggeredGrabAnim();

    if (m_bGrabAnimPlaying)
        return;

    _vector vMoveDir = XMLoadFloat3(&m_vChaseDir);
    if (!XMVector3Equal(vMoveDir, XMVectorZero()))
    {
        Look_To(vMoveDir, fDT);
        GroundedMove(vMoveDir, fDT);
    }

    Decide_NextAnim();
}

void CNormalTitanState_Chase::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    Decide_NextState();
}

void CNormalTitanState_Chase::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    cout << "[TITAN_CHASE] ENTER\n";

    const auto& tInfo = m_tRef.pSensor->Get_TargetDisplacement();

    m_fChaseDist = tInfo.fDist;

    m_bGrabAnimPlaying = false;
    m_iGrabAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_fGrabAnimCooldownElapsed = m_fGrabAnimCooldown;

    if (m_tRef.pBoundCtlr)
    {
        m_tRef.pBoundCtlr->Clear_PendingGrabAnim();
        m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(true);
    }

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::RUN_WALK);
}

void CNormalTitanState_Chase::Exit()
{
    if (m_tRef.pBoundCtlr)
    {
        m_tRef.pBoundCtlr->Clear_PendingGrabAnim();
        m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(false);
    }

    CTitanState::Exit();
}

void CNormalTitanState_Chase::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

void CNormalTitanState_Chase::Decide_NextState()
{
    /* 공통 유틸(Detect 유효성 / Attack 진입 / Hurt / Dead 판정)은 추후 분리 예정 */
}

void CNormalTitanState_Chase::Decide_NextAnim()
{
    if (m_bGrabAnimPlaying)
        return;

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::RUN_WALK);
}


void CNormalTitanState_Chase::Try_PlayTriggeredGrabAnim()
{
    if (!m_tRef.pBoundCtlr)
        return;

    if (m_bGrabAnimPlaying)
        return;

    if (m_fGrabAnimCooldownElapsed < m_fGrabAnimCooldown)
        return;

    std::string strAnimName;
    if (!m_tRef.pBoundCtlr->Consume_PendingGrabAnim(strAnimName))
        return;

    auto iter = m_tComponents.animator->NameToClipIndex.find(strAnimName);
    if (iter == m_tComponents.animator->NameToClipIndex.end())
        return;

    m_iGrabAnimClip = iter->second;
    m_bGrabAnimPlaying = true;

    m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(false);

    m_tComponents.animator.Set_NextAnimationClip(m_iGrabAnimClip);

    cout << "[TITAN_CHASE] Grab Triggered : " << strAnimName << "\n";
}

void CNormalTitanState_Chase::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex != m_iGrabAnimClip)
        return;

    m_bGrabAnimPlaying = false;
    m_iGrabAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_fGrabAnimCooldownElapsed = 0.f;

    if (m_tRef.pBoundCtlr)
        m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(false);

    cout << "[TITAN_CHASE] Grab Animation Finished\n";
}

std::shared_ptr<CNormalTitanState_Chase> CNormalTitanState_Chase::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CNormalTitanState_Chase>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
