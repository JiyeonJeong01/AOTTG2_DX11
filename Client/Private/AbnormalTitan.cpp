#include "AbNormalTitan.h"
#include "TitanState.h"
#include "AbnormalTitanStateMachine.h"
#include "AnimationClip_Titan.h"
#include "TargetSensor.h"
#include "TitanBound_Controller.h"
#include "HitBox.h"
#include "HurtBox.h"
#include "NavMesh.h"
#include "Titan_Scriptable_Object.h"
#include "VFX_Manager.h"

NS_BEGIN(Client)

CAbnormalTitan::CAbnormalTitan()
{
}

CAbnormalTitan::~CAbnormalTitan()
{
}

void CAbnormalTitan::Awake(void* pCtx)
{
    m_goTitan = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);
    m_upStateMachine = CAbnormalTitanStateMachine::Create(m_goTitan, this);
    m_upNav = GAME_INSTANCE.Create_NavMesh(L"../../Client/Bin/Assets/DataFiles/NavMesh.dat", true);

    IF_NULL_RETURN_MSG_BREAK(m_upStateMachine, , "m_upStateMachine is nullptr");
}

void CAbnormalTitan::Start(void* pCtx)
{
    {
        CGameObject* goVFX = SYS_GAMEOBJECT.Get_Wrapper(m_refVFXManager.hObject);
        IF_NULL_RETURN_MSG_BREAK(goVFX, , "goVFX is nullptr");
        m_pVFX_Manager = goVFX->Get_Script<CVFX_Manager>();
        IF_NULL_RETURN_MSG_BREAK(m_pVFX_Manager, , "m_pVFX_Manager is nullptr");
    }

    /* 컴포넌트 참조 */
    {
        m_tComponents.transform = m_goTitan->Get_Component<CTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.transform.Is_Valid(), , "transform is invalid");

        m_tComponents.animator = m_goTitan->Get_Component<CAnimator>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.animator.Is_Valid(), , "animator is invalid");

        m_tComponents.collider = m_goTitan->Get_Component<CCollider>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.collider.Is_Valid(), , "collider is invalid");

        m_tComponents.rigidbody = m_goTitan->Get_Component<CRigidbody>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.rigidbody.Is_Valid(), , "rigidbody is invalid");

        m_tComponents.meshRenderer = m_goTitan->Get_Component<CMeshRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_tComponents.meshRenderer.Is_Valid(), , "meshRenderer is invalid");
    }

    /* 런타임 정보 참조 */
    {
        m_tRef.pFSM = m_upStateMachine.get();
        IF_NULL_RETURN_MSG_BREAK(m_tRef.pFSM, , "m_tRef.pFSM is nullptr");

        m_tRef.pSensor = m_goTitan->Get_Script_InChildren<CTargetSensor>();
        IF_NULL_RETURN_MSG_BREAK(m_tRef.pSensor, , "m_tRef.pSensor is nullptr");

        m_tRef.pBoundCtlr = m_goTitan->Get_Script_InChildren<CTitanBound_Controller>();
        IF_NULL_RETURN_MSG_BREAK(m_tRef.pBoundCtlr, , "m_tRef.pBoundCtlr is nullptr");

        /* 히트박스 캐싱 */
        auto allHitBoxes = m_goTitan->Get_AllScripts_InChildren<CHitBox>();
        for (auto& hit : allHitBoxes)
        {
            CGameObject* goHitBox = hit->Get_HitBoxObject();
            IF_NULL_RETURN_MSG_BREAK(goHitBox, , "goHitBox is nullptr");

            ///* ERASE_마스크_설정 */
            //{
            //    if (!goHitBox)
            //        __debugbreak();
            //    auto col = goHitBox->Get_Component<CCollider>();
            //    col->iMask = (O_HITBOX | O_ENEMY);
            //    col->iDiscardMask |= (O_HITBOX | O_ENEMY /*| O_WALKABLE*/);
            //}

            auto [iter, bInserted] = m_AllHitBoxes.emplace(string(goHitBox->Get_Label()), hit);
            IF_TRUE_RETURN_MSG_BREAK(!bInserted, , "duplicated hitbox label");
        }

        /* 허트박스에 이벤트 등록 */
        auto allHurtBoxes = m_goTitan->Get_AllScripts_InChildren<CHurtBox>();
        for (auto& hurt : allHurtBoxes)
        {
            if (!hurt) continue;
            hurt->Subscribe_OnHurt(&CAbnormalTitan::On_Hurt, this);

            ///* ERASE_마스크_설정 */
            //{
            //    auto* goHurt = hurt->Get_HurtBoxObject();
            //    if (!goHurt)
            //        __debugbreak();
            //    auto col = goHurt->Get_Component<CCollider>();
            //    col->iMask |= (O_HURTBOX | O_ENEMY);
            //    col->iDiscardMask |= (O_HURTBOX | O_ENEMY | O_WALKABLE);
            //}

            //if (goHurt && goHurt->Get_Label() == TITAN_WEAK_POINT)
            //{
            //    m_goWeakPoint = goHurt;
            //}
        }
        //IF_NULL_RETURN_MSG_BREAK(m_goWeakPoint, , "m_goWeakPoint is nullptr");

        m_tRef.m_pStunnedAcc = &m_iStunnedAcc;
        m_tRef.pAllHitBoxes = &m_AllHitBoxes;
        m_tRef.pPose = &m_ePose;
        m_tRef.pNav = m_upNav.get();
    }

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE);

    auto* scTitanSO = m_goTitan->Get_Script<CTitan_Scriptable_Object>();
    IF_NULL_RETURN_MSG_BREAK(scTitanSO, , "scTitanSO is nullptr");

    TITAN_SCRIPTABLE_OBJECT tSO{};
    tSO = scTitanSO->Get_Data();
    m_tStats.fCurSpeed = tSO.fCurSpeed;
    m_tStats.fMaxSpeed = tSO.fMaxSpeed;
    m_iHitEffect = tSO.iHitEffect;

    /* 플레이어 상태에게 전달 */
    TITAN_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;
    tContext.pPatrol = &m_tPatrol;
    tContext.pSO = &tSO;

    m_goEren = tSO.goEren;
    IF_NULL_RETURN_MSG_BREAK(m_goEren, , "m_goEren is nullptr");

    m_upStateMachine->Cache_TitanInfos(tContext);
    m_spCurState = m_upStateMachine->Sync_StateMachine();

    m_tRef.pSensor->Set_TargetMask(O_EREN | O_SCOUT | O_PLAYER);
    m_tRef.pSensor->Subscribe_OnDetectedTarget(&CAbnormalTitan::On_DetectedHumanSide, this);
    m_upStateMachine->Subscribe_OnChangedCurState(&CAbnormalTitan::OnChange_CurState, this);

    /* 히트박스 전부 끄기 */
    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);
}

