#include "AbnormalTitanState_Attack.h"

#include "AnimationClip_Titan.h"
#include "AbnormalTitanStateMachine.h"
#include "TargetSensor.h"
#include "RandomUtil.h"

using namespace ANIM_TITAN;

NS_BEGIN(Client)

CAbnormalTitanState_Attack::CAbnormalTitanState_Attack(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState)
    : CTitanState(goTitan, scTitan, eState)
{
}

CAbnormalTitanState_Attack::~CAbnormalTitanState_Attack()
{
}

HRESULT CAbnormalTitanState_Attack::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goTitan, E_FAIL, "m_goTitan is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_scTitan, E_FAIL, "m_scTitan is nullptr.");

    return S_OK;
}

void CAbnormalTitanState_Attack::Priority_Update(_float fDT)
{
    if (!m_bAcivated)
        return;


    const DISPLACEMENT& tInfo = m_tRef.pSensor->Get_TargetDisplacement();
    m_vAttackDir = tInfo.vDirXZ;
    m_fAttackDist = tInfo.fDist;

    Look_To(XMVector3Normalize(XMLoadFloat3(&m_vAttackDir)), fDT);
}

void CAbnormalTitanState_Attack::Update(_float fDT)
{
    CTitanState::Update(fDT);

}

void CAbnormalTitanState_Attack::Late_Update(_float fDT)
{
    CTitanState::Late_Update(fDT);

    Decide_NextState();
}

void CAbnormalTitanState_Attack::Enter(_uint iDetailFlag)
{
    CTitanState::Enter(iDetailFlag);

    if (iDetailFlag >= To<_uint>(TITAN_MOVE::END))
    {
        cout << "[TITAN_ATTACK] invalid detail flag. fallba\n";
        iDetailFlag = To<_uint>(TITAN_MOVE::WALK);
    }

    m_eMoveState = To<TITAN_MOVE>(iDetailFlag);
    *m_tRef.pPose = TITAN_POSE::STAND;

    if (iDetailFlag == To<_uint>(TITAN_MOVE::WALK))
    {
        cout << "[TITAN_MOVE] ENTER WALK\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_TITAN::RUN_ABNORMAL_3);
    }
}

void CAbnormalTitanState_Attack::Exit()
{
    CTitanState::Exit();
}

void CAbnormalTitanState_Attack::Setup_CachedTitanContext()
{
    CTitanState::Setup_CachedTitanContext();
}

_uint CAbnormalTitanState_Attack::Get_DetailState() const
{
    return To<_uint>(m_eMoveState);
}

void CAbnormalTitanState_Attack::Decide_NextState()
{
    /* 공통 유틸(Detect / Chase / Hurt / Dead 판정)은 추후 분리 예정 */
    //if (m_fElapsedMoveTime >= m_fMaxMoveTime)
    //{
    //    _int iNextAnim = CRandomUtil::Get_Int(0, To<_int>(TITAN_IDLE::END) - 1);
    //    m_tRef.pFSM->Change_State(To<_uint>(TITAN_STATE::IDLE), iNextAnim);
    //    return;
    //}
}

void CAbnormalTitanState_Attack::Decide_NextAnim()
{
}

std::shared_ptr<CAbnormalTitanState_Attack> CAbnormalTitanState_Attack::Create(
    Engine::CGameObject* goTitan,
    CTitan* scTitan,
    TITAN_STATE eState)
{
    auto pInstance = std::make_shared<CAbnormalTitanState_Attack>(goTitan, scTitan, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}

NS_END
