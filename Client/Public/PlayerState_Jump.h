#pragma once
#include "PlayerState.h"

NS_BEGIN(Client)

class CPlayerState_Jump final : public CPlayerState
{
public:
    CPlayerState_Jump(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    ~CPlayerState_Jump();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag) override;
    void Exit() override;

    void Setup_CachedPlayerInfos() override;

private :
    JUMP    m_eJumpState = JUMP::JUMP_BEGIN;

private :
    void Decide_NextAnim() override;
    void Decide_NextState() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_JumpFinished(const Engine::ANIMATION_EVENT_DATA& tData);


public:
    static std::shared_ptr<CPlayerState_Jump> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};

NS_END
