#pragma once

#include "TitanState.h"

NS_BEGIN(Client)

class CNormalTitanState_Grab final : public CTitanState
{
public:
    CNormalTitanState_Grab(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CNormalTitanState_Grab();

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

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_EatSlowFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    CTransform      m_trHuman;
    _float3*        m_pGrabbedPoint{};

public:
    static std::shared_ptr<CNormalTitanState_Grab> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
};



NS_END