void CAbnormalTitan::Priority_Update(void* pCtx, _float fDT)
{
    Validate_Target();
    m_upStateMachine->Priority_Update(fDT);
}

void CAbnormalTitan::Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Update(fDT);
}

void CAbnormalTitan::Late_Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Late_Update(fDT);
}

TITAN_CONTEXT CAbnormalTitan::Get_TitanContext()
{
    /* 플레이어 상태에게 전달 */
    TITAN_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;
    tContext.pPatrol = &m_tPatrol;

    return tContext;
}

void CAbnormalTitan::Set_Target(Engine::CGameObject* pTarget)
{
    if (m_goTarget == pTarget)
        return;

    m_goTarget = pTarget;

    if (m_tRef.pSensor)
    {
        if (pTarget)
            m_tRef.pSensor->Set_Target(pTarget);
        else
            m_tRef.pSensor->Clear_Target();
    }

    m_OnChanged_Target.Invoke(m_goTarget);
}

void CAbnormalTitan::Clear_Target()
{
    Set_Target(nullptr);
}

_bool CAbnormalTitan::Has_Target() const
{
    return m_goTarget != nullptr;
}

_bool CAbnormalTitan::Is_ValidTarget(Engine::CGameObject* pTarget)
{
    if (!pTarget)
        return false;

    CTransform trTarget = pTarget->Get_Component<CTransform>();
    if (!trTarget.Is_Valid())
        return false;

    return true;
}

CGameObject* CAbnormalTitan::Get_CurTarget() const
{
    return m_goTarget;
}

_bool CAbnormalTitan::Is_Moving()
{
    if (!m_spCurState)
        return false; 
    const auto eState = m_spCurState->Get_State();

    _float fLenSq = XMVectorGetX(XMVector3LengthSq(XMLoadFloat3(&m_tComponents.rigidbody->vLinearVel)));

    return eState == TITAN_STATE::MOVE || (eState == TITAN_STATE::CHASE && fLenSq >= 6.f);
}

void CAbnormalTitan::On_Grab(SIDE eSide, CHuman* pHuman)
{
    TITAN_STATE eState = m_spCurState->Get_State();
    if (eState == TITAN_STATE::GRAB || eState == TITAN_STATE::HURT || eState == TITAN_STATE::DEAD)
        return;

    TITAN_GRAB eGrabbed = TITAN_GRAB::END;
    if (eSide == SIDE::LEFT)
        eGrabbed = TITAN_GRAB::LEFT;
    else if (eSide == SIDE::RIGHT)
        eGrabbed = TITAN_GRAB::RIGHT;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::GRAB), To<_uint>(eGrabbed));
}

void CAbnormalTitan::On_Dead(const _float fAccuracy)
{
    TITAN_STATE eState = m_spCurState->Get_State();
    if (eState == TITAN_STATE::DEAD)
        return;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::DEAD), 0);
}

void CAbnormalTitan::On_Stunned()
{
    TITAN_STATE eState = m_spCurState->Get_State();
    if (eState == TITAN_STATE::STUNNED || eState == TITAN_STATE::DEAD)
        return;

    if (m_goEren)
        Set_Target(m_goEren);

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::STUNNED), To<_uint>(eState));
}

