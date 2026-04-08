#include "PlayerState_Reload.h"

#include "Logger.h"
#include "GameObject.h"
#include "PlayerStateMachine.h"
#include "AnimationClip_Player.h"
#include "ODM_Gear.h"
#include "GroundChecker.h"

CPlayerState_Reload::CPlayerState_Reload(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_Reload::~CPlayerState_Reload()
{
}

HRESULT CPlayerState_Reload::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");

    return S_OK;
}

void CPlayerState_Reload::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_Reload::On_AnimFinished, this);
}

void CPlayerState_Reload::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);

    Control_Camera();
    LookTo_InputDir(fDT);
    Finish_Grappling();
}

void CPlayerState_Reload::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    /* TODO : 이거 하체 상체 섞어야 함 !!!!!! */
    /*if (m_eReloadState == RELOAD::GROUNDED)
        CPlayerState::GroundedMove(fDT);*/
}

void CPlayerState_Reload::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    Decide_NextState();
}

void CPlayerState_Reload::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    if (iDetailFlag >= To<_uint>(RELOAD::END))
    {
        m_eReloadState = RELOAD::GROUNDED;
    }
    else
    {
        m_eReloadState = To<RELOAD>(iDetailFlag);
    }

    if (m_eReloadState == RELOAD::GROUNDED)
    {
        if (m_pBlade && m_pBlade->Can_ReloadBlade())
            m_pBlade->Reload_Blade();
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::CHANGE_BLADE);
        cout << "[RELOAD] ENTER GROUNDED\n";
        return;
    }

    if (m_eReloadState == RELOAD::AIR)
    {
        if (m_pBlade && m_pBlade->Can_ReloadBlade())
            m_pBlade->Reload_Blade();
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::CHANGE_BLADE_AIR);
        cout << "[RELOAD] ENTER AIR\n";
        return;
    }

    cout << "[RELOAD] 지정되지 않은 상태\n";
}

void CPlayerState_Reload::Exit()
{
    CPlayerState::Exit();
}

void CPlayerState_Reload::Decide_NextState()
{
    /* 리로드 애니메이션이 끝날 때까지 유지 */
    if (m_eReloadState != RELOAD::END)
        return;

    if (m_tRef.pGroundChecker && m_tRef.pGroundChecker->Get_OnWalkable())
    {
        _bool bHasInput = !XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero());

        if (bHasInput)
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE));
        else
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE));

        return;
    }

    m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::AIRBORNE_MOVE), To<_uint>(AIRBORNE_MOVE::AIR_FALL));
}

void CPlayerState_Reload::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[RELOAD] On_AnimFinished\n";

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::CHANGE_BLADE]
        || iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::CHANGE_BLADE_AIR])
    {
        On_ReloadFinished(tData);
    }
}

void CPlayerState_Reload::On_ReloadFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_eReloadState = RELOAD::END;
    cout << " => [RELOAD] On_ReloadFinished\n";
}

std::shared_ptr<CPlayerState_Reload> CPlayerState_Reload::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Reload>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
