#include "PlayerState_AirborneMove.h"
#include "AnimationClip_Player.h"
#include "PlayerStateMachine.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"
#include "Player_SkillController.h"

CPlayerState_AirborneMove::CPlayerState_AirborneMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_AirborneMove::~CPlayerState_AirborneMove()
{
}

HRESULT CPlayerState_AirborneMove::Initialize()
{
    return CPlayerState::Initialize();
}

void CPlayerState_AirborneMove::Setup_CachedPlayerInfos()
{
    CPlayerState::Setup_CachedPlayerInfos();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_AirborneMove::On_AnimFinished, this);
}

void CPlayerState_AirborneMove::Priority_Update(_float fDT)
{
    Control_Camera();
    LookTo_InputDir(fDT);
    Try_Grappling();
}

void CPlayerState_AirborneMove::Update(_float fDT)
{
    _bool bLeftHook = m_tInputCmd.bLeftAnchorHeld;
    _bool bRightHook = m_tInputCmd.bRightAnchorHeld;

    /* 그래플링 끝 */
    if (m_eAirborneState != AIRBORNE_MOVE::AIR_FALL
        && (bLeftHook == false && bRightHook == false))
    {
        m_eAirborneState = AIRBORNE_MOVE::AIR_FALL;
        m_tRef.pGear->Finish_Grappling();
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);

        return;
    }

    /* 그래플링 좌/우/정면 적용 */
    if (m_eAirborneState == AIRBORNE_MOVE::AIR_BEGIN)
        return;

    Decide_HookAnim();
}

void CPlayerState_AirborneMove::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
    Decide_NextState();
}

void CPlayerState_AirborneMove::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    if (iDetailFlag < To<_uint>(AIRBORNE_MOVE::END))
        m_eAirborneState = To<AIRBORNE_MOVE>(iDetailFlag);

    _uint iFlag = m_tRef.pGear->Get_UsingFlag();

    if ((iFlag & To<_uint>(SIDE::BOTH)) != 0 /* 기어 사용 중 */
        && m_eAirborneState == AIRBORNE_MOVE::AIR_BEGIN)
    {
        /* Grapple Action (Left, Right, or Both) */
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::DASH);

        cout << "[AIRBORNE_MOVE] ENTER BEGIN\n";

        return;
    }
    if (m_eAirborneState == AIRBORNE_MOVE::AIR_FALL)
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);

        cout << "[AIRBORNE_MOVE] ENTER FALL\n";
        return;
    }
    if (m_eAirborneState == AIRBORNE_MOVE::AIR)
    {
        cout << "[AIRBORNE_MOVE] ENTER AIR\n";
        Decide_HookAnim();
        return;
    }

    cout << "[AIRBORNE_MOVE] 지정되지 않은 상태\n";

}

void CPlayerState_AirborneMove::Exit()
{
    CPlayerState::Exit();
}

void CPlayerState_AirborneMove::Decide_NextState()
{
    /* -> GROUNDED_MOVE(착지/슬라이딩) */
    if (m_eAirborneState == AIRBORNE_MOVE::AIR_FALL && m_tRef.pGroundChecker->Get_OnWalkable())
    {
        cout << "[AIRBORNE_MOVE] -> GROUNDED_MOVE\n";
        const float THREASHOLD = 10.f;

        _float3 fLinearVel = m_tComponents.rigidbody.Get_LinearVel();

        const _float fLinearVelSq = fLinearVel.x * fLinearVel.x + fLinearVel.z * fLinearVel.z;

        if (fLinearVelSq > THREASHOLD)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::SLIDE));
        else
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::DASH_LAND));

        return;
    }

    LOG_INFO(" ======== [ AIRBORNE_MOVE ] ======== ");
    LOG_INFO("%d", m_tInputCmd.bStrongAttackPressed);

    _bool bNormalAtk = m_tInputCmd.bNormalAttackPressed;

    if (bNormalAtk)
    {
        cout << "[AIRBORNE_MOVE] -> AIRBORNE_ATTACK::NOMAL\n";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_ATTACK), To<_uint>(AIRBORNE_ATTACK::NORMAL));
        return;
    }

    _bool bStrongAtk = m_tInputCmd.bStrongAttackPressed;
    if (bStrongAtk)
    {
        cout << "[AIRBORNE_MOVE] -> AIRBORNE_ATTACK::STRONG\n";
        SKILL eTrySkill = m_pSkillController->Get_CurSkill();
        _bool bCanAtk = m_pSkillController->Try_UseSKill(eTrySkill);
        if (bCanAtk)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_ATTACK), To<_uint>(AIRBORNE_ATTACK::STRONG));
        return;
    }

}

void CPlayerState_AirborneMove::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[AIRBORNE_ATTACK] On_AnimFinished\n";

    _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::DASH])
        On_AirDashFinished(tData);
}

void CPlayerState_AirborneMove::On_AirDashFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    Decide_HookAnim();

    cout << " => [AIRBORNE_MOVE] On_NomalFinished\n";
}

void CPlayerState_AirborneMove::Decide_HookAnim()
{
    _bool bLeftHook = m_tInputCmd.bLeftAnchorHeld;
    _bool bRightHook = m_tInputCmd.bRightAnchorHeld;

    /* -> 정면 */
    if (m_eAirborneState != AIRBORNE_MOVE::AIR_FRONT && bLeftHook == true && bRightHook == true)
    {
        m_eAirborneState = AIRBORNE_MOVE::AIR_FRONT;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_HOOK);
    }
    /* -> 좌측 앵커 사용 */
    else if (m_eAirborneState != AIRBORNE_MOVE::AIR_LEFT && bLeftHook == true && bRightHook == false)
    {
        m_eAirborneState = AIRBORNE_MOVE::AIR_LEFT;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_LEFT);
    }
    /* -> 우측 앵커 사용 */
    else if (m_eAirborneState != AIRBORNE_MOVE::AIR_RIGHT && bLeftHook == false && bRightHook == true)
    {
        m_eAirborneState = AIRBORNE_MOVE::AIR_RIGHT;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RIGHT);
    }
    /* 사용 중이 아님 */
    else
    {
        m_eAirborneState = AIRBORNE_MOVE::AIR_FALL;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);
    }
}

std::shared_ptr<CPlayerState_AirborneMove> CPlayerState_AirborneMove::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_AirborneMove>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
