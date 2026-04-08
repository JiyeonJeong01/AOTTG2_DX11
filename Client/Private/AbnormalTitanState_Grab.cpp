#include "AbnormalTitanState_Grab.h"

#include "AnimationClip_Titan.h"
#include "AbnormalTitanStateMachine.h"
#include "TitanBound_Controller.h"

CAbnormalTitanState_Grab::CAbnormalTitanState_Grab(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Grab::~CAbnormalTitanState_Grab()
{
}

HRESULT CAbnormalTitanState_Grab::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Grab::Priority_Update(_float fDT)
{
    CTitanState::Priority_Update(fDT);

    if (!m_bAcivated)
        return;
}

void CAbnormalTitanState_Grab::Update(_float fDT)
{
    CTitanState::Update(fDT);

    const _vector vOffset = { -0.f, 0.3f, 0.f };
    m_trHuman.Set_Position(XMLoadFloat3(m_pGrabbedPoint) + vOffset);
}

void CAbnormalTitanState_Grab::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    cout << "[TITAN_GRAB] ENTER\n";

    if (m_tComponents.animator->fTrackPosition >= 600)
    {
        CHuman* pHuman = m_tRef.pBoundCtlr->Get_GrabbedHuman();
        if (pHuman)
            pHuman->On_Dead();
    }

    Decide_NextState();
}

void CAbnormalTitanState_Grab::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);
    *m_tRef.pPose = TITAN_POSE::STAND;

    TITAN_GRAB eSide = To<TITAN_GRAB>(iDetailFlag);

    if (eSide == TITAN_GRAB::LEFT)
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::EAT_SLOW_L);
        m_eGrabState = TITAN_GRAB::LEFT;
    }
    else if (eSide == TITAN_GRAB::RIGHT)
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::EAT_SLOW_R);
        m_eGrabState = TITAN_GRAB::RIGHT;
    }
    else
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), To<_uint>(TITAN_IDLE::DEFAULT));
        return;
    }

    CGameObject* pObj = m_tRef.pBoundCtlr->Get_GrabbedObject();
    if (nullptr == pObj)
    {
        m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), To<_uint>(TITAN_IDLE::DEFAULT));
        return;
    }

    m_trHuman = pObj->Get_Component<CTransform>();
    m_pGrabbedPoint = m_tRef.pBoundCtlr->Get_GrabbedPoint();
}

void CAbnormalTitanState_Grab::Exit()
{
    CTitanState::Exit();
}

void CAbnormalTitanState_Grab::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CAbnormalTitanState_Grab::Get_DetailState() const
{
    return To<_uint>(m_eGrabState);
}

void CAbnormalTitanState_Grab::Decide_NextState()
{
}

void CAbnormalTitanState_Grab::Decide_NextAnim()
{
}

void CAbnormalTitanState_Grab::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[TITAN GRAB STATE] On_AnimFinished\n";

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_TITAN::EAT_SLOW_L]
        || iIndex == m_tComponents.animator->NameToClipIndex[ANIM_TITAN::EAT_SLOW_R])
        On_EatSlowFinished(tData);
}

void CAbnormalTitanState_Grab::On_EatSlowFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), To<_uint>(TITAN_IDLE::DEFAULT));
}

std::shared_ptr<CAbnormalTitanState_Grab> CAbnormalTitanState_Grab::Create(Engine::CGameObject* goTitan, CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Grab>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}
