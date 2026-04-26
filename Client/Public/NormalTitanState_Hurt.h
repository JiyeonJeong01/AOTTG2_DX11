#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CNormalTitanState_Hurt final : public CTitanState
{
public:
    CNormalTitanState_Hurt(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CNormalTitanState_Hurt();

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

    void    On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    TITAN_HURT  m_eHurtState = TITAN_HURT::END;
    _uint       m_iHurtAnimClip = INVALID_ANIM_CLIP_INDEX;

    _bool       m_bWaitRecovery = false;
    _float      m_fRecoveryTime = 3.f;
    _float      m_fRecoveryElapsed = 0.f;

    _bool       m_bLegDownFinished = false;

    TP_SFX      m_tHurtFSX = {  };

private :
    _bool   Is_LegHurt(TITAN_HURT eHurt) const;
    _bool   Is_ArmHurt(TITAN_HURT eHurt) const;
    _bool   Is_EyeHurt(TITAN_HURT eHurt) const;

    const char* Get_HurtAnimName(TITAN_HURT eHurt) const;

public:
    static std::shared_ptr<CNormalTitanState_Hurt> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
};

NS_END
