#include "PlayerState_GroundedMove.h"

#include "Logger.h"
#include "GameObject.h"
#include "PlayerStateMachine.h"
#include "AnimationClip_Player.h"
#include "ODM_Gear.h"

CPlayerState_GroundedMove::CPlayerState_GroundedMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_GroundedMove::~CPlayerState_GroundedMove()
{
}

HRESULT CPlayerState_GroundedMove::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");

    return S_OK;
}

void CPlayerState_GroundedMove::Priority_Update(_float fDT)
{
    Control_Camera();
    LookTo_InputDir(fDT);
}

void CPlayerState_GroundedMove::Update(_float fDT)
{
    Move(fDT);
}

void CPlayerState_GroundedMove::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    Decide_NextState();
}

void CPlayerState_GroundedMove::Decide_NextState()
{
    if (m_tInputCmd.bBoostPressed)
    {
        /* 점프 */
    }

    if (XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero()))
    {
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE));
    }

    if (m_tInputCmd.bLeftAnchorPressed)
    {
        /* 앵커 고정 가능한지 판단 */
        m_tRef.pGear->Try_Grappling(SIDE::LEFT);
    }
    if (m_tInputCmd.bRightAnchorPressed)
    {
        /* 앵커 고정 가능한지 판단 */
        m_tRef.pGear->Try_Grappling(SIDE::RIGHT);
    }
}

void CPlayerState_GroundedMove::Enter(_uint iDetailFlag)
{
    LOG_INFO("[ ENTER PLAYERSTATE_GROUNDEMOVE ]");
    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);
}

void CPlayerState_GroundedMove::Move(_float fDT)
{
    const _float fMaxSpeed = m_pInfo->fMaxSpeed;
    const _float fCurSpeed = m_pInfo->fCurSpeed;

    /* 플레이어의 현재 속도 */
    _float3 vLinearVel = m_tComponents.rigidbody.Get_LinearVel();

    _float3 vMoveDir{};
    vMoveDir.x = m_tInputCmd.vMove.x;
    vMoveDir.z = m_tInputCmd.vMove.z;

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
    if (fHorizontalSpeedSq < fMaxSpeed * fMaxSpeed)
    {
        _float3 vForce{};
        vForce.x = vMoveDir.x * fCurSpeed * fCurSpeed;
        vForce.z = vMoveDir.z * fCurSpeed * fCurSpeed;

        m_tComponents.rigidbody.Add_Force(vForce);
    }
}

std::shared_ptr<CPlayerState_GroundedMove> CPlayerState_GroundedMove::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_GroundedMove>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
