#include "ErenTitan.h"

#include "AnimationClip_Eren.h"
#include "GroundChecker.h"
#include "TargetSensor.h"
#include "HitBox.h"
#include "HurtBox.h"

NS_BEGIN(Client)

void CErenTitan::Awake(void* pCtx)
{
    m_goEren = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goEren, , "m_goEren is nullptr");

    m_pGroundChecker = m_goEren->Get_Script_InChildren<CGroundChecker>();
    m_pSensor = m_goEren->Get_Script_InChildren<CTargetSensor>();

    m_trEren = m_goEren->Get_Component<CTransform>();
    m_rbEren = m_goEren->Get_Component<CRigidbody>();
    m_animEren = m_goEren->Get_Component<CAnimator>();

    /* null 검사 한 번에 */
    if (m_pGroundChecker == nullptr || m_pSensor == nullptr)
    {
        __debugbreak();
    }

    if (!m_trEren.Is_Valid() || !m_rbEren.Is_Valid() || !m_animEren.Is_Valid())
    {
        __debugbreak();
    }

    m_pSensor->Set_TargetMask(O_TITAN);
    m_pSensor->Subscribe_OnDetectedTarget(&CErenTitan::On_DetectedCombatTargets, this);

    m_animEren->OnAnimationFinished.Add_Listener(&CErenTitan::On_AnimFinished, this);
}

void CErenTitan::Start(void* pCtx)
{
    /* 히트박스 캐싱 */
    auto allHitBoxes = m_goEren->Get_AllScripts_InChildren<CHitBox>();
    for (auto& hit : allHitBoxes)
    {
        CGameObject* goHitBox = hit->Get_HitBoxObject();
        IF_NULL_RETURN_MSG_BREAK(goHitBox, , "goHitBox is nullptr");

        auto [iter, bInserted] = m_AllHitBoxes.emplace(string(goHitBox->Get_Label()), hit);
        IF_TRUE_RETURN_MSG_BREAK(!bInserted, , "duplicated hitbox label");

        hit->Set_TargetMask(O_TITAN);
        hit->Subscribe_OnSuccessHit(&CErenTitan::On_SuccessAttack, this);
    }

    /* 허트박스에 이벤트 등록 */
    auto allHurtBoxes = m_goEren->Get_AllScripts_InChildren<CHurtBox>();
    for (auto& hurt : allHurtBoxes)
    {
        hurt->Subscribe_OnHurt(&CErenTitan::On_Hurt, this);
    }

    /* 히트박스 전부 끄기 */
    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);
}

void CErenTitan::Priority_Update(void* pCtx, _float fDT)
{
}

void CErenTitan::Update(void* pCtx, _float fDT)
{
    switch (m_eStepType)
    {
    case EREN_STEP_TYPE::BORNE :
        Process_Born(fDT);
        break;

    case EREN_STEP_TYPE::COMBAT :
        Process_Combat(fDT);
        break;

    case EREN_STEP_TYPE::MOVE_TO:
        Process_MoveTo(fDT);
        break;

    case EREN_STEP_TYPE::LIFT_ROCK :
        break;

    case EREN_STEP_TYPE::WALK_ROCK :
        return;

    case EREN_STEP_TYPE::FIX_ROCK :
        return;

    case EREN_STEP_TYPE::END :
        return;
    }
}

void CErenTitan::Late_Update(void* pCtx, _float fDT)
{
}

void CErenTitan::Move_To(_fvector vDir, _float fDT, _float fSpeed)
{
    /* 플레이어의 현재 속도 */
    _float3 vLinearVel = m_rbEren.Get_LinearVel();

    _float3 vMoveDir{};
    XMStoreFloat3(&vMoveDir, vDir);

    const _float fMoveLenSq = vMoveDir.x * vMoveDir.x + vMoveDir.z * vMoveDir.z;

    /* 입력 없음 */
    if (fMoveLenSq <= 0.f)
        return;

    _float3 vHorizontalVel{};
    vHorizontalVel.x = vLinearVel.x;
    vHorizontalVel.z = vLinearVel.z;

    const _float fHorizontalSpeedSq =
        vHorizontalVel.x * vHorizontalVel.x +
        vHorizontalVel.z * vHorizontalVel.z;

    /* 최대 속도 제한 */
    if (fHorizontalSpeedSq < m_fMaxSpeed * m_fMaxSpeed)
    {
        _float3 vForce{};
        vForce.x = vMoveDir.x * fSpeed * fSpeed;
        vForce.z = vMoveDir.z * fSpeed * fSpeed;

        m_rbEren.Add_Force(vForce);
    }
}

