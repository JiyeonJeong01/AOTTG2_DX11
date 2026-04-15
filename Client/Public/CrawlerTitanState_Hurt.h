#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CCrawlerTitanState_Hurt final : public CTitanState
{
public:
    CCrawlerTitanState_Hurt(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CCrawlerTitanState_Hurt();

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

    void    On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    TITAN_HURT  m_eHurtState = TITAN_HURT::END;
    _uint       m_iHurtAnimClip = INVALID_ANIM_CLIP_INDEX;

public:
    static std::shared_ptr<CCrawlerTitanState_Hurt> Create(
        Engine::CGameObject* goTitan,
        CTitan* scTitan,
        TITAN_STATE eState);
};

NS_END
