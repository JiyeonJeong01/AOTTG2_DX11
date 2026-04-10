#include "AbnormalTitanState_Chase.h"

#include "AbnormalTitan.h"
#include "AnimationClip_Titan.h"
#include "AbnormalTitanStateMachine.h"
#include "TargetSensor.h"
#include "TitanBound_Controller.h"

NS_BEGIN(Client)
using namespace ANIM_TITAN;

CAbnormalTitanState_Chase::CAbnormalTitanState_Chase(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Chase::~CAbnormalTitanState_Chase()
{
}

HRESULT CAbnormalTitanState_Chase::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goOwner is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Chase::Priority_Update(_float fDT)
{
    if (!m_bAcivated)
        return;

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_vChaseDir = tInfo.vDirXZ;
    m_fChaseDist = tInfo.fDist;
}

void CAbnormalTitanState_Chase::Update(_float fDT)
{
    CTitanState::Update(fDT);

    /* 애니메이션 간의 최소 전환 시간 확보 */
    if (m_fGrabAnimCooldownElapsed < m_fGrabAnimCooldown)
    {
        m_fGrabAnimCooldownElapsed += fDT;

        if (m_fGrabAnimCooldownElapsed >= m_fGrabAnimCooldown)
        {
            m_fGrabAnimCooldownElapsed = m_fGrabAnimCooldown;

            /* 애니메이션 실행 중에는 전환 금지 */
            if (m_tRef.pBoundCtlr && !m_bGrabAnimPlaying)
                m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(true);
        }
    }

    /* Pending 된 애니메이션이 있다면 실행 */
    if (!m_bGrabAnimPlaying)
        Try_PlayTriggeredGrabAnim();

    if (m_bGrabAnimPlaying)
        return;

    /* 없다면 타겟을 쫓기 위해 움직이기 */
    _vector vMoveDir = XMLoadFloat3(&m_vChaseDir);
    if (!XMVector3Equal(vMoveDir, XMVectorZero()))
    {
        Look_To(vMoveDir, fDT);
        GroundedMove(vMoveDir, fDT);
    }

    Decide_NextAnim();
}

void CAbnormalTitanState_Chase::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    Decide_NextState();
}

void CAbnormalTitanState_Chase::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);
    *m_tRef.pPose = TITAN_POSE::STAND;

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

void CAbnormalTitanState_Chase::Exit()
{
    if (m_tRef.pBoundCtlr)
    {
        m_tRef.pBoundCtlr->Clear_PendingGrabAnim();
        m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(false);
    }

    CTitanState::Exit();
}

void CAbnormalTitanState_Chase::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CAbnormalTitanState_Chase::On_AnimFinished, this);
}

_uint CAbnormalTitanState_Chase::Get_DetailState() const
{
    return To<_uint>(m_eChaseState);
}

void CAbnormalTitanState_Chase::Decide_NextState()
{
    /* 공통 유틸(Detect 유효성 / Attack 진입 / Hurt / Dead 판정)은 추후 분리 예정 */
}

void CAbnormalTitanState_Chase::Decide_NextAnim()
{
    if (m_bGrabAnimPlaying)
        return;

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::RUN_ABNORMAL_1);
}

/* Pending된 애니메이션이 있다면 조건에 따라 실행하기 */
void CAbnormalTitanState_Chase::Try_PlayTriggeredGrabAnim()
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

    _uint iIndex = m_tComponents.animator.Get_AnimationClipIdx_By_Name(strAnimName);
    if (To<_uint>(INVALID_ANIM_CLIP_INDEX) == iIndex)
        return;

    m_iGrabAnimClip = iIndex;
    m_bGrabAnimPlaying = true;

    m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(false);
    m_tComponents.animator.Set_NextAnimationClip(m_iGrabAnimClip);

    cout << "[TITAN_CHASE] Grab Triggered : " << strAnimName << "\n";
}

void CAbnormalTitanState_Chase::On_SuccessGrabHuman(SIDE eSid, _float3* vGrabPoint, CHuman* pHuman)
{
}

void CAbnormalTitanState_Chase::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
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
        m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(true);

    cout << "[TITAN_CHASE] Grab Animation Finished\n";
}

std::shared_ptr<CAbnormalTitanState_Chase> CAbnormalTitanState_Chase::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Chase>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