void CErenTitan::Look_To(_fvector vDir, _float fDT)
{
    if (!m_bYawInitialized)
    {
        _vector vInitLook = m_trEren.Get_StateXM(STATE::LOOK);
        vInitLook = XMVectorSetY(vInitLook, 0.f);

        const _float fEps = 1e-4f;
        if (XMVectorGetX(XMVector3LengthSq(vInitLook)) < fEps)
            vInitLook = XMVectorSet(0.f, 0.f, -1.f, 0.f);
        else
            vInitLook = XMVector3Normalize(vInitLook);

        m_fCurrentYaw = atan2f(XMVectorGetX(vInitLook), XMVectorGetZ(vInitLook));
        m_bYawInitialized = true;
    }

    _vector vTargetLook = vDir;
    vTargetLook *= -1.f;
    vTargetLook = XMVectorSetY(vTargetLook, 0.f);

    const _float fEps = 1e-4f;
    if (XMVectorGetX(XMVector3LengthSq(vTargetLook)) < fEps)
        return;

    vTargetLook = XMVector3Normalize(vTargetLook);

    _float fTargetYaw = atan2f(XMVectorGetX(vTargetLook), XMVectorGetZ(vTargetLook));

    _float fDeltaYaw = fTargetYaw - m_fCurrentYaw;
    while (fDeltaYaw > XM_PI)  fDeltaYaw -= XM_2PI;
    while (fDeltaYaw < -XM_PI) fDeltaYaw += XM_2PI;

    const _float fT = 1.f - expf(-m_fRotateSharpness * fDT);
    _float fNewYaw = m_fCurrentYaw + fDeltaYaw * fT;

    m_fCurrentYaw = fNewYaw;

    _vector qRot = XMQuaternionRotationRollPitchYaw(0.f, fNewYaw, 0.f);
    m_trEren.Set_Rotation_Quaternion(qRot);
}

void CErenTitan::Process_Born(_float fDT)
{
    if (m_eBorn == EREN_BORN::CAN_LAND && m_pGroundChecker->Get_OnWalkable())
    {
        m_eBorn = EREN_BORN::LAND;
        m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::BORN);
    }
}

void CErenTitan::Process_Combat(_float fDT)
{
    if (!Validate_Target())
        return;

    /* 타겟이 없으면 전투 상태는 유지하되 WAIT */
    if (m_goLatestCombatTarget == nullptr || !m_trLastestCombatTarget.Is_Valid())
    {
        m_eCombat = EREN_COMBAT::WAIT;
        return;
    }

    const _vector vDiff =
        XMLoadFloat3(&m_trLastestCombatTarget->vPosition) -
        XMLoadFloat3(&m_trEren->vPosition);

    const _float fDist = XMVectorGetX(XMVector3Length(vDiff));

    _vector vDir = vDiff;
    vDir = XMVectorSetY(vDir, 0.f);

    if (XMVectorGetX(XMVector3LengthSq(vDir)) > 1e-6f)
    {
        vDir = XMVector3Normalize(vDir);
        Look_To(vDir, fDT);
    }

    /* 공격 애니메이션 재생 중이면 끝날 때까지 아무 것도 하지 않음 */
    if (Is_CombatAttacking())
        return;

    m_fElapsedAttackInterval += fDT;

    const EREN_COMBAT_PATTERN& tPattern = m_CombatPattern[m_iCurComboIndex];
    m_fKeepDistance = tPattern.fKeepDistance;

    /* 아직 공격 거리 밖 */
    if (fDist > m_fKeepDistance)
    {
        m_eCombat = EREN_COMBAT::APPROACHING;

        if (fDist > m_fShouldRunDistance)
        {
            m_fCombatSpeed = m_fRunSpeed;
            m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::RUN);
        }
        else
        {
            m_fCombatSpeed = m_fWalkSpeed;
            m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::WALK);
        }

        Move_To(vDir, fDT, m_fCombatSpeed);
        return;
    }

    /* 공격 거리 안이지만 아직 공격 쿨이 아님 -> 뒤로 잠시 빠꾸 */
    if (m_fElapsedAttackInterval < m_fAttackInterval)
    {
        m_eCombat = EREN_COMBAT::APPROACHING;
        m_fCombatSpeed = m_fWalkSpeed;
        return;
    }

    m_fElapsedAttackInterval = 0.f;
    Start_ComboAttack(tPattern.eCombatType);

    ++m_iCurComboIndex;
    if (m_iCurComboIndex >= m_iTotalComboIndex)
        m_iCurComboIndex = 0;
}

