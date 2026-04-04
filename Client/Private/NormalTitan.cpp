#include "NormalTitan.h"
#include "TitanState.h"
#include "NormalTitanStateMachine.h"
#include "TargetSensor.h"
#include "TitanBound_Controller.h"

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
    }

    /* 플레이어 상태에게 전달 */
    TITAN_CONTEXT tContext;
    tContext.tComponents = m_tComponents;
    tContext.tRef = m_tRef;
    tContext.pStats = &m_tStats;

    m_upStateMachine->Cache_TitanInfos(tContext);

    m_tRef.pSensor->Subscribe_OnDetectedHuman(&CNormalTitan::On_DetectedHuman, this);
    m_upStateMachine->Subscribe_OnChangedCurState(&CNormalTitan::OnChange_CurState, this);

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

    if (!pTarget->Has_Mask(HUMAN))
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
    if (eState == TITAN_STATE::GRAB || eState == TITAN_STATE::DEAD)
        return;

    TITAN_GRAB eGrabbed = TITAN_GRAB::END;
    if (eSide == SIDE::LEFT)
        eGrabbed = TITAN_GRAB::LEFT;
    else if (eSide == SIDE::RIGHT)
        eGrabbed = TITAN_GRAB::RIGHT;

    m_upStateMachine->Change_State(To<_uint>(TITAN_STATE::GRAB), To<_uint>(eGrabbed));
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

void CNormalTitan::On_DetectedHuman(CGameObject* goHuman)
{

    if (Has_Target() || !Is_ValidTarget(goHuman))
        return;

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
