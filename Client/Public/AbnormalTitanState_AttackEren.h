#pragma once
#include "TitanState.h"

NS_BEGIN(Client)

class CAbnormalTitan;
class CHitBox;

class CAbnormalTitanState_AttackEren final : public CTitanState
{
public:
    CAbnormalTitanState_AttackEren(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
    ~CAbnormalTitanState_AttackEren() override;

public:
    HRESULT Initialize() override;

    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag = 0) override;
    void Exit() override;

    _uint Get_DetailState() const override;

private:
    void Setup_CachedTitanContext() override;

private:
    void    Try_CachePunchHitBox();
    void    Set_PunchHitBoxActive(_bool bActive);
    void    Select_AttackAnim();
    void    Attack_Throw();
    void    Finish_Attack();
    _bool   Has_Target() const;

private:
    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    CHitBox*            m_pPunchHitBoxL = nullptr;
    CHitBox*            m_pPunchHitBoxR = nullptr;

    _float              m_fAttackDist = 0.f;
    _float              m_fPunchAttackRange = 20.f;
    _float              m_fOriginRotateSharpness = 0.f;
    _float              m_fAttackRotateSharpness = 0.9f;

    _uint               m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
    _bool               m_bAttackAnimPlaying = false;
    _bool               m_bUsePunch = false;

public:
    static std::shared_ptr<CAbnormalTitanState_AttackEren> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
};

NS_END