void CErenTitan::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (m_eStepType == EREN_STEP_TYPE::BORNE)
    {
        On_AnimBornFinished(iIndex);
    }
    else if (m_eStepType == EREN_STEP_TYPE::COMBAT ||
            m_eStepType == EREN_STEP_TYPE::MOVE_TO)
    {
        On_AnimCombatFinished(iIndex);
    }

}

void CErenTitan::Start_Born()
{
    m_bBornCompleted = false;
    m_eStepType = EREN_STEP_TYPE::BORNE;
    m_eBorn = EREN_BORN::AIR_FALL;
    m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::JUMP_AIR);
}

void CErenTitan::Start_Combat()
{
    m_eStepType = EREN_STEP_TYPE::COMBAT;

    m_CombatPattern.clear();
    {
        {
            EREN_COMBAT_PATTERN kick{};
            kick.eCombatType = EREN_COMBAT::KICK;
            kick.fKeepDistance = 11.f;
            m_CombatPattern.push_back(kick);
        }
        {
            EREN_COMBAT_PATTERN fullCombo{};
            fullCombo.eCombatType = EREN_COMBAT::FULL_COMBO;
            fullCombo.fKeepDistance = 14.f;
            m_CombatPattern.push_back(fullCombo);
        }
        {
            EREN_COMBAT_PATTERN comb1{};
            comb1.eCombatType = EREN_COMBAT::COMBO1;
            comb1.fKeepDistance = 13.f;
            m_CombatPattern.push_back(comb1);
        }
        {
            EREN_COMBAT_PATTERN comb2{};
            comb2.eCombatType = EREN_COMBAT::COMBO2;
            comb2.fKeepDistance = 13.f;
            m_CombatPattern.push_back(comb2);
        }
        {
            EREN_COMBAT_PATTERN comb3{};
            comb3.eCombatType = EREN_COMBAT::COMBO3;
            comb3.fKeepDistance = 13.f;
            m_CombatPattern.push_back(comb3);
        }
    }

    m_iCurComboIndex = 0;
    m_iCurCombatCnt = 0;
    m_iTotalComboIndex = To<_int>(m_CombatPattern.size());
    m_fElapsedAttackInterval = m_fAttackInterval;   /* 시작하자마자 공격 가능하게 */
    m_fKeepDistance = 0.f;
    m_eCombat = EREN_COMBAT::WAIT;
    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);
}

void CErenTitan::Start_MoveTo()
{
    m_eStepType = EREN_STEP_TYPE::MOVE_TO;
    m_bMoveToArrived = false;

    m_CombatPattern.clear();
    {
        {
            EREN_COMBAT_PATTERN kick{};
            kick.eCombatType = EREN_COMBAT::KICK;
            kick.fKeepDistance = 14.5f;
            m_CombatPattern.push_back(kick);
        }
        {
            EREN_COMBAT_PATTERN fullCombo{};
            fullCombo.eCombatType = EREN_COMBAT::FULL_COMBO;
            fullCombo.fKeepDistance = 14.f;
            m_CombatPattern.push_back(fullCombo);
        }
        {
            EREN_COMBAT_PATTERN comb1{};
            comb1.eCombatType = EREN_COMBAT::COMBO1;
            comb1.fKeepDistance = 13.f;
            m_CombatPattern.push_back(comb1);
        }
        {
            EREN_COMBAT_PATTERN comb2{};
            comb2.eCombatType = EREN_COMBAT::COMBO2;
            comb2.fKeepDistance = 13.f;
            m_CombatPattern.push_back(comb2);
        }
        {
            EREN_COMBAT_PATTERN comb3{};
            comb3.eCombatType = EREN_COMBAT::COMBO3;
            comb3.fKeepDistance = 13.f;
            m_CombatPattern.push_back(comb3);
        }
    }

    m_iCurComboIndex = 0;
    m_iTotalComboIndex = To<_int>(m_CombatPattern.size());
    m_fElapsedAttackInterval = m_fAttackInterval;
    m_fKeepDistance = 0.f;
    m_eCombat = EREN_COMBAT::WAIT;

    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);

    m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::RUN);
}

