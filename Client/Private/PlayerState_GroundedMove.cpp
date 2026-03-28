#include "PlayerState_GroundedMove.h"

#include "Logger.h"
#include "GameObject.h"
#include "PlayerStateMachine.h"
#include "AnimationClip_Player.h"
#include "ODM_Gear.h"

CPlayerState_GroundedMove::CPlayerState_GroundedMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
    : CPlayerState(goPlayer, scPlayer)
{
}

CPlayerState_GroundedMove::~CPlayerState_GroundedMove()
{
}

HRESULT CPlayerState_GroundedMove::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");

    m_rbPlayer = m_goPlayer->Get_Component<CRigidbody>();

    return S_OK;
}

void CPlayerState_GroundedMove::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);
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
    const _float fMaxSpeed = m_tRef.pInfo->fMaxSpeed;
    const _float fCurSpeed = m_tRef.pInfo->fCurSpeed;

    /* 플레이어의 현재 속도 */
    _float3 vLinearVel = m_rbPlayer.Get_LinearVel();

    _float3 vMoveDir{};
    vMoveDir.x = m_tInputCmd.vMove.x;
    vMoveDir.z = m_tInputCmd.vMove.z;

    LOG_INFO(" ====================================================================== ");
    LOG_INFO(" ===== [ vMove ] X : %.2f, Z : %.2f ===== ", vMoveDir.x, vMoveDir.z);

    const _float fMoveLenSq = vMoveDir.x * vMoveDir.x + vMoveDir.z * vMoveDir.z;

    LOG_INFO(" ===== [ fMoveLenSq ] X : %.2f ===== ", fMoveLenSq);

    /* 입력 없음 */
    if (fMoveLenSq <= 0.f)
        return;


    _float3 vHorizontalVel{};
    vHorizontalVel.x = vLinearVel.x;
    vHorizontalVel.z = vLinearVel.z;

    LOG_INFO(" ===== [ vLinearVel ] : %.2f, %.2f, %.2f ===== ", vLinearVel.x, vLinearVel.y, vLinearVel.z);

    const _float fHorizontalSpeedSq =
        vHorizontalVel.x * vHorizontalVel.x +
        vHorizontalVel.z * vHorizontalVel.z;

    LOG_INFO(" ===== [ fHorizontalSpeedSq ] : %.2f ===== ", fHorizontalSpeedSq);
    LOG_INFO(" ===== [ fMaxSpeed^2 ] : %.2f ===== ", fMaxSpeed * fMaxSpeed);

    /* 최대 속도 제한 */
    if (fHorizontalSpeedSq < fMaxSpeed * fMaxSpeed)
    {
        _float3 vForce{};
        vForce.x = vMoveDir.x * fCurSpeed * fCurSpeed;
        vForce.z = vMoveDir.z * fCurSpeed * fCurSpeed;

        m_rbPlayer.Add_Force(vForce);

        LOG_INFO(" ===== [ vForce ] : %.2f, %.2f, %.2f ===== ", vForce.x, vForce.y, vForce.z);
    }
}

std::shared_ptr<CPlayerState_GroundedMove> CPlayerState_GroundedMove::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer)
{
    auto pInstance = std::make_shared<CPlayerState_GroundedMove>(goPlayer, scPlayer);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
