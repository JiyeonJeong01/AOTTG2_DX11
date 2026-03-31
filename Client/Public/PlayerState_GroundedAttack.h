#pragma once
#include "PlayerState.h"

NS_BEGIN(Client)

class CPlayerState_GroundedAttack final : public CPlayerState
{
public:
    CPlayerState_GroundedAttack(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    ~CPlayerState_GroundedAttack();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag) override;
    void Exit() override;

    void Setup_CachedPlayerInfos() override;

private:
    void Decide_NextState() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_Attack2Finished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    GROUNDED_ATTACK     m_eGroundedAttackState = GROUNDED_ATTACK::ATK;

private:

public:
    static std::shared_ptr<CPlayerState_GroundedAttack> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};

NS_END
