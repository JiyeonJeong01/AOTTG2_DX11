#pragma once
#include "PlayerState.h"

NS_BEGIN(Client)

class CPlayerState_Airborne final : public CPlayerState
{
public:
    CPlayerState_Airborne();
    ~CPlayerState_Airborne();

public:
    void Enter() override;


private:


public:
    static std::unique_ptr<CPlayerState_Airborne> Create(Engine::CGameObject* pPlayer);
};

NS_END
