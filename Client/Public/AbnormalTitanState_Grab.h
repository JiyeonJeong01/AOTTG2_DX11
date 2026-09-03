#pragma once

#include "TitanState.h"

NS_BEGIN(Client)

class CAbnormalTitanState_Grab final : public CTitanState
{
public:
    CAbnormalTitanState_Grab(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CAbnormalTitanState_Grab();

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

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_EatSlowFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    TITAN_GRAB      m_eGrabState = TITAN_GRAB::END;
    CTransform      m_trHuman;
    _float3* m_pGrabbedPoint{};
    _bool           m_bHumanDead = false;

public:
    static std::shared_ptr<CAbnormalTitanState_Grab> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
};



NS_END
