#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CAbnormalTitanState_Stunned final : public CTitanState
{
public:
    CAbnormalTitanState_Stunned(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CAbnormalTitanState_Stunned();

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

    _uint   m_iPrevState = 0;
    _float  m_fElapsedStunnedTime = 0.f;
    _float  m_fMaxStunnedTime = 1.f;

public:
    static std::shared_ptr<CAbnormalTitanState_Stunned> Create(
        Engine::CGameObject* goTitan,
        CTitan* scTitan,
        TITAN_STATE eState);
};

NS_END