void CErenTitan::Activate_Hitbox(const std::string& strKey, _bool bActive)
{
    if (auto iter = m_AllHitBoxes.find(strKey); iter != m_AllHitBoxes.end())
        iter->second->Set_Active(bActive);
}

void CErenTitan::Process_MoveTo(_float fDT)
{
    /* 현재 공격 애니메이션 재생 중이면 끝날 때까지 유지 */
    if (Is_CombatAttacking())
        return;

    /* 유효한 전투 타겟 갱신 */
    Validate_Target();

    /* 최신 타겟이 있고 충분히 가까우면 잠시 전투 */
    if (m_goLatestCombatTarget != nullptr && m_trLastestCombatTarget.Is_Valid())
    {
        const _vector vTargetDiff = XMLoadFloat3(&m_trLastestCombatTarget->vPosition) - XMLoadFloat3(&m_trEren->vPosition);
        const _float fTargetDist = XMVectorGetX(XMVector3Length(vTargetDiff));

        _vector vTargetDir = XMVectorSetY(vTargetDiff, 0.f);

        if (XMVectorGetX(XMVector3LengthSq(vTargetDir)) > 1e-6f)
        {
            vTargetDir = XMVector3Normalize(vTargetDir);
            Look_To(vTargetDir, fDT);
        }

        /* 공격 시작 가능한 거리 안이면 전투 우선 */
        if (fTargetDist <= m_fShouldAttackDist)
        {
            const EREN_COMBAT_PATTERN& tPattern = m_CombatPattern[m_iCurComboIndex];
            m_fKeepDistance = tPattern.fKeepDistance;

            /* 너무 붙어 있지 않으면 살짝 접근 */
            if (fTargetDist > m_fKeepDistance)
            {
                m_eCombat = EREN_COMBAT::APPROACHING;
                m_fCombatSpeed = m_fWalkSpeed;
                m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::WALK);
                Move_To(vTargetDir, fDT, m_fCombatSpeed * 0.2f);
                return;
            }

            Start_ComboAttack(tPattern.eCombatType);

            ++m_iCurComboIndex;
            if (m_iCurComboIndex >= m_iTotalComboIndex)
                m_iCurComboIndex = 0;

            return;
        }

        /** 한 번 전투 대상으로 잡혔더라도 충분히 멀어졌으면 MOVE_TO 본래 목적 복귀 */
        if (fTargetDist >= m_fMoveToResumeDist)
        {
            m_goLatestCombatTarget = nullptr;
            m_trLastestCombatTarget = {};
            m_eCombat = EREN_COMBAT::WAIT;
        }
    }

    /** 목표 지점으로 이동 */
    const _vector vMoveDiff =
        XMLoadFloat3(&m_vTargetPos) -
        XMLoadFloat3(&m_trEren->vPosition);

    const _float fMoveDist = XMVectorGetX(XMVector3Length(vMoveDiff));

    /** 목표 지점 도착 */
    if (fMoveDist <= m_fMoveToArriveDist)
    {
        m_bMoveToArrived = true;
        m_eCombat = EREN_COMBAT::WAIT;
        m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::IDLE);
        return;
    }

    m_bMoveToArrived = false;

    _vector vMoveDir = XMVectorSetY(vMoveDiff, 0.f);
    if (XMVectorGetX(XMVector3LengthSq(vMoveDir)) <= 1e-6f)
        return;

    vMoveDir = XMVector3Normalize(vMoveDir);

    Look_To(vMoveDir, fDT);

    m_eCombat = EREN_COMBAT::APPROACHING;
    m_fCombatSpeed = m_fRunSpeed;
    m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::RUN);
    Move_To(vMoveDir, fDT, m_fCombatSpeed);
}

_bool CErenTitan::Is_CombatAttacking() const
{
    return m_eCombat == EREN_COMBAT::KICK ||
        m_eCombat == EREN_COMBAT::FULL_COMBO ||
        m_eCombat == EREN_COMBAT::COMBO1 ||
        m_eCombat == EREN_COMBAT::COMBO2 ||
        m_eCombat == EREN_COMBAT::COMBO3;
}

