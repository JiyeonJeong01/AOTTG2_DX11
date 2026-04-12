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
    m_vDetectDir = tInfo.vDir;
    m_fChaseDist = tInfo.fDist;
}

void CAbnormalTitanState_Chase::Update(_float fDT)
{
    CTitanState::Update(fDT);

    /* 애니메이션 간의 최소 전환 시간 확보 */
    if (m_fTriggeredGrabAnimCooldownElapsed < m_fTriggeredGrabAnimCooldown)
    {
        m_fTriggeredGrabAnimCooldownElapsed += fDT;

        if (m_fTriggeredGrabAnimCooldownElapsed >= m_fTriggeredGrabAnimCooldown)
        {
            m_fTriggeredGrabAnimCooldownElapsed = m_fTriggeredGrabAnimCooldown;

            /* 애니메이션 실행 중에는 전환 금지 */
            if (m_tRef.pBoundCtlr && !m_bGrabAnimPlaying)
                m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(true);
        }
    }

    if (m_fProximityGrabAnimCooldownElapsed < m_fProximityGrabAnimCooldown)
    {
        m_fProximityGrabAnimCooldownElapsed += fDT;

        if (m_fProximityGrabAnimCooldownElapsed > m_fProximityGrabAnimCooldown)
            m_fProximityGrabAnimCooldownElapsed = m_fProximityGrabAnimCooldown;
    }

    /* Pending 된 애니메이션이 있다면 실행 */
    if (!m_bGrabAnimPlaying)
        Try_PlayTriggeredGrabAnim();

    /* 충분히 가깝지만 충돌 바운드에 걸리지 않은 경우 */
    if (!m_bGrabAnimPlaying && (m_fChaseDist < m_fShouldGrabDist))
        Try_PlayProximityGrabAnim();

    if (m_bGrabAnimPlaying)
        return;

    if (m_fProximityGrabAnimCooldownElapsed < m_fProximityGrabAnimCooldown)
    {
        Decide_NextAnim();
        return;
    }

    /* 없다면 타겟을 쫓기 위해 움직이기 */
    _vector vMoveDir = XMLoadFloat3(&m_vChaseDir);
    if (!XMVector3Equal(vMoveDir, XMVectorZero()))
    {
        vMoveDir = XMVector3Normalize(vMoveDir);

        Look_To(vMoveDir, fDT);

        _vector vLook = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::LOOK) * -1.f);
        _float fDot = XMVectorGetX(XMVector3Dot(vLook, vMoveDir));

        if (m_fChaseDist > m_fStopMoveDist && fDot > 0.3f)
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
    m_fTriggeredGrabAnimCooldownElapsed = m_fTriggeredGrabAnimCooldown;

    if (m_tRef.pBoundCtlr)
    {
        m_tRef.pBoundCtlr->Clear_PendingGrabAnim();
        m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(true);
    }
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
}

void CAbnormalTitanState_Chase::Decide_NextAnim()
{
    if (m_bGrabAnimPlaying)
        return;

    if (m_fProximityGrabAnimCooldownElapsed < m_fProximityGrabAnimCooldown)
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE);
        return;
    }

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::RUN_ABNORMAL_1);
}

/* Pending된 애니메이션이 있다면 조건에 따라 실행하기 */
void CAbnormalTitanState_Chase::Try_PlayTriggeredGrabAnim()
{
    if (!m_tRef.pBoundCtlr)
        return;

    if (m_bGrabAnimPlaying)
        return;

    if (m_fTriggeredGrabAnimCooldownElapsed < m_fTriggeredGrabAnimCooldown)
        return;

    std::string strAnimName;
    if (!m_tRef.pBoundCtlr->Consume_PendingGrabAnim(strAnimName))
        return;

    _uint iIndex = m_tComponents.animator.Get_AnimationClipIdx_By_Name(strAnimName);
    if (To<_uint>(INVALID_ANIM_CLIP_INDEX) == iIndex)
        return;

    if (m_fChaseDist > m_fShouldGrabDist)
        return;

    m_iGrabAnimClip = iIndex;
    m_bGrabAnimPlaying = true;

    m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(false);
    m_tComponents.animator.Set_NextAnimationClip(m_iGrabAnimClip);

    cout << "[TITAN_CHASE] Grab Triggered : " << strAnimName << "\n";
}

void CAbnormalTitanState_Chase::Try_PlayProximityGrabAnim()
{
    _vector vLook = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::LOOK)) * -1.f;
    _vector vRight = XMVector3Normalize(m_tComponents.transform.Get_StateXM(STATE::RIGHT));
    _vector vDir = XMVector3Normalize(XMLoadFloat3(&m_vDetectDir));

    _float fFrontDot = XMVectorGetX(XMVector3Dot(vLook, vDir));
    _float fRightDot = XMVectorGetX(XMVector3Dot(vRight, vDir));
    _float fY = XMVectorGetY(vDir);

    _bool bFrontSide = fFrontDot > 0.f;
    _bool bLeftSide = fRightDot < 0.f;

    const char* pAnim = nullptr;

    if (bFrontSide)
    {
        if (fY > 0.6f)
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_HIGH_L : ANIM_TITAN::GRAB_HIGH_R;
        else if (fY > 0.3f)
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_HEAD_FRONT_1 : ANIM_TITAN::GRAB_HEAD_FRONT_R;
        else if (fY < 0.01f)
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_GROUND_FRONT_L : ANIM_TITAN::GRAB_GROUND_FRONT_R;
        else
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_STOMACH_L : ANIM_TITAN::GRAB_STOMACH_R;
    }
    else
    {
        if (fY > 0.55f)
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_HEAD_BACK_L : ANIM_TITAN::GRAB_HEAD_BACK_R;
        else if (fY > 0.2f)
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_BACK_L : ANIM_TITAN::GRAB_BACK_R;
        else if (fY < -0.25f)
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_GROUND_BACK_L : ANIM_TITAN::GRAB_GROUND_BACK_R;
        else
            pAnim = bLeftSide ? ANIM_TITAN::GRAB_CORE_L : ANIM_TITAN::GRAB_CORE_R;
    }

    if (pAnim == nullptr)
        return;

    _uint iIndex = m_tComponents.animator.Get_AnimationClipIdx_By_Name(pAnim);
    if (To<_uint>(INVALID_ANIM_CLIP_INDEX) == iIndex)
        return;

    m_iGrabAnimClip = iIndex;
    m_bGrabAnimPlaying = true;
    m_fProximityGrabAnimCooldownElapsed = 0.f;

    m_tComponents.animator.Set_NextAnimationClip(m_iGrabAnimClip);
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
    m_fTriggeredGrabAnimCooldownElapsed = 0.f;

    //if (m_tRef.pBoundCtlr)
    //    m_tRef.pBoundCtlr->Set_GrabTriggerEnabled(true);

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
