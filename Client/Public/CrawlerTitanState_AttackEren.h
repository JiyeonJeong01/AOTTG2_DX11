#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CCrawlerTitan;
class CHitBox;

class CCrawlerTitanState_AttackEren final : public CTitanState
{
public:
    CCrawlerTitanState_AttackEren(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CCrawlerTitanState_AttackEren() override;

public:
    HRESULT Initialize() override;

    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag = 0) override;
    void Exit() override;

    _uint Get_DetailState() const override;

private:
    void Cache_TitanContext(const TITAN_CONTEXT& tContext) override;
    void Setup_CachedTitanContext() override;

private:
    void    Try_CacheHitBox();
    void    Set_BodyHitBoxActive(_bool bActive);

    void    Play_JumpStart();
    void    Play_JumpAir();
    void    Play_JumpLand();

    void    Finish_Attack();
    _bool   Has_Target() const;

private:
    void    On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    CHitBox* m_pBodyHitBox = nullptr;

    _float              m_fDistToEren = 0.f;
    _float              m_fJumpAttackRange = 17.f;
    _float              m_fOriginRotateSharpness = 0.f;
    _float              m_fAttackRotateSharpness = 7.f;

    _float              m_fStayAttackErenDist = 0.f;

    _uint               m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
    _bool               m_bAttackAnimPlaying = false;
    _bool               m_bBodyHitBoxActive = false;
    _bool               m_bJumpStarted = false;
    _bool               m_bJumpAir = false;
    _bool               m_bJumpLanded = false;

public:
    static std::shared_ptr<CCrawlerTitanState_AttackEren> Create(
        Engine::CGameObject* goTitan,
        CTitan* scTitan,
        TITAN_STATE eState);
};

NS_END
