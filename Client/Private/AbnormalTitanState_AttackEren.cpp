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

    m_fOriginRotateSharpness = m_fRotateSharpness;

    m_goThrowRockPool.clear();

    return S_OK;
}

void CAbnormalTitanState_AttackEren::Priority_Update(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    if (!Has_Target())
        return;

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_fDistToEren = tInfo.fDist;

    if (m_bFlushThrow)
    {
        Flush_Throw();
        m_bFlushThrow = false;
        m_bThrown = true;
        m_fElapsedThrownTime = 0.f;
    }
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
    m_fDistToEren = tInfo.fDist;

    const _vector vDir = XMLoadFloat3(&tInfo.vDirXZ);

    if (!XMVector3Equal(vDir, XMVectorZero()))
        Look_To(vDir, fDT);

    /* 공격 애니메이션이 끝난 뒤, 가까우면 idle 유지하며 쿨타임 대기 */
    if (!m_bAttackAnimPlaying)
    {
        if (m_fDistToEren <= m_fStayAttackErenDist)
        {
            m_fElapsedAttackCoolTime += fDT;

            if (m_fElapsedAttackCoolTime >= m_fAttackCoolTime)
            {
                m_fElapsedAttackCoolTime = 0.f;
                Select_AttackAnim();
            }
        }
        else
        {
            Finish_Attack();
        }

        return;
    }

    if (m_eAtkEren == TITAN_ATTACK_EREN::THROW)
    {
        if (m_tComponents.animator->fTrackPosition >= 75.f && !m_bThrown)
        {
            m_bFlushThrow = true;
            Ready_Throw();
        }

        if (m_bThrown)
        {
            m_fElapsedThrownTime += fDT;
            if (m_fElapsedThrownTime > m_fTotalThrownTime)
            {
                for (auto* goObj : m_goThrowRockPool)
                    goObj->Set_Enable(false);

                Finish_Attack();
            }
        }
    }
}

void CAbnormalTitanState_AttackEren::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    UNREFERENCED_PARAMETER(fDT);

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
    m_fDistToEren = 0.f;
    m_bThrown = false;
    m_bFlushThrow = false;
    m_fElapsedThrownTime = 0.f;
    m_fRotateSharpness = m_fAttackRotateSharpness;
    m_fElapsedAttackCoolTime = 0.f;

    Try_CacheHitBox();
    Set_PunchHitBoxActive(false);

    if (!Has_Target())
    {
        Disable_ThrowRocks();
        Finish_Attack();
        return;
    }

    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_fDistToEren = tInfo.fDist;

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

    m_fRotateSharpness = m_fOriginRotateSharpness;
    Disable_ThrowRocks();

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

void CAbnormalTitanState_AttackEren::Cache_TitanContext(const TITAN_CONTEXT& tContext)
{
    CTitanState::Cache_TitanContext(tContext);

    m_fStayAttackErenDist = tContext.fStayAttackErenDist;
}

