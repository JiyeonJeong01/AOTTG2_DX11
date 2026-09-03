#include "PlayerState_Grabbed.h"

#include "AnimationClip_Player.h"
#include "Player.h"
#include "Entity_Define.h"
#include "Scout_Controller.h"

#include "PlayerStateMachine.h"
#include "CinematicSystem.h"
#include "Easing_Function.h"

CPlayerState_Grabbed::CPlayerState_Grabbed(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_Grabbed::~CPlayerState_Grabbed()
{
}

HRESULT CPlayerState_Grabbed::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_scPlayer, E_FAIL, "m_scPlayer is nullptr");

    return S_OK;
}

void CPlayerState_Grabbed::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    
}

void CPlayerState_Grabbed::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);

    Handle_Trail(fDT, WIDTH_TYPE::NONE);
}

void CPlayerState_Grabbed::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    m_fElapsedGrabTime += fDT;
    if (m_fElapsedGrabTime > 3.f)
    {
        if (g_bRequestResupplyPerformed)
        {
            CTitan* pTitan = m_scPlayer->Get_GrabbTitan();
            if (pTitan)
                pTitan->Force_Idle();

            const _float3 vPos = { 12.2f, 15.68f, 1.41f };
            m_tComponents.transform.Set_Position(XMLoadFloat3(&vPos));
            m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE));

            CGameObject* goScouts = GAME_INSTANCE.Find_GameObject("Scouts");
            if (goScouts)
            {
                CScout_Controller* scScouts = goScouts->Get_Script<CScout_Controller>();
                if (scScouts)
                    scScouts->Start_RescueDialogue();
            }
        }
        else
        {
            if (!m_bFadeOut)
                Start_DeathFade();

            Update_DeathFade(fDT);
        }
    }
}

void CPlayerState_Grabbed::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);
}

void CPlayerState_Grabbed::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    m_fElapsedGrabTime = 0.f;
    m_fFadeTime = 0.f;
    m_bFadeOut = false;

    m_goFadeUI = m_scPlayer->Get_FadeUI();
    m_crFadeUI = {};
    if (m_goFadeUI)
    {
        m_crFadeUI = m_goFadeUI->Get_Component<CCanvasRenderer>();
        if (m_crFadeUI.Is_Valid())
            m_crFadeUI.Set_Color({ 0.f, 0.f, 0.f, 0.f });

        m_goFadeUI->Set_Enable(false);
    }

    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::GRABBED);

    m_tComponents.collider.Set_Enable(false);
    m_tComponents.rigidbody.Set_Enable(false);
    m_tComponents.springJoint.Set_Enable(false);
}

void CPlayerState_Grabbed::Exit()
{
    CPlayerState::Exit();

    m_tComponents.collider.Set_Enable(true);
    m_tComponents.rigidbody.Set_Enable(true);
    m_tComponents.springJoint.Set_Enable(true);
}

void CPlayerState_Grabbed::Start_DeathFade()
{
    m_bFadeOut = true;
    m_fFadeTime = 0.f;

    if (m_goFadeUI)
        m_goFadeUI->Set_Enable(true);
}

void CPlayerState_Grabbed::Update_DeathFade(_float fDT)
{
    m_fFadeTime += fDT;

    const _float fRatio = CEasingFunction::EaseOutCubic(
        CEasingFunction::Clamp01(m_fFadeTime / m_fFadeDuration));

    if (m_crFadeUI.Is_Valid())
        m_crFadeUI.Set_Color({ 0.f, 0.f, 0.f, fRatio });

    if (m_fFadeTime < m_fFadeDuration)
        return;

    g_bPauseTitanUpdate = false;
    g_bSkipOpeningOnce = true;
    SYS_CINEMATIC.Reset_Cinematic();
    GAME_INSTANCE.Request_RestartScene();
}

void CPlayerState_Grabbed::Decide_NextState()
{
}

std::shared_ptr<CPlayerState_Grabbed> CPlayerState_Grabbed::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Grabbed>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
