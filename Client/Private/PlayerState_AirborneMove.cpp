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

void CPlayerState_AirborneMove::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_AirborneMove::On_AnimFinished, this);
}

void CPlayerState_AirborneMove::Priority_Update(_float fDT)
{
    Control_Camera();

    if (m_tRef.pGear->Has_Anchor())
        LookTo_AnchorPos(fDT);
    else 
        LookTo_InputDir(fDT);

    Try_Grappling();
    Finish_Grappling();

    if (m_tInputCmd.bRopeReelHeld)
        Handle_Trail(fDT, WIDTH_TYPE::BOLD);
    else
        Handle_Trail(fDT, WIDTH_TYPE::NORMAL);

    Handle_SpeedLines(fDT);
}

void CPlayerState_AirborneMove::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    _bool bLeftHook = m_tInputCmd.bLeftAnchorHeld;
    _bool bRightHook = m_tInputCmd.bRightAnchorHeld;

    m_tRef.pGear->Set_ReelBoost(m_tInputCmd.bRopeReelHeld);

    /* 그래플링 끝 */
    if (m_eAirborneState != AIRBORNE_MOVE::AIR_FALL
        && bLeftHook == false && bRightHook == false)
    {
        m_eAirborneState = AIRBORNE_MOVE::AIR_FALL;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_FALL);

        m_fGroundStableTime = 0.f;
        m_fAirStableTime = 0.f;

        return;
    }

    /* AIR_BEGIN 중에는 DASH 끝날 때까지 유지 */
    if (m_eAirborneState == AIRBORNE_MOVE::AIR_BEGIN)
        return;

    Update_AnchorAirOrSlide();
}

void CPlayerState_AirborneMove::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
    Decide_NextState();
}

void CPlayerState_AirborneMove::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    m_fGroundStableTime = 0.f;
    m_fAirStableTime = 0.f;
    m_fAirReleaseElapsedTime = m_fTotalAirReleaseTime; /* 바로 시작할 수 있도록 */

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
    if (m_eAirborneState == AIRBORNE_MOVE::AIR
        || m_eAirborneState == AIRBORNE_MOVE::AIR_LEFT
        || m_eAirborneState == AIRBORNE_MOVE::AIR_RIGHT
        || m_eAirborneState == AIRBORNE_MOVE::AIR_FRONT
        || m_eAirborneState == AIRBORNE_MOVE::SLIDE_LEFT
        || m_eAirborneState == AIRBORNE_MOVE::SLIDE_RIGHT
        || m_eAirborneState == AIRBORNE_MOVE::SLIDE_FRONT)
    {
        cout << "[AIRBORNE_MOVE] ENTER AIR/SLIDE\n";
        Update_AnchorAirOrSlide();
        return;
    }

    cout << "[AIRBORNE_MOVE] 지정되지 않은 상태\n";

}

void CPlayerState_AirborneMove::Exit()
{
    CPlayerState::Exit();

    m_fGroundStableTime = 0.f;
    m_fAirStableTime = 0.f;
}

void CPlayerState_AirborneMove::Decide_NextState()
{
    _bool bLeftHook = m_tInputCmd.bLeftAnchorHeld;
    _bool bRightHook = m_tInputCmd.bRightAnchorHeld;
    const _bool bUsingAnchor = (bLeftHook || bRightHook);

    /* 훅 없이 추락 착지했을 때만 지상 상태로 전환 */
    if (bUsingAnchor == false
        && m_eAirborneState == AIRBORNE_MOVE::AIR_FALL
        && m_tRef.pGroundChecker->Get_OnWalkable())
    {
        cout << "[AIRBORNE_MOVE] -> GROUNDED_MOVE\n";
        const float THREASHOLD = 4.f;

        _float3 fLinearVel = m_tComponents.rigidbody.Get_LinearVel();
        const _float fLinearVelSq = fLinearVel.x * fLinearVel.x + fLinearVel.z * fLinearVel.z;

        if (fLinearVelSq > THREASHOLD)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::SLIDE));
        else
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::DASH_LAND));

        return;
    }

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
        SKILL_TYPE eTrySkill = m_pSkillController->Get_CurSkillType();
        _bool bCanAtk = m_pSkillController->Try_UseSKill(eTrySkill);
        if (bCanAtk)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_ATTACK), To<_uint>(AIRBORNE_ATTACK::STRONG));
        return;
    }

    _bool bReload = m_tInputCmd.bReloadBlade;
    if (bReload)
    {
        cout << "[AIRBORNE_MOVE] -> RELOAD::AIR\n";
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::RELOAD), To<_uint>(RELOAD::AIR));
        return;
    }
}

void CPlayerState_AirborneMove::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[AIRBORNE_MOVE] On_AnimFinished | ClipIdx : " << tData.iAnimationClip << "\n";

    _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    cout << "[AIRBORNE_MOVE] DASH IDX : " << m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::DASH] << "\n";

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::DASH])
        On_AirDashFinished(tData);
    else if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::AIR_RELEASE])
        m_bAirReleasePlayed = false;
}