void CAbnormalTitanState_AttackEren::Try_CacheHitBox()
{
    /* punch */
    if (m_pPunchHitBoxL != nullptr && m_pPunchHitBoxR != nullptr
        && (m_goThrowRockPool.size() == std::size(m_strRocks)))
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

    /* Rock */
    if (m_goThrowRockPool.size() != std::size(m_strRocks))
    {
        m_goThrowRockPool.clear();
        for (const auto& strRock : m_strRocks)
        {
            CGameObject* goRock = nullptr;
            auto itRock = m_tRef.pAllHitBoxes->find(strRock);
            if (itRock != m_tRef.pAllHitBoxes->end())
            {
                goRock = itRock->second->Get_HitBoxObject();
                if (!goRock) continue;
                goRock->Set_Enable(false);
                itRock->second->Set_Active(false);

                m_goThrowRockPool.push_back(goRock);
            }
        }
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

    if (m_fDistToEren <= m_fPunchAttackRange)
    {
        pAnimName = ANIM_TITAN::ATTACK_COMBO_PUNCH;
        m_eAtkEren = TITAN_ATTACK_EREN::PUNCH;
        m_bUsePunch = true;
    }
    else
    {
        pAnimName = ANIM_TITAN::ATTACK_THROW;
        m_eAtkEren = TITAN_ATTACK_EREN::THROW;
        m_bUsePunch = false;
    }

    _uint iAnimClip = m_tComponents.animator.Get_AnimationClipIdx_By_Name(pAnimName);
    if (iAnimClip == INVALID_ANIM_CLIP_INDEX)
    {
        Finish_Attack();
        return;
    }

    m_iAttackAnimClip = iAnimClip;
    m_tComponents.animator.Set_NextAnimationClip(m_iAttackAnimClip);

    if ((m_tComponents.animator->iAnimationClip != m_iAttackAnimClip &&
        m_tComponents.animator->iNextAnimationClip != m_iAttackAnimClip))
    {
        m_bAttackAnimPlaying = true;

        if (m_bUsePunch)
            Set_PunchHitBoxActive(true);
    }

    if (m_bUsePunch)
    {
        Set_PunchHitBoxActive(true);
    }
}

void CAbnormalTitanState_AttackEren::Ready_Throw()
{
    _vector vStartPos = XMLoadFloat3(&m_tComponents.transform->vPosition);
    _vector vUp = { 0.f, 1.f, 0.f, 0.f };
    _vector vLook = m_tComponents.transform.Get_StateXM(STATE::LOOK);

    vStartPos += vUp * m_vRockOffset.y;
    vStartPos += vLook * m_vRockOffset.z;

    for (auto* goObj : m_goThrowRockPool)
        goObj->Set_Enable(true);

    for (const auto& strRock : m_strRocks)
    {
        auto it = m_tRef.pAllHitBoxes->find(strRock);
        if (it == m_tRef.pAllHitBoxes->end())
            continue;
        if (it->second == nullptr)
            continue;
        it->second->Set_Active(true);
    }

    for (_int i = 0; i < m_goThrowRockPool.size(); ++i)
    {
        CGameObject* goRock = m_goThrowRockPool[i];
        if (!goRock)
            continue;

        CTransform trRock = goRock->Get_Component<CTransform>();
        XMStoreFloat3(&trRock->vPosition, vStartPos);

        CRigidbody rbRock = goRock->Get_Component<CRigidbody>();
        if (rbRock.Is_Valid())
        {
            rbRock.Set_LinearVel(_float3(0.f, 0.f, 0.f));
            rbRock.Set_AngularVel(_float3(0.f, 0.f, 0.f));
        }
    }

    m_fElapsedThrownTime = 0.f;
}

void CAbnormalTitanState_AttackEren::Finish_Attack()
{
    Set_PunchHitBoxActive(false);

    m_bAttackAnimPlaying = false;

    m_bUsePunch = false;
    m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
    m_bThrown = false;
    m_bFlushThrow = false;
    m_fElapsedThrownTime = 0.f;

    if (m_tRef.pFSM == nullptr)
        return;

    /* 일정 거리 내에 있다면 상태 전환 없이 idle + 쿨타임 대기 */
    if (m_fDistToEren <= m_fStayAttackErenDist)
    {
        m_fElapsedAttackCoolTime = 0.f;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE);
        return;
    }

    if (Has_Target())
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
    else
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::MOVE)); /* TODO MOVE 가 맞음 */
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

void CAbnormalTitanState_AttackEren::Flush_Throw()
{
    _vector vDirs[20];

    m_fElapsedThrownTime = 0.f;
    Make_ThrowDirs(vDirs, m_tRef.pSensor->Get_TargetDisplacement());

    for (_int i = 0; i < m_goThrowRockPool.size(); ++i)
    {
        CGameObject* goRock = m_goThrowRockPool[i];
        if (!goRock) continue;

        CRigidbody rbRock = goRock->Get_Component<CRigidbody>();

        if (!rbRock.Is_Valid())
            continue;

        _float3 vImpulse;
        XMStoreFloat3(&vImpulse, vDirs[i] * m_fThrowImpulse);

        rbRock.Add_LinearImpulse(vImpulse);
    }
}

