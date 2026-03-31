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
    Try_Grappling();
}

void CPlayerState_GroundedMove::Update(_float fDT)
{
    if (m_eGroundedMoveState == GROUNDED_MOVE::DASH_LAND)
        return;
    Move(fDT);
}

void CPlayerState_GroundedMove::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

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
    }
    else if (iDetailFlag == To<_uint>(GROUNDED_MOVE::DASH_LAND))
    {
        cout << "[GROUNDED_MOVE] ENTER DASH_LAND\n";
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::DASH_LAND);
    }
    else if (iDetailFlag == To<_uint>(GROUNDED_MOVE::SLIDE))
    {
        /* 유체 저항 일시적 감소 */
        cout << "[GROUNDED_MOVE] ENTER SLIDE\n";

        m_fOriginDrag = m_tComponents.rigidbody.Get_Drag();
        m_tComponents.rigidbody.Set_Drag(m_fSlidingDrag);
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::SLIDE);
    }
}

void CPlayerState_GroundedMove::Exit()
{
    CPlayerState::Exit();

    /* 유체 저항 복구 */
    m_tComponents.rigidbody.Set_Drag(m_fOriginDrag);
}

void CPlayerState_GroundedMove::Setup_CachedPlayerInfos()
{
    CPlayerState::Setup_CachedPlayerInfos();

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
        {        //_uint iCurIdx = m_tComponents.animator->iAnimationClip;
                //_uint iNextIdx = m_tComponents.animator->iAnimationClip;

                //_bool bShouldStay = false;
                //bShouldStay = iCurIdx == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::DASH_LAND)
                //              || iNextIdx == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::DASH_LAND);
                //if (bShouldStay)
                //    return;

                //bShouldStay = iCurIdx == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::SLIDE)
                //              || iNextIdx == m_tComponents.animator.Get_AnimationClipIdx_By_Name(ANIM_PLAYER::SLIDE);
                //if (bShouldStay)
                //    return;
        }

        if (m_eGroundedMoveState == GROUNDED_MOVE::DASH_LAND)
            return;
        if (m_eGroundedMoveState == GROUNDED_MOVE::SLIDE)
        {
            const _float3 vLinearVel = m_tComponents.rigidbody.Get_LinearVel();
            _float fLinearVelSq = vLinearVel.x * vLinearVel.x + vLinearVel.z * vLinearVel.z;
            if (fLinearVelSq > 5.f) return; /* 슬라이딩 속도가 일정 이하인 경우 IDLE로 전환 */
        }

        cout << "[GROUNDED_MOVE] -> IDLE\n";

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE));
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

void CPlayerState_GroundedMove::Move(_float fDT)
{
    const _float fMaxSpeed = m_pStats->fMaxSpeed;
    const _float fCurSpeed = m_pStats->fCurSpeed;

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
