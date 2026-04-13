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

    void    Cache_TitanContext(const TITAN_CONTEXT& tContext) override;
    void    Setup_CachedTitanContext() override;

    _uint   Get_DetailState() const override;

private:
    void    Decide_NextState() override;
    void    Decide_NextAnim() override;

private:
    CGameObject*    m_goTarget{};
    _float3         m_vChaseDir{};
    _float3         m_vDetectDir{};
    _float          m_fChaseDist = FLT_MAX;

    _bool           m_bGrabAnimPlaying = false;
    _float          m_fTriggeredGrabAnimCooldownElapsed = 0.f;
    _float          m_fTriggeredGrabAnimCooldown = 0.2f;
    _float          m_fProximityGrabAnimCooldownElapsed = 0.f;
    _float          m_fProximityGrabAnimCooldown = 0.2f;
    _uint           m_iGrabAnimClip = INVALID_ANIM_CLIP_INDEX;

    _float          m_fShouldGrabDist = 8.5f;
    _float          m_fSlowDownStartDist = 5.f;
    _float          m_fStopMoveDist = 2.f;

    _float          m_fOriginalRotationSharpness = 0.f;
    _float          m_fFastRotationSharpness = 5.f;

    _float          m_fStayAttackErenDist = 0.f;

    TITAN_CHASE     m_eChaseState = TITAN_CHASE::END;
    _char           m_szChaseAnimName[32];

private:
    void    Move(_float fDT);
    void    Try_PlayTriggeredGrabAnim();
    void    Try_PlayProximityGrabAnim();
    void    On_SuccessGrabHuman(SIDE eSid, _float3* vGrabPoint, CHuman* pHuman);

    void    On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

public:
    static std::shared_ptr<CAbnormalTitanState_Chase> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);

};

NS_END