void CPlayerState_AirborneMove::On_AirDashFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    UNREFERENCED_PARAMETER(tData);

    Update_AnchorAirOrSlide();

    cout << " => [AIRBORNE_MOVE] On_NomalFinished\n";
}

void CPlayerState_AirborneMove::Decide_HookAnim()
{
    Update_AnchorAirOrSlide();
}

void CPlayerState_AirborneMove::Update_AnchorAirOrSlide()
{
    const _bool bOnGroundRaw = m_tRef.pGroundChecker->Get_OnWalkable();

    if (bOnGroundRaw)
    {
        m_fGroundStableTime += GAME_INSTANCE.Get_DT();
        m_fAirStableTime = 0.f;
    }
    else
    {
        m_fAirStableTime += GAME_INSTANCE.Get_DT();
        m_fGroundStableTime = 0.f;
    }

    _bool bOnGround = false;

    if (Is_AnchorSliding())
        bOnGround = (m_fAirStableTime < 0.05f);
    else
        bOnGround = (m_fGroundStableTime >= 0.03f);

    Decide_AnchorMoveAnim(bOnGround);
}

void CPlayerState_AirborneMove::Decide_AnchorMoveAnim(_bool bOnGround)
{
    if (Try_AirReleaseMotion())
        return;

    _bool bLeftHook = m_tInputCmd.bLeftAnchorHeld;
    _bool bRightHook = m_tInputCmd.bRightAnchorHeld;

    AIRBORNE_MOVE eNextState = AIRBORNE_MOVE::AIR_FALL;
    const char* pNextAnim = ANIM_PLAYER::AIR_FALL;

    if (bLeftHook == true && bRightHook == true)
    {
        if (bOnGround)
        {
            eNextState = AIRBORNE_MOVE::SLIDE_FRONT;
            pNextAnim = ANIM_PLAYER::SLIDE;
        }
        else
        {
            eNextState = AIRBORNE_MOVE::AIR_FRONT;
            pNextAnim = ANIM_PLAYER::AIR_HOOK;
        }
    }
    else if (bLeftHook == true && bRightHook == false)
    {
        if (bOnGround)
        {
            eNextState = AIRBORNE_MOVE::SLIDE_LEFT;
            pNextAnim = ANIM_PLAYER::SLIDE;
        }
        else
        {
            eNextState = AIRBORNE_MOVE::AIR_LEFT;
            pNextAnim = ANIM_PLAYER::AIR_LEFT;
        }
    }
    else if (bLeftHook == false && bRightHook == true)
    {
        if (bOnGround)
        {
            eNextState = AIRBORNE_MOVE::SLIDE_RIGHT;
            pNextAnim = ANIM_PLAYER::SLIDE;
        }
        else
        {
            eNextState = AIRBORNE_MOVE::AIR_RIGHT;
            pNextAnim = ANIM_PLAYER::AIR_RIGHT;
        }
    }
    else
    {
        eNextState = AIRBORNE_MOVE::AIR_FALL;
        pNextAnim = ANIM_PLAYER::AIR_FALL;
    }

    if (m_eAirborneState != eNextState)
    {
        m_eAirborneState = eNextState;
        m_tComponents.animator.Set_NextAnimationClip(pNextAnim);

        cout << "[AIRBORNE_MOVE] AnchorMove -> " << To<_uint>(eNextState) << "\n";
    }
}

_bool CPlayerState_AirborneMove::Is_AnchorSliding() const
{
    return m_eAirborneState == AIRBORNE_MOVE::SLIDE_LEFT
        || m_eAirborneState == AIRBORNE_MOVE::SLIDE_RIGHT
        || m_eAirborneState == AIRBORNE_MOVE::SLIDE_FRONT;
}

_bool CPlayerState_AirborneMove::Try_AirReleaseMotion()
{
    m_fAirReleaseElapsedTime += GAME_INSTANCE.Get_DT();

    if (m_fAirReleaseElapsedTime > m_fTotalAirReleaseTime)
    {
        m_fAirReleaseElapsedTime = m_fTotalAirReleaseTime;
        m_bAirReleasePlayed = false;
    }
    else    /* 쿨타임  */
        return false;

    if (!m_bAirReleasePlayed
        && m_tComponents.transform->vPosition.y >= 8.f
        && m_tComponents.rigidbody->vLinearVel.y >= 10.f)
    {
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::AIR_RELEASE);
        m_fAirReleaseElapsedTime = 0.f;
        m_bAirReleasePlayed = true;
    }

    return m_bAirReleasePlayed;
}

std::shared_ptr<CPlayerState_AirborneMove> CPlayerState_AirborneMove::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_AirborneMove>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
