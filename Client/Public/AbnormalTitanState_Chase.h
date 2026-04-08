#pragma once

#include "TitanState.h"

NS_BEGIN(Client)

class CTitanBound_Controller;

class CAbnormalTitanState_Chase final : public CTitanState
{
public:
    CAbnormalTitanState_Chase(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CAbnormalTitanState_Chase();

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

private:
    CGameObject* m_goTarget{};
    _float3         m_vChaseDir{};
    _float          m_fChaseDist = FLT_MAX;

    _bool           m_bGrabAnimPlaying = false;
    _float          m_fGrabAnimCooldownElapsed = 0.f;
    _float          m_fGrabAnimCooldown = 0.f;
    _uint           m_iGrabAnimClip = INVALID_ANIM_CLIP_INDEX;


    TITAN_CHASE     m_eChaseState = TITAN_CHASE::END;
private:
    _float  Get_ChaseDist();
    void    Move(_float fDT);
    void    Try_PlayTriggeredGrabAnim();
    void    On_SuccessGrabHuman(SIDE eSid, _float3* vGrabPoint, CHuman* pHuman);

    void    On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

public:
    static std::shared_ptr<CAbnormalTitanState_Chase> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);

};

NS_END
