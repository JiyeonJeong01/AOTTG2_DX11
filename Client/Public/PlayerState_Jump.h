#pragma once
#include "PlayerState.h"

NS_BEGIN(Client)

class CPlayerState_Jump final : public CPlayerState
{
public:
    CPlayerState_Jump(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
    ~CPlayerState_Jump();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Decide_NextState() override;
    void Enter(_uint iDetailFlag) override;



public:
    static std::shared_ptr<CPlayerState_Jump> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
};

NS_END
