#include "CrawlerTitanState_AttackEren.h"

#include "CrawlerTitan.h"
#include "AnimationClip_Titan.h"
#include "CrawlerTitanStateMachine.h"
#include "TargetSensor.h"
#include "GroundChecker.h"
#include "HitBox.h"

NS_BEGIN(Client)
using namespace ANIM_TITAN;

CCrawlerTitanState_AttackEren::CCrawlerTitanState_AttackEren(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CCrawlerTitanState_AttackEren::~CCrawlerTitanState_AttackEren()
{
}

HRESULT CCrawlerTitanState_AttackEren::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    m_fOriginRotateSharpness = m_fRotateSharpness;

    return S_OK;
}

void CCrawlerTitanState_AttackEren::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!Has_Target())
        return;

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_fDistToEren = tInfo.fDist;
}

void CCrawlerTitanState_AttackEren::Update(_float fDT)
{
    CTitanState::Update(fDT);

    if (!Has_Target())
    {
        Finish_Attack();
        return;
    }

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_fDistToEren = tInfo.fDist;

    const _vector vDir = XMLoadFloat3(&tInfo.vDirXZ);
    if (!XMVector3Equal(vDir, XMVectorZero()))
        Look_To(vDir, fDT);

    if (!m_bAttackAnimPlaying)
    {
        if (m_fDistToEren <= m_fJumpAttackRange)
            Play_JumpStart();
        else
            Finish_Attack();

        return;
    }

    if (m_bJumpAir && !m_bJumpLanded)
    {
        if (m_tRef.pGroundChecker && m_tRef.pGroundChecker->Get_OnWalkable())
        {
            Play_JumpLand();
            return;
        }
    }
}

void CCrawlerTitanState_AttackEren::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    UNREFERENCED_PARAMETER(fDT);

    if (!Has_Target() && !m_bAttackAnimPlaying)
        Finish_Attack();
}

void CCrawlerTitanState_AttackEren::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);
    UNREFERENCED_PARAMETER(iDetailFlag);

    *m_tRef.pPose = TITAN_POSE::CRAWL;

    m_bAttackAnimPlaying = false;
    m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_fDistToEren = 0.f;
    m_fRotateSharpness = m_fAttackRotateSharpness;

    m_bBodyHitBoxActive = false;
    m_bJumpStarted = false;
    m_bJumpAir = false;
    m_bJumpLanded = false;

    Try_CacheHitBox();
    Set_BodyHitBoxActive(false);

    if (!Has_Target())
    {
        Finish_Attack();
        return;
    }

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_fDistToEren = tInfo.fDist;

    if (!XMVector3Equal(XMLoadFloat3(&tInfo.vDirXZ), XMVectorZero()))
        Look_To(XMLoadFloat3(&tInfo.vDirXZ), 0.f);

    if (m_fDistToEren <= m_fJumpAttackRange)
        Play_JumpStart();
    else
        Finish_Attack();
}

void CCrawlerTitanState_AttackEren::Exit()
{
    Set_BodyHitBoxActive(false);

    m_bAttackAnimPlaying = false;
    m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_fRotateSharpness = m_fOriginRotateSharpness;

    m_bBodyHitBoxActive = false;
    m_bJumpStarted = false;
    m_bJumpAir = false;
    m_bJumpLanded = false;

    CTitanState::Exit();
}

void CCrawlerTitanState_AttackEren::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(
        &CCrawlerTitanState_AttackEren::On_AnimFinished, this);
}

_uint CCrawlerTitanState_AttackEren::Get_DetailState() const
{
    return 0;
}

void CCrawlerTitanState_AttackEren::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    m_fStayAttackErenDist = tContext.fStayAttackErenDist;
}

void CCrawlerTitanState_AttackEren::Try_CacheHitBox()
{
    if (m_pBodyHitBox != nullptr)
        return;

    if (m_tRef.pAllHitBoxes == nullptr)
        return;

    auto it = m_tRef.pAllHitBoxes->find(TITAN_CRAWLER_BODY);
    if (it != m_tRef.pAllHitBoxes->end())
    {
        m_pBodyHitBox = it->second;
        m_pBodyHitBox->Set_Active(false);
    }
    else
    {
        m_pBodyHitBox = nullptr;
    }
}

