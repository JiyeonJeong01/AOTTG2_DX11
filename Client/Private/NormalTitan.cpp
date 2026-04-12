#include "NormalTitan.h"

#include "AnimationClip_Titan.h"
#include "TitanState.h"
#include "NormalTitanStateMachine.h"
#include "TargetSensor.h"
#include "TitanBound_Controller.h"
#include "HitBox.h"
#include "HurtBox.h"
#include "NavMesh.h"

NS_BEGIN(Client)
    CNormalTitan::CNormalTitan()
{
}

CNormalTitan::~CNormalTitan()
{
}

void CNormalTitan::Awake(void* pCtx)
{
    m_goTitan = SYS_GAMEOBJECT.Get_Wrapper(m_hObject);
    m_upStateMachine = CNormalTitanStateMachine::Create(m_goTitan, this);
    m_upNav = GAME_INSTANCE.Create_NavMesh(L"../../Client/Bin/Assets/DataFiles/NavMesh.dat");

    IF_NULL_RETURN_MSG_BREAK(m_upStateMachine, , "m_upStateMachine is nullptr");
}

void CNormalTitan::Start(void* pCtx)
{
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


        m_tRef.pNav = m_upNav.get();

        /* 히트박스 캐싱 */
        auto allHitBoxes = m_goTitan->Get_AllScripts_InChildren<CHitBox>();
        for (auto& hit : allHitBoxes)
        {
            CGameObject* goHitBox = hit->Get_HitBoxObject();
            IF_NULL_RETURN_MSG_BREAK(goHitBox, , "goHitBox is nullptr");

            auto [iter, bInserted] = m_AllHitBoxes.emplace(string(goHitBox->Get_Label()), hit);
            IF_TRUE_RETURN_MSG_BREAK(!bInserted, , "duplicated hitbox label");
        }

        /* 허트박스에 이벤트 등록 */
        auto allHurtBoxes = m_goTitan->Get_AllScripts_InChildren<CHurtBox>();
        for (auto& hurt : allHurtBoxes)
        {
            hurt->Subscribe_OnHurt(&CNormalTitan::On_Hurt, this);
        }

        m_tRef.pAllHitBoxes = &m_AllHitBoxes;

        m_tRef.pPose = &m_ePose;
    }

    m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::IDLE);

    /* 플레이어 상태에게 전달 */
    m_tStats.fCurSpeed = 8.f;
    m_tStats.fMaxSpeed = 8.f;

    TITAN_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;
    tContext.pPatrol = &m_tPatrol;
    m_tRef.m_pStunnedAcc = &m_iStunnedAcc;

    m_upStateMachine->Cache_TitanInfos(tContext);
    m_spCurState = m_upStateMachine->Sync_StateMachine();

    m_tRef.pSensor->Set_TargetMask(O_CROPS | O_PLAYER);
    m_tRef.pSensor->Subscribe_OnDetectedTarget(&CNormalTitan::On_DetectedHumanSide, this);

    m_upStateMachine->Subscribe_OnChangedCurState(&CNormalTitan::OnChange_CurState, this);

    /* 히트박스 전부 끄기 */
    for (auto& hit : m_AllHitBoxes)
        hit.second->Set_Active(false);
}

void CNormalTitan::Priority_Update(void* pCtx, _float fDT)
{
    Validate_Target();
    m_upStateMachine->Priority_Update(fDT);
}

void CNormalTitan::Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Update(fDT);
}

void CNormalTitan::Late_Update(void* pCtx, _float fDT)
{
    m_upStateMachine->Late_Update(fDT);
}

TITAN_CONTEXT CNormalTitan::Get_TitanContext()
{
    /* 플레이어 상태에게 전달 */
    TITAN_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;
    tContext.pPatrol = &m_tPatrol;

    return tContext;
}

void CNormalTitan::Set_Target(Engine::CGameObject* pTarget)
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

void CNormalTitan::Clear_Target()
{
    Set_Target(nullptr);
}

_bool CNormalTitan::Has_Target() const
{
    return m_goTarget != nullptr;
}

_bool CNormalTitan::Is_ValidTarget(Engine::CGameObject* pTarget)
{
    if (!pTarget)
        return false;

    if (!pTarget->Is_ExactMask(O_PLAYER) && !pTarget->Is_ExactMask(O_CROPS))
        return false;

    CTransform trTarget = pTarget->Get_Component<CTransform>();
    if (!trTarget.Is_Valid())
        return false;

    return true;
}

CGameObject* CNormalTitan::Get_CurTarget() const
{
    return m_goTarget;
}

void CNormalTitan::On_Grab(SIDE eSide, CHuman* pHuman)
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
    pHuman->On_Grabbed(eSide, this);
}

void CNormalTitan::On_Dead(const _float fAccuracy)
{
    TITAN_STATE eState = m_spCurState->Get_State();
    if (eState == TITAN_STATE::DEAD)
        return;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::DEAD), 0);
}

void CNormalTitan::On_Stunned()
{
    TITAN_STATE eState = m_spCurState->Get_State();
    if (eState == TITAN_STATE::DEAD)
        return;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::STUNNED), To<_uint>(eState));
}

void CNormalTitan::On_Hurt(const HIT_INFO& tHitInfo, const std::string& strHurtBox)
{
    UNREFERENCED_PARAMETER(tHitInfo);

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

    if (eHurt == TITAN_HURT::END)
        return;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::HURT), To<_uint>(eHurt));
}

void CNormalTitan::Validate_Target()
{
    if (!m_goTarget)
        return;

    if (!Is_ValidTarget(m_goTarget))
    {
        Clear_Target();
    }
}

void CNormalTitan::On_DetectedHumanSide(CGameObject* goHuman)
{
    if (!Is_ValidTarget(goHuman))
        return;

    if (goHuman->Has_Mask(O_EREN))
    {
        return;
    }

    if (Has_Target())
    {
        if (m_goTarget->Get_Mask() == goHuman->Get_Mask())
            return;
    }

    Set_Target(goHuman);

    TITAN_STATE eCur = m_spCurState ? m_spCurState->Get_State() : TITAN_STATE::IDLE;
    _bool bToChase = eCur == TITAN_STATE::IDLE || eCur == TITAN_STATE::MOVE;
    if (bToChase)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::CHASE));
    }
}

void CNormalTitan::OnChange_CurState(std::shared_ptr<CTitanState> spNewState)
{
    IF_NULL_RETURN_MSG_BREAK(spNewState, , "spNewState is nullptr");

    m_spCurState = spNewState;
    strncpy_s(m_szState, sizeof(m_szState), spNewState->Get_StateName(), _TRUNCATE);
}

NS_END;