_bool CErenTitan::Validate_Target()
{
    if (m_goLatestCombatTarget != nullptr)
    {
        CTitan* scTitan = m_goLatestCombatTarget->Get_Script_InChildren<CTitan>();
        if (!scTitan)
            return false;

        if (scTitan->Is_Alive() && m_trLastestCombatTarget.Is_Valid())
            return true;

        m_goLatestCombatTarget = nullptr;
    }

    while (!m_goPendingCombatTarget.empty())
    {
        CGameObject* goTarget = m_goPendingCombatTarget.front();
        m_goPendingCombatTarget.pop();

        if (goTarget == nullptr)
            continue;

        CTransform trTarget = goTarget->Get_Component<CTransform>();
        if (!trTarget.Is_Valid())
            continue;

        m_goLatestCombatTarget = goTarget;
        m_trLastestCombatTarget = trTarget;
        return true;
    }

    m_goLatestCombatTarget = nullptr;
    m_trLastestCombatTarget = {};
    return false;
}

void CErenTitan::Start_ComboAttack(EREN_COMBAT eType)
{
    m_eCombat = eType;

    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);

    switch (eType)
    {
    case EREN_COMBAT::FULL_COMBO:
        m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::ATTACK_COMBO_FULL);
        Activate_Hitbox(HAND_R, true);
        Activate_Hitbox(HAND_L, true);
        Activate_Hitbox(LEG_L, true);
        break;

    case EREN_COMBAT::KICK:
        m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::ATTACK_KICK);
        Activate_Hitbox(LEG_L, true);

        break;

    case EREN_COMBAT::COMBO1:
        m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::ATTACK_COMBO_1);
        Activate_Hitbox(HAND_R, true);
        break;

    case EREN_COMBAT::COMBO2:
        m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::ATTACK_COMBO_2);
        Activate_Hitbox(HAND_L, true);
        break;

    case EREN_COMBAT::COMBO3:
        m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::ATTACK_COMBO_3);
        Activate_Hitbox(HAND_R, true);
        break;

    default:
        break;
    }
}

_vector CErenTitan::Get_AttackPower()
{
    _vector vLook = XMVector3Normalize(m_trEren.Get_StateXM(STATE::LOOK)) * -1.f;
    _vector vRight = XMVector3Normalize(m_trEren.Get_StateXM(STATE::RIGHT));
    _vector vWorldUp = { 0.f, 0.2f, 0.f, 0.f };

    _float fBase = 80.f;

    switch (m_eCombat)
    {
    case EREN_COMBAT::FULL_COMBO :
        return XMVector3Normalize(vLook) * fBase;
    case EREN_COMBAT::COMBO1 :
        return XMVector3Normalize(vRight + vLook) * fBase;
    case EREN_COMBAT::COMBO2 :
        return XMVector3Normalize(-1.f * vRight + vWorldUp) * fBase;
    case EREN_COMBAT::COMBO3 :
        return XMVector3Normalize(vRight + vLook) * fBase;
    case EREN_COMBAT::KICK :
        return XMVector3Normalize(vLook) * fBase;
    }

    return XMVector3Normalize(vLook) * fBase;
}

void CErenTitan::On_AnimBornFinished(const _uint iIndex)
{
    if (iIndex == m_animEren->NameToClipIndex[ANIM_EREN_TITAN::JUMP_AIR])
    {
        m_eBorn = EREN_BORN::CAN_LAND;
    }
    else if (iIndex == m_animEren->NameToClipIndex[ANIM_EREN_TITAN::BORN])
    {
        m_eBorn = EREN_BORN::END;
        m_bBornCompleted = true;
    }
}