void CCrawlerTitanState_AttackEren::Set_BodyHitBoxActive(_bool bActive)
{
    m_bBodyHitBoxActive = bActive;

    if (m_pBodyHitBox)
        m_pBodyHitBox->Set_Active(bActive);
}

void CCrawlerTitanState_AttackEren::Play_JumpStart()
{
    const _uint iAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_CRAWLER_JUMP_0);
    if (iAnimClip == INVALID_ANIM_CLIP_INDEX)
    {
        Finish_Attack();
        return;
    }

    m_iAttackAnimClip = iAnimClip;
    m_bAttackAnimPlaying = true;
    m_bJumpStarted = true;
    m_bJumpAir = false;
    m_bJumpLanded = false;

    Set_BodyHitBoxActive(false);
    m_tComponents.animator.Set_NextAnimationClip(m_iAttackAnimClip);
}

void CCrawlerTitanState_AttackEren::Play_JumpAir()
{
    const _uint iAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_CRAWLER_JUMP_1);
    if (iAnimClip == INVALID_ANIM_CLIP_INDEX)
    {
        Finish_Attack();
        return;
    }

    m_iAttackAnimClip = iAnimClip;
    m_bAttackAnimPlaying = true;
    m_bJumpAir = true;
    m_bJumpLanded = false;

    Set_BodyHitBoxActive(false);
    m_tComponents.animator.Set_NextAnimationClip(m_iAttackAnimClip);
}

void CCrawlerTitanState_AttackEren::Play_JumpLand()
{
    const _uint iAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_CRAWLER_JUMP_2);
    if (iAnimClip == INVALID_ANIM_CLIP_INDEX)
    {
        Finish_Attack();
        return;
    }

    m_iAttackAnimClip = iAnimClip;
    m_bAttackAnimPlaying = true;
    m_bJumpLanded = true;
    m_bJumpAir = false;

    Set_BodyHitBoxActive(true);
    m_tComponents.animator.Set_NextAnimationClip(m_iAttackAnimClip);
}

void CCrawlerTitanState_AttackEren::Finish_Attack()
{
    Set_BodyHitBoxActive(false);

    m_bAttackAnimPlaying = false;
    m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;

    m_bBodyHitBoxActive = false;
    m_bJumpStarted = false;
    m_bJumpAir = false;
    m_bJumpLanded = false;

    if (m_tRef.pFSM == nullptr)
        return;

    if (Has_Target())
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
    else
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE));
}

_bool CCrawlerTitanState_AttackEren::Has_Target() const
{
    if (!m_scTitan)
        return false;

    auto scTitan = dynamic_cast<CCrawlerTitan*>(m_scTitan);
    if (!scTitan)
        return false;

    return scTitan->Has_Target();
}

void CCrawlerTitanState_AttackEren::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    const _uint iJump0 = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_CRAWLER_JUMP_0);
    const _uint iJump1 = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_CRAWLER_JUMP_1);
    const _uint iJump2 = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_CRAWLER_JUMP_2);

    if (tData.iAnimationClip == INVALID_ANIM_CLIP_INDEX)
        return;

    if (tData.iAnimationClip == iJump0)
    {
        Play_JumpAir();
        return;
    }

    if (tData.iAnimationClip == iJump1)
    {
        if (m_tRef.pGroundChecker && m_tRef.pGroundChecker->Get_OnWalkable())
        {
            Play_JumpLand();
        }
        return;
    }

    if (tData.iAnimationClip == iJump2)
    {
        Set_BodyHitBoxActive(false);
        Finish_Attack();
        return;
    }
}

std::shared_ptr<CCrawlerTitanState_AttackEren> CCrawlerTitanState_AttackEren::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CCrawlerTitanState_AttackEren>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