/* hurt는 플레이어와 아군에게만 진입 */
void CAbnormalTitan::On_Hurt(const HIT_INFO& tHitInfo, const std::string& strHurtBox)
{
    UNREFERENCED_PARAMETER(tHitInfo);

    const _int iPlayerAtkMask = O_PLAYER | O_HITBOX;
    const _int iCropsAtkMask = O_SCOUT | O_HITBOX;
    const _int iAttackerMask = tHitInfo.goAttacker->Get_Mask();
    if (!((iPlayerAtkMask == iAttackerMask) || (iCropsAtkMask == iAttackerMask)))
        return;

    TITAN_HURT eHurt = TITAN_HURT::END;

    if (strHurtBox == "HurtBox_Eye")
    {
        if (m_ePose == TITAN_POSE::STAND)
            eHurt = TITAN_HURT::STAND_EYE;
        else if (m_ePose == TITAN_POSE::SIT)
            eHurt = TITAN_HURT::SIT_EYE;
    }
    else if (strHurtBox == "HurtBox_ArmL")
    {
        if (m_ePose == TITAN_POSE::STAND)
            eHurt = TITAN_HURT::STAND_ARM_L;
    }
    else if (strHurtBox == "HurtBox_ArmR")
    {
        if (m_ePose == TITAN_POSE::STAND)
            eHurt = TITAN_HURT::STAND_ARM_R;
    }
    else if (strHurtBox == "HurtBox_LegL")
    {
        if (m_ePose == TITAN_POSE::STAND)
            eHurt = TITAN_HURT::STAND_LEG_L;
    }
    else if (strHurtBox == "HurtBox_LegR")
    {
        if (m_ePose == TITAN_POSE::STAND)
            eHurt = TITAN_HURT::STAND_LEG_R;
    }
    //else if (strHurtBox == TITAN_WEAK_POINT)
    //{
    //    if (tHitInfo.goAttacker->Is_ExactMask(O_PLAYER | O_HITBOX)) /* 플레이어의 공격만 받는다 */
    //    {
    //        CTransform tr = m_goWeakPoint->Get_Component<CTransform>();
    //        const _vector vPoint = XMLoadFloat3(&tr->vPosition);
    //        _vector vDiff = vPoint - XMLoadFloat3(&tHitInfo.vHitPoint);
    //        _float fDiff = XMVectorGetX(XMVector3Length(vDiff));

    //        On_Dead(fDiff);
    //    }

    //    if (m_ePose == TITAN_POSE::STAND)
    //        m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::DEAD), To<_uint>(TITAN_DEAD::STAND_DEAD));
    //    else if (m_ePose == TITAN_POSE::SIT)
    //        m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::DEAD), To<_uint>(TITAN_DEAD::SIT_DEAD));
    //    else if (m_ePose == TITAN_POSE::CRAWL)
    //        m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::DEAD), To<_uint>(TITAN_DEAD::CRAWL_DEAD));
    //    return;
    //}

    if (eHurt == TITAN_HURT::END)
        return;

    m_pVFX_Manager->Play_CombatEffect(To<COMBAT_VFX>(m_iHitEffect), tHitInfo.vHitPoint);
    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::HURT), To<_uint>(eHurt));
}

void CAbnormalTitan::Validate_Target()
{
    if (!m_goTarget)
        return;

    if (!Is_ValidTarget(m_goTarget))
    {
        Clear_Target();
    }
}

void CAbnormalTitan::On_DetectedHumanSide(CGameObject* goHuman)
{
    if (!Is_ValidTarget(goHuman))
        return;

    const uint32_t iNewMask = goHuman->Get_Mask();
    const uint32_t iPrevMask = m_goTarget ? m_goTarget->Get_Mask() : 0;

    if (!(iNewMask == O_PLAYER || iNewMask == O_EREN || iNewMask == O_SCOUT)) /* 타겟이 될 수 있는 대상 */
        return;

    if (Has_Target())
    {
        if (iNewMask >= iPrevMask) /* O_EREN(6) >= O_PLAYER(1) -> 무시 */
            return; /* 다르거나 우선순위가 높은 마스크의 대상에는 타겟 변경 가능 */
    }

    Set_Target(goHuman);
    TITAN_STATE eCur = m_spCurState ? m_spCurState->Get_State() : TITAN_STATE::IDLE;

    if (iNewMask == O_EREN)
    {
        _bool CantAtkEren = eCur == TITAN_STATE::ATTACK_EREN || eCur == TITAN_STATE::DEAD;
        if (!CantAtkEren)
        {
            m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::ATTACK_EREN));
            return;
        }
    }

    _bool bToChase = eCur == TITAN_STATE::IDLE || eCur == TITAN_STATE::MOVE || eCur == TITAN_STATE::ATTACK_EREN;
    if (bToChase)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
    }
}

void CAbnormalTitan::OnChange_CurState(std::shared_ptr<CTitanState> spNewState)
{
    IF_NULL_RETURN_MSG_BREAK(spNewState, , "spNewState is nullptr");

    m_spCurState = spNewState;
    strncpy_s(m_szState, sizeof(m_szState), spNewState->Get_StateName(), _TRUNCATE);
}

NS_END;
