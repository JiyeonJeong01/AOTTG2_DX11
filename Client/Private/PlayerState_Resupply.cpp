#include "PlayerState_Resupply.h"

#include "Logger.h"
#include "GameObject.h"
#include "PlayerStateMachine.h"
#include "AnimationClip_Player.h"
#include "ODM_Gear.h"

CPlayerState_Resupply::CPlayerState_Resupply(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_Resupply::~CPlayerState_Resupply()
{
}

HRESULT CPlayerState_Resupply::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");

    return S_OK;
}

void CPlayerState_Resupply::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_Resupply::On_AnimFinished, this);
}

void CPlayerState_Resupply::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);

    Control_Camera();
    LookTo_InputDir(fDT);
    Finish_Grappling();
    Handle_Trail(fDT, WIDTH_TYPE::NONE);
    Handle_BladeTrail(fDT, WIDTH_TYPE::NONE);
}

void CPlayerState_Resupply::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    //CPlayerState::GroundedMove(fDT);
}

void CPlayerState_Resupply::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    Decide_NextState();
}

void CPlayerState_Resupply::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    m_bFinished = false;

    m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::RESUPPLY);

    SYS_SOUND.PlayForceSFX(L"Human_Resupply", CHANNEL_1, 1.f);
}

void CPlayerState_Resupply::Exit()
{
    if (m_pBlade)
    {
        m_pBlade->iCurAtkRemain = m_pBlade->iNumAtkPerBlade;
        m_pBlade->iCurBladesRemain = m_pBlade->iTotalNumBlades;
        m_pBlade->Enable_Blades(true);
    }

    if (m_tRef.pGear)
        m_tRef.pGear->Fill_Max();

    CPlayerState::Exit();
}

void CPlayerState_Resupply::Decide_NextState()
{
    if (!m_bFinished)
        return;

    _bool bHasInput = !XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero());
    if (bHasInput)
    {
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE));
        return;
    }

    m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE));
}

void CPlayerState_Resupply::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[RESUPPLY] On_AnimFinished\n";

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::RESUPPLY])
        On_ResupplyFinished(tData);
}

void CPlayerState_Resupply::On_ResupplyFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_bFinished = true;
    cout << " => [RESUPPLY] On_ResupplyFinished\n";
}

std::shared_ptr<CPlayerState_Resupply> CPlayerState_Resupply::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_Resupply>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