void CAbnormalTitanState_AttackEren::Disable_ThrowRocks()
{
    for (_int i = 0; i < m_goThrowRockPool.size(); ++i)
    {
        CGameObject* goRock = m_goThrowRockPool[i];
        if (!goRock)
            continue;

        CRigidbody rbRock = goRock->Get_Component<CRigidbody>();
        if (rbRock.Is_Valid())
        {
            rbRock.Set_LinearVel(_float3(0.f, 0.f, 0.f));
            rbRock.Set_AngularVel(_float3(0.f, 0.f, 0.f));
        }

        goRock->Set_Enable(false);
    }

    for (const auto& strRock : m_strRocks)
    {
        auto it = m_tRef.pAllHitBoxes->find(strRock);
        if (it == m_tRef.pAllHitBoxes->end())
            continue;
        if (it->second == nullptr)
            continue;

        it->second->Set_Active(false);
    }
}

void CAbnormalTitanState_AttackEren::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    const _uint iAnimIdx1 = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_COMBO_PUNCH);
    const _uint iAnimIdx2 = m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_TITAN::ATTACK_THROW);

    if (tData.iAnimationClip != iAnimIdx1 && tData.iAnimationClip != iAnimIdx2)
        return;

    if (tData.iAnimationClip == INVALID_ANIM_CLIP_INDEX)
        return;

    if (m_eAtkEren == TITAN_ATTACK_EREN::THROW)
    {
        /* 돌을 이미 던졌다면, 애니메이션만 끝난 상태로 전환 */
        if (m_bThrown)
        {
            m_bAttackAnimPlaying = false;

            m_bUsePunch = false;
            m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
            m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE);
            return;
        }
    }

    Finish_Attack();
}

void CAbnormalTitanState_AttackEren::Make_ThrowDirs(_vector vDirs[20], const DISPLACEMENT& tInfo)
{
    _vector vForward = XMLoadFloat3(&tInfo.vDir);

    if (XMVectorGetX(XMVector3LengthSq(vForward)) <= 1e-6f)
        vForward = XMVectorSet(0.f, 0.f, 1.f, 0.f);
    else
        vForward = XMVector3Normalize(vForward);

    /* 기준 up */
    _vector vWorldUp = XMVectorSet(0.f, 1.f, 0.f, 0.f);

    /* forward가 up이랑 거의 평행이면 다른 축 사용 */
    if (fabs(XMVectorGetX(XMVector3Dot(vForward, vWorldUp))) > 0.98f)
        vWorldUp = XMVectorSet(1.f, 0.f, 0.f, 0.f);

    /* forward 기준 직교 basis 생성 */
    _vector vRight = XMVector3Normalize(XMVector3Cross(vWorldUp, vForward));
    _vector vUp = XMVector3Normalize(XMVector3Cross(vForward, vRight));

    /* 원뿔 반각 */
    const _float fMaxAngleDeg = 10.f;
    const _float fMaxAngleRad = XMConvertToRadians(fMaxAngleDeg);

    for (_int i = 0; i < 20; ++i)
    {
        /* 0 ~ 1 */
        const _float u1 = static_cast<_float>(rand()) / static_cast<_float>(RAND_MAX);
        const _float u2 = static_cast<_float>(rand()) / static_cast<_float>(RAND_MAX);

        /* cone 내부에서 고르게 퍼지도록 */
        const _float fTheta = XM_2PI * u1;
        const _float fAngle = fMaxAngleRad * sqrtf(u2);

        const _float fSin = sinf(fAngle);
        const _float fCos = cosf(fAngle);

        /* local cone offset */
        _vector vLocal =
            vRight * (cosf(fTheta) * fSin) +
            vUp * (sinf(fTheta) * fSin) +
            vForward * fCos;

        vDirs[i] = XMVector3Normalize(vLocal);
    }
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
