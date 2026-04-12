#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CAbnormalTitanState_Idle final : public CTitanState
{
public:
    CAbnormalTitanState_Idle(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CAbnormalTitanState_Idle();

public:
    HRESULT Initialize() override;
    void    Priority_Update(_float fDT) override;
    void    Update(_float fDT) override;
    void    Late_Update(_float fDT) override;

    void    Enter(_uint iDetailFlag) override;
    void    Exit() override;

    void    Cache_TitanContext(const TITAN_CONTEXT& tContext) override;
    void    Setup_CachedTitanContext() override;
    _uint   Get_DetailState() const override;

private:
    void    Decide_NextState() override;
    void    Decide_NextAnim() override;

private:
    TITAN_IDLE  m_eIdleState = TITAN_IDLE::DEFAULT;
    _float      m_fElapsedIdleTime = 0.f;
    _float      m_fMaxIdleTime = 5.f;

public:
    static std::shared_ptr<CAbnormalTitanState_Idle> Create(
        Engine::CGameObject* goTitan,
        CTitan* scTitan,
        TITAN_STATE eState);
};

NS_END
