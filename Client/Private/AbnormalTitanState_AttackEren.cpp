#include "AbnormalTitanState_AttackEren.h"

#include "AbnormalTitan.h"
#include "AnimationClip_Titan.h"
#include "AbnormalTitanStateMachine.h"
#include "TargetSensor.h"
#include "HitBox.h"

NS_BEGIN(Client)
using namespace ANIM_TITAN;

CAbnormalTitanState_AttackEren::CAbnormalTitanState_AttackEren(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_AttackEren::~CAbnormalTitanState_AttackEren()
{
}

HRESULT CAbnormalTitanState_AttackEren::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_AttackEren::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!Has_Target())
        return;

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_fAttackDist = tInfo.fDist;
}

void CAbnormalTitanState_AttackEren::Update(_float fDT)
{
    CTitanState::Update(fDT);

    if (!Has_Target())
    {
        Finish_Attack();
        return;
    }

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    const _vector vDir = XMLoadFloat3(&tInfo.vDirXZ);

    if (!XMVector3Equal(vDir, XMVectorZero()))
        Look_To(vDir, fDT);
}

void CAbnormalTitanState_AttackEren::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    UNREFERENCED_PARAMETER(fDT);

    if (!m_bAcivated)
        return;

    if (!Has_Target() && !m_bAttackAnimPlaying)
        Finish_Attack();
}

void CAbnormalTitanState_AttackEren::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);
    *m_tRef.pPose = TITAN_POSE::STAND;

    m_bAttackAnimPlaying = false;
    m_bUsePunch = false;
    m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_fAttackDist = 0.f;

    Try_CachePunchHitBox();
    Set_PunchHitBoxActive(false);

    if (!Has_Target())
    {
        Finish_Attack();
        return;
    }

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_fAttackDist = tInfo.fDist;

    /* 타겟을 보도록 회전 */
    if (!XMVector3Equal(XMLoadFloat3(&tInfo.vDirXZ), XMVectorZero()))
        Look_To(XMLoadFloat3(&tInfo.vDirXZ), 0.f);

    Select_AttackAnim();
}

void CAbnormalTitanState_AttackEren::Exit()
{
    Set_PunchHitBoxActive(false);

    m_bAttackAnimPlaying = false;
    m_bUsePunch = false;
    m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;

    CTitanState::Exit();
}

void CAbnormalTitanState_AttackEren::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(
        &CAbnormalTitanState_AttackEren::On_AnimFinished, this);
}

_uint CAbnormalTitanState_AttackEren::Get_DetailState() const
{
    return 0;
}

void CAbnormalTitanState_AttackEren::Try_CachePunchHitBox()
{
    if (m_pPunchHitBoxL != nullptr && m_pPunchHitBoxR != nullptr)
        return;

    if (m_tRef.pAllHitBoxes == nullptr)
        return;

    auto itL = m_tRef.pAllHitBoxes->find(TITAN_PUNCH_ATTACK_L);
    if (itL != m_tRef.pAllHitBoxes->end())
    {
        m_pPunchHitBoxL = itL->second;
        m_pPunchHitBoxL->Set_Active(false);
    }
    else
    {
        m_pPunchHitBoxL = nullptr;
    }

    auto itR = m_tRef.pAllHitBoxes->find(TITAN_PUNCH_ATTACK_R);
    if (itR != m_tRef.pAllHitBoxes->end())
    {
        m_pPunchHitBoxR = itR->second;
        m_pPunchHitBoxR->Set_Active(false);
    }
    else
    {
        m_pPunchHitBoxR = nullptr;
    }
}

void CAbnormalTitanState_AttackEren::Set_PunchHitBoxActive(_bool bActive)
{
    if (m_pPunchHitBoxL)
        m_pPunchHitBoxL->Set_Active(bActive);
    if (m_pPunchHitBoxR)
        m_pPunchHitBoxR->Set_Active(bActive);
}

void CAbnormalTitanState_AttackEren::Select_AttackAnim()
{
    const char* pAnimName = nullptr;

    if (m_fAttackDist <= m_fPunchAttackRange)
    {
        pAnimName = ANIM_TITAN::ATTACK_COMBO_PUNCH;
        m_bUsePunch = true;
    }
    else
    {
        pAnimName = ANIM_TITAN::ATTACK_THROW;
        m_bUsePunch = false;
    }

    _uint iAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(pAnimName);
    if (iAnimClip == INVALID_ANIM_CLIP_INDEX)
    {
        Finish_Attack();
        return;
    }

    m_iAttackAnimClip = iAnimClip;
    m_bAttackAnimPlaying = true;

    if (m_bUsePunch)
    {
        Set_PunchHitBoxActive(true);
    }
    else
    {
        Attack_Throw();
    }

    m_tComponents.animator.Set_NextAnimationClip(m_iAttackAnimClip);
}

void CAbnormalTitanState_AttackEren::Attack_Throw()
{
}

void CAbnormalTitanState_AttackEren::Finish_Attack()
{
    Set_PunchHitBoxActive(false);

    m_bAttackAnimPlaying = false;
    m_bUsePunch = false;
    m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;

    if (m_tRef.pFSM == nullptr)
        return;

    if (Has_Target())
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
    else
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE)); /* TODO MOVE 가 맞음 */
}

_bool CAbnormalTitanState_AttackEren::Has_Target() const
{
    if (!m_scTitan)
        return false;

    auto scTitan = dynamic_cast<CAbnormalTitan*>(m_scTitan);
    if (!scTitan)
        return false;

    return scTitan->Has_Target();
}

void CAbnormalTitanState_AttackEren::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    if (!m_bAttackAnimPlaying)
        return;

    if (tData.iAnimationClip == INVALID_ANIM_CLIP_INDEX)
        return;

    if (tData.iAnimationClip != m_iAttackAnimClip)
        return;

    Finish_Attack();
}

std::shared_ptr<CAbnormalTitanState_AttackEren> CAbnormalTitanState_AttackEren::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_AttackEren>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
