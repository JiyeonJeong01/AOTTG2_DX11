#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CCrawlerTitanState_Stunned final : public CTitanState
{
public:
    CCrawlerTitanState_Stunned(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CCrawlerTitanState_Stunned();

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

private:
    _uint   m_iPrevState = 0;
    _float  m_fElapsedStunnedTime = 0.f;
    _float  m_fMaxStunnedTime = 0.6f;

public:
    static std::shared_ptr<CCrawlerTitanState_Stunned> Create(
        Engine::CGameObject* goTitan,
        CTitan* scTitan,
        TITAN_STATE eState);
};

NS_END
