#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CCrawlerTitanState_Dead final : public CTitanState
{
public:
    CCrawlerTitanState_Dead(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CCrawlerTitanState_Dead();

public:
    HRESULT Initialize() override;
    void    Priority_Update(_float fDT) override;
    void    Update(_float fDT) override;
    void    Late_Update(_float fDT) override;

    void    Enter(_uint iDetailFlag) override;
    void    Exit() override;

    void    Setup_CachedTitanContext() override;
    _uint   Get_DetailState() const override;

private:
    void    Decide_NextState() override;
    void    Decide_NextAnim() override;

public:
    static std::shared_ptr<CCrawlerTitanState_Dead> Create(
        Engine::CGameObject* goTitan,
        CTitan* scTitan,
        TITAN_STATE eState);
};

NS_END
