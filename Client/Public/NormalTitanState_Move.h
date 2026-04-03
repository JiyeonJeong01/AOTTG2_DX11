#pragma once

#include "TitanState.h"

NS_BEGIN(Client)

class CNormalTitanState_Move final : public CTitanState
{
public:
    CNormalTitanState_Move(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CNormalTitanState_Move();

public:
    HRESULT Initialize() override;
    void    Priority_Update(_float fDT) override;
    void    Update(_float fDT) override;
    void    Late_Update(_float fDT) override;

    void    Enter(_uint iDetailFlag) override;
    void    Exit() override;

    void    Setup_CachedTitanContext() override;

private:
    void    Decide_NextState() override;
    void    Decide_NextAnim() override;

    void    Move(_float fDT);

private:
    TITAN_MOVE  m_eMoveState = TITAN_MOVE::WALK;

    _float      m_fElapsedMoveTime = 0.f;
    _float      m_fMaxMoveTime = 3.f;

public:
    static std::shared_ptr<CNormalTitanState_Move> Create( Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
};

NS_END