void CErenTitan::On_AnimCombatFinished(const _uint iIndex)
{
    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);

    const _uint iKick = m_animEren->NameToClipIndex[ANIM_EREN_TITAN::ATTACK_KICK];
    const _uint iFullCombo = m_animEren->NameToClipIndex[ANIM_EREN_TITAN::ATTACK_COMBO_FULL];
    const _uint iComb1 = m_animEren->NameToClipIndex[ANIM_EREN_TITAN::ATTACK_COMBO_1];
    const _uint iComb2 = m_animEren->NameToClipIndex[ANIM_EREN_TITAN::ATTACK_COMBO_2];
    const _uint iComb3 = m_animEren->NameToClipIndex[ANIM_EREN_TITAN::ATTACK_COMBO_3];

    if (iIndex == iComb1)
    {
        Start_ComboAttack(EREN_COMBAT::COMBO2);
        return;
    }

    if (iIndex == iComb2)
    {
        Start_ComboAttack(EREN_COMBAT::COMBO3);
        return;
    }

    if (iIndex == iKick ||
        iIndex == iFullCombo ||
        iIndex == iComb3)
    {
        /** MOVE_TO 중에는 다음 프레임 Process_MoveTo가 다시 판단 */
        if (m_eStepType == EREN_STEP_TYPE::MOVE_TO)
        {
            m_eCombat = EREN_COMBAT::WAIT;
            return;
        }

        if (m_goLatestCombatTarget == nullptr || !m_trLastestCombatTarget.Is_Valid())
        {
            m_eCombat = EREN_COMBAT::WAIT;
            return;
        }

        const _float fDist =
            XMVectorGetX(XMVector3Length( XMLoadFloat3(&m_trLastestCombatTarget->vPosition) - XMLoadFloat3(&m_trEren->vPosition)));

        if (fDist > m_fShouldRunDistance)
        {
            m_eCombat = EREN_COMBAT::APPROACHING;
            m_fCombatSpeed = m_fRunSpeed;
            m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::RUN);
        }
        else if (fDist > m_fKeepDistance)
        {
            m_eCombat = EREN_COMBAT::APPROACHING;
            m_fCombatSpeed = m_fWalkSpeed;
            m_animEren.Set_NextAnimationClip(ANIM_EREN_TITAN::WALK);
        }
        else
        {
            m_eCombat = EREN_COMBAT::WAIT;
        }
    }
}

void CErenTitan::On_DetectedCombatTargets(CGameObject* goTitan)
{
    if (!goTitan->Has_Mask(O_TITAN))
        return;

    if (goTitan->Has_Mask(O_HURTBOX))
        return;

    if (goTitan->Has_Mask(O_HITBOX))
        return;

    /* BORNE이나 COMBAT, MOVE_TO 상태에서 감지된 타겟만 받는다. */
    if (!(m_eStepType == EREN_STEP_TYPE::BORNE ||
        m_eStepType == EREN_STEP_TYPE::COMBAT ||
        m_eStepType == EREN_STEP_TYPE::MOVE_TO))
        return;

    if (m_goLatestCombatTarget)
    {
        /* 이미 타켓이 있다면 push_back */
        m_goPendingCombatTarget.push(goTitan);
        return;
    }

    m_goLatestCombatTarget = goTitan;
    m_trLastestCombatTarget = m_goLatestCombatTarget->Get_Component<CTransform>();
}


void CErenTitan::On_Hurt(const HIT_INFO& tHitInfo, const std::string& strHurtBox)
{
    UNREFERENCED_PARAMETER(tHitInfo);

    /* 다쳤을 때 이벤트 */
}

void CErenTitan::On_SuccessAttack(CGameObject* goTitan)
{
    CTitan* scTitan = goTitan->Get_Script_InChildren<CTitan>();

    if (!scTitan)
    {
        CHurtBox* pHurtBox = goTitan->Get_Script_InChildren<CHurtBox>();
        if (!pHurtBox) return;

        goTitan = pHurtBox->Get_Owner();
        if (!goTitan)  return;
        scTitan = goTitan->Get_Script_InChildren<CTitan>();
        if (!scTitan) return;
    }


    scTitan->On_Stunned();
    CRigidbody rbTitan = goTitan->Get_Component<CRigidbody>();
    if (!rbTitan.Is_Valid())
        return;

    CTransform trTitan = goTitan->Get_Component<CTransform>();
    if (!trTitan.Is_Valid())
        return;

    m_iCurCombatCnt++;

    _float3 vAttack;
    XMStoreFloat3(&vAttack, Get_AttackPower());
    rbTitan.Set_Restitution(1.f);
    rbTitan.Add_LinearImpulse(vAttack);
}

void CErenTitan::Set_ErenStep(EREN_STEP_TYPE eType)
{
}

_bool CErenTitan::Is_BornCompleted() const
{
    return m_bBornCompleted;
}

_int CErenTitan::Get_CurCombatTitans() const
{
    return m_iCurCombatCnt;
}

_bool CErenTitan::Is_LiftCompleted() const
{
    return false;
}

_bool CErenTitan::Is_FixCompleted() const
{
    return false;
}

NS_END;
