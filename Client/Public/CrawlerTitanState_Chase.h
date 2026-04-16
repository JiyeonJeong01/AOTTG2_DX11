#pragma once

#include "TitanState.h"

NS_BEGIN(Client)

class CCrawlerTitanState_Chase final : public CTitanState
{
public:
    CCrawlerTitanState_Chase(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CCrawlerTitanState_Chase();

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
    CGameObject*    m_goTarget = nullptr;
    _float3         m_vChaseDir{};
    _float3         m_vDetectDir{};
    _float          m_fChaseDist = FLT_MAX;

    _float          m_fShouldAttackDist = 4.f;
    _float          m_fSlowDownStartDist = 5.f;
    _float          m_fStopMoveDist = 2.f;

    _float          m_fOriginalRotationSharpness = 0.f;
    _float          m_fFastRotationSharpness = 5.f;

    _float          m_fStayAttackErenDist = 0.f;

    TITAN_CHASE     m_eChaseState = TITAN_CHASE::END;

    CHitBox*        m_pHitBoxL = nullptr;
    CHitBox*        m_pHitBoxR = nullptr;
private:

    void    Try_CacheHitBox();
    void    Set_ChaseHitBoxActive(_bool bActive);
public:
    static std::shared_ptr<CCrawlerTitanState_Chase> Create(
        Engine::CGameObject* goTitan,
        CTitan* scTitan,
        TITAN_STATE eState);
};

NS_END
