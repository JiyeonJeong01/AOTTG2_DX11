#include "PlayerState_Dodge.h"

#include "Logger.h"
#include "GameObject.h"
#include "PlayerStateMachine.h"
#include "AnimationClip_Player.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"

CPlayerState_Dodge::CPlayerState_Dodge(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_Dodge::~CPlayerState_Dodge()
{
}

HRESULT CPlayerState_Dodge::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_scPlayer, E_FAIL, "m_scPlayer is nullptr");

    return S_OK;
}

void CPlayerState_Dodge::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_Dodge::On_AnimFinished, this);
}

void CPlayerState_Dodge::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);

    Control_Camera();
    LookTo_InputDir(fDT);

    Try_Grappling();
    Finish_Grappling();
}

void CPlayerState_Dodge::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    if (!m_bFinished)
        Move_Dodge();
}

void CPlayerState_Dodge::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    Decide_NextState();
}

void CPlayerState_Dodge::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    m_bFinished = false;

    m_vDodgeDir = GAME_INSTANCE.Cam_Look();
    m_vDodgeDir.y = 0.f;
    m_vDodgeDir.x *= -1.f;
    m_vDodgeDir.z *= -1.f;

    const _float fLenSq = m_vDodgeDir.x * m_vDodgeDir.x + m_vDodgeDir.z * m_vDodgeDir.z;
    if (fLenSq > 0.f)
    {
        const _float fLen = sqrtf(fLenSq);
        m_vDodgeDir.x /= fLen;
        m_vDodgeDir.z /= fLen;
    }
    else
    {
        m_vDodgeDir = { 0.f, 0.f, -1.f };
    }

    _float3 vImpulse{};
    vImpulse.x = m_vDodgeDir.x * m_fDodgeImpulse;
    vImpulse.z = m_vDodgeDir.z * m_fDodgeImpulse;
    m_tComponents.rigidbody.Add_LinearImpulse(vImpulse);

    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::DODGE);
}

void CPlayerState_Dodge::Exit()
{
    CPlayerState::Exit();
}

void CPlayerState_Dodge::Move_Dodge()
{
    const _float fMaxSpeed = m_pStats->fMaxSpeed * m_fDodgeMaxSpeedMul;

    _float3 vLinearVel = m_tComponents.rigidbody.Get_LinearVel();

    _float3 vHorizontalVel{};
    vHorizontalVel.x = vLinearVel.x;
    vHorizontalVel.z = vLinearVel.z;

    const _float fHorizontalSpeedSq =
        vHorizontalVel.x * vHorizontalVel.x +
        vHorizontalVel.z * vHorizontalVel.z;

    if (fHorizontalSpeedSq < fMaxSpeed * fMaxSpeed)
    {
        _float3 vForce{};
        vForce.x = m_vDodgeDir.x * m_fDodgeAssistForce;
        vForce.z = m_vDodgeDir.z * m_fDodgeAssistForce;

        m_tComponents.rigidbody.Add_Force(vForce);
    }
}

void CPlayerState_Dodge::Decide_NextState()
{
    if (!m_bFinished)
        return;

    if (m_tRef.pGroundChecker->Get_OnWalkable())
    {
        cout << "[DODGE] -> GROUNDED_MOVE\n";
        const float THREASHOLD = 4.f;

        _float3 fLinearVel = m_tComponents.rigidbody.Get_LinearVel();

        const _float fLinearVelSq = fLinearVel.x * fLinearVel.x + fLinearVel.z * fLinearVel.z;

        if (fLinearVelSq > THREASHOLD)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::SLIDE));
        else
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE), To<_uint>(GROUNDED_MOVE::DASH_LAND));

        return;
    }

    m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));
}

void CPlayerState_Dodge::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[DODGE] On_AnimFinished\n";

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::DODGE])
        On_DodgeFinished(tData);
}

void CPlayerState_Dodge::On_DodgeFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_bFinished = true;
    cout << " => [DODGE] On_DodgeFinished\n";
}

std::shared_ptr<CPlayerState_Dodge> CPlayerState_Dodge::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Dodge>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
