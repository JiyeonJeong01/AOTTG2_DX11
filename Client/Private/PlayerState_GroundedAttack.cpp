#include "PlayerState_GroundedAttack.h"

#include "Logger.h"
#include "GameObject.h"
#include "PlayerStateMachine.h"
#include "AnimationClip_Player.h"
#include "HitBox.h"
#include "ODM_Gear.h"

CPlayerState_GroundedAttack::CPlayerState_GroundedAttack(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
    : CPlayerState(goPlayer, scPlayer, eState)
{
}

CPlayerState_GroundedAttack::~CPlayerState_GroundedAttack()
{
}

HRESULT CPlayerState_GroundedAttack::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_goPlayer, E_FAIL, "m_goPlayer is nullptr");

    return S_OK;
}

void CPlayerState_GroundedAttack::Priority_Update(_float fDT)
{
    CPlayerState::Priority_Update(fDT);
    Control_Camera();
    LookTo_InputDir(fDT);
    Finish_Grappling();
    Handle_Trail(fDT, WIDTH_TYPE::NONE);
}

void CPlayerState_GroundedAttack::Update(_float fDT)
{
    CPlayerState::Update(fDT);

    CPlayerState::GroundedMove(fDT);
}

void CPlayerState_GroundedAttack::Late_Update(_float fDT)
{
    CPlayerState::Late_Update(fDT);

    if (m_bHitBoxActive && m_eGroundedAttackState == GROUNDED_ATTACK::ATK)
        CPlayerState::Sync_HiBox(PLAYER_BLADE_ATTACK);

    Decide_NextState();
}

void CPlayerState_GroundedAttack::Enter(_uint iDetailFlag)
{
    CPlayerState::Enter(iDetailFlag);

    if (iDetailFlag == To<_uint>(GROUNDED_ATTACK::ATK))
    {
        m_eGroundedAttackState = GROUNDED_ATTACK::ATK;
        m_tComponents.animator.Set_NextAnimationClip(ANIM_PLAYER::ATTACK_2);
    }

    if (!CPlayerState::Set_HitBoxActive(PLAYER_BLADE_ATTACK, true))
    {
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE), 0);
        return;
    }

    m_bHitBoxActive = true;
}

void CPlayerState_GroundedAttack::Exit()
{
    CPlayerState::Exit();

    CPlayerState::Set_HitBoxActive(PLAYER_BLADE_ATTACK, false);

    m_bHitBoxActive = false;
}

void CPlayerState_GroundedAttack::Setup_CachedPlayerContext()
{
    CPlayerState::Setup_CachedPlayerContext();

    m_tComponents.animator->OnAnimationFinished.Add_Listener(&CPlayerState_GroundedAttack::On_AnimFinished, this);
}

void CPlayerState_GroundedAttack::Decide_NextState()
{
    /* 공격 애니메이션은 끝까지 진행 */
    if (m_eGroundedAttackState == GROUNDED_ATTACK::ATK)
        return;

    /* -> GROUNDED_MOVE */
    _bool bHasInput = !XMVector3Equal(XMLoadFloat3(&m_tInputCmd.vMove), XMVectorZero());
    if (bHasInput)
    {

        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::GROUNDED_MOVE));
        return;
    }

    /* -> IDLE */
    if (!bHasInput)
    {
        m_tRef.pFSM->Change_State(To<_uint>(PLAYER_STATE::IDLE));
    }
}

void CPlayerState_GroundedAttack::On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    if (!m_bAcivated)
        return;

    cout << "[GROUNDED_ATTACK] On_AnimFinished\n";

    const _uint iIndex = tData.iAnimationClip;
    if (iIndex == INVALID_ANIM_CLIP_INDEX)
        return;

    if (iIndex == m_tComponents.animator->NameToClipIndex[ANIM_PLAYER::ATTACK_2])
        On_Attack2Finished(tData);
}


void CPlayerState_GroundedAttack::On_Attack2Finished(const Engine::ANIMATION_EVENT_DATA& tData)
{
    m_eGroundedAttackState = GROUNDED_ATTACK::END;
    cout << " => [GROUNDED_ATTACK] On_Attack2Finished\n";
}


std::shared_ptr<CPlayerState_GroundedAttack> CPlayerState_GroundedAttack::Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState)
{
    auto pInstance = std::make_shared<CPlayerState_GroundedAttack>(goPlayer, scPlayer, eState);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create faild");
    return pInstance;
}
