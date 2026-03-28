#pragma once
#include "PlayerState.h"
#include "Animator.h"

NS_BEGIN(Client)

/* MOVE, RELOAD, ATTACK */

class CPlayerState_Idle final : public CPlayerState
{
public:
    CPlayerState_Idle(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
    ~CPlayerState_Idle();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Decide_NextState() override;
    void Enter() override;

public:
    static std::shared_ptr<CPlayerState_Idle> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
};

NS_END
