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
    CPlayerState::Priority_Update(fDT);

    Control_Camera();
    LookTo_InputDir(fDT);
    Try_Grappling();
    Finish_Grappling();

    if (m_eGroundedMoveState == GROUNDED_MOVE::SLIDE)
        Handle_Trail(fDT, WIDTH_TYPE::THIN);
    else
        Handle_Trail(fDT, WIDTH_TYPE::NONE);
}

void CPlayerState_GroundedMove::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    if (m_eGroundedMoveState == GROUNDED_MOVE::DASH_LAND)
        return;
    if (m_eGroundedMoveState == GROUNDED_MOVE::SLIDE)
        return;

    CPlayerState::GroundedMove(fDT);
}

void CPlayerState_GroundedMove::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    Decide_NextAnim();
    Decide_NextState();
}

void CPlayerState_GroundedMove::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    m_eGroundedMoveState = To<GROUNDED_MOVE>(iDetailFlag);

    if (iDetailFlag == To<_uint>(GROUNDED_MOVE::RUN))
    {
        cout << "[GROUNDED_MOVE] ENTER RUN\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);
        CPlayerState::GroundedMove(m_fRunCorrectionDT); /* 바로 run으로 들어오는 경우 움직임 끊겨보인다. */
    }
    else if (iDetailFlag == To<_uint>(GROUNDED_MOVE::DASH_LAND)) /* 착지 */
    {
        cout << "[GROUNDED_MOVE] ENTER DASH_LAND\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::DASH_LAND);
    }
    else if (iDetailFlag == To<_uint>(GROUNDED_MOVE::SLIDE))
    {
        /* 유체 저항 일시적 감소 */
        cout << "[GROUNDED_MOVE] ENTER SLIDE\n";

        m_tComponents.rigidbody.Set_Drag(m_fSlidingDrag);
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SLIDE);

        /* 슬라이딩 방향으로 힘 줘서 슬라이딩 효과 유지하기 */
        _float3 vVel = m_tComponents.rigidbody.Get_LinearVel();
        const _float fVelScale = 0.45f;
        vVel.x *= fVelScale;
        vVel.y = 0.f; 
        vVel.z *= fVelScale;
        m_tComponents.rigidbody.Add_LinearImpulse(vVel);
    }
}

void CPlayerState_GroundedMove::Exit()
{
    CPlayerState::Exit();

    /* 유체 저항 복구 */
    m_tComponents.rigidbody.Set_Drag(m_fOriginDrag);
}

void CPlayerState_GroundedMove::Cache_PlayerContext(const PLAYER_CONTEXT& tContext)
{
    CPlayerState::Cache_PlayerContext(tContext);

    m_fOriginDrag = tContext.fOriginDrag;
}

void CPlayerState_GroundedMove::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_GroundedMove::On_AnimFinished, this);
}

void CPlayerState_GroundedMove::Decide_NextState()
{
    /* -> JUMP */
    if (m_tInputCmd.bBoostPressed)
    {
        cout << "[GROUNDED_MOVE] -> JUMP::JUMP_BEGIN\n";

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::JUMP), To<_uint>(JUMP::JUMP_BEGIN));
        return;
    }

    /* -> GROUNDED_ATTAK */
    if (m_tInputCmd.bNormalAttackPressed)
    {
        cout << "[GROUNDED_MOVE] -> GROUNDED_ATTACK::ATK\n";

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_ATTACK), To<_uint>(GROUNDED_ATTACK::ATK));
        return;
    }

    /* -> IDLE */
    if (XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero()))
    {
        /* 아직 상태 전환하지 않고 애니메이션을 마저 재생해야 하는 경우 */
        /* - 공중에서 바닥으로 착지 후 착지/슬라이딩 애니메이션을 재생하는 경우 */
        if (m_eGroundedMoveState == GROUNDED_MOVE::DASH_LAND)
            return;
        if (m_eGroundedMoveState == GROUNDED_MOVE::SLIDE)
        {
            const _float3 vLinearVel = m_tComponents.rigidbody.Get_LinearVel();
            _float fLinearVelSq = vLinearVel.x * vLinearVel.x + vLinearVel.z * vLinearVel.z;
            if (fLinearVelSq > m_fSlideThreshold) return; /* 슬라이딩 속도가 일정 이하인 경우 IDLE로 전환 */
        }

        cout << "[GROUNDED_MOVE] -> IDLE\n";

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE));
        return;
    }

    /* -> RELOAD */
    _bool bReload = m_tInputCmd.bReloadBlade;
    if (bReload)
    {
        cout << "[GROUNDED_MOVE] -> RELOAD::GROUNDED\n";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::RELOAD), To<_uint>(RELOAD::GROUNDED));
        return;
    }

    /* -> RESUPPLY */
    _bool bInteract = m_tInputCmd.bInteract;
    if (bInteract)
    {
        cout << "[GROUNDED_MOVE] -> RESUPPLY\n";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::RESUPPLY), 0);
        return;
    }

    /* -> DODGE */
    _bool bDodge = m_tInputCmd.bDodge;
    if (bDodge)
    {
        cout << "[GROUNDED_MOVE] -> DODGE\n";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::DODGE), 0);
        return;
    }
}

void CPlayerState_GroundedMove::Decide_NextAnim()
{
    /* SLIDE -> RUN */
    _bool bInputMove = !XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero());
    if (bInputMove)
    {
        if (m_eGroundedMoveState == GROUNDED_MOVE::SLIDE)
        {
            const _float3 vLinearVel = m_tComponents.rigidbody.Get_LinearVel();
            _float fLinearVelSq = vLinearVel.x * vLinearVel.x + vLinearVel.z * vLinearVel.z;
            if (fLinearVelSq < m_fSlideThreshold)
            {
                m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);
                m_eGroundedMoveState = GROUNDED_MOVE::RUN;
                m_tComponents.rigidbody.Set_Drag(m_fOriginDrag);
                GroundedMove(m_fRunCorrectionDT);

                cout << "[GROUNDED_MOVE] SLIDE -> RUN\n";
            }
        }
    }
}

void CPlayerState_GroundedMove::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[GROUNDED_MOVE] On_AnimFinished\n";

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::DASH_LAND])
        On_DashLandFinished(tData);
}

void CPlayerState_GroundedMove::On_DashLandFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    cout << " => [GROUNDED_MOVE] On_DashLandFinished : ";

    /* 입력이 없는 경우 -> IDLE */
    if (XMVector3Equal(XMVectorZero(), XMLoadFloat3(&m_tInputCmd.vMove)))
    {
        cout << " -> IDLE\n ";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE), 0);
    }
    /* 입력이 있는 경우 RUN */
    else
    {
        cout << " -> GROUNDED_MOVE::RUN\n";
        m_eGroundedMoveState = GROUNDED_MOVE::RUN;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RUN);   
    }
}

std::shared_ptr<CPlayerState_GroundedMove> CPlayerState_GroundedMove::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_GroundedMove>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
