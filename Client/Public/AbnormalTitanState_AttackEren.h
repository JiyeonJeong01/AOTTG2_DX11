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
    void    Try_CacheHitBox();
    void    Set_PunchHitBoxActive(_bool bActive);
    void    Select_AttackAnim();
    void    Ready_Throw();
    void    Finish_Attack();
    _bool   Has_Target() const;
    void    Flush_Throw();
    void    Disable_ThrowRocks();
private:
    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    TITAN_ATTACK_EREN   m_eAtkEren = TITAN_ATTACK_EREN::WAIT;
    _bool               m_bThrown = false;
    CHitBox*            m_pPunchHitBoxL = nullptr;
    CHitBox*            m_pPunchHitBoxR = nullptr;
    std::vector<CGameObject*>   m_goThrowRockPool;

    _float              m_fAttackDist = 0.f;
    _float              m_fPunchAttackRange = 20.f;
    _float              m_fOriginRotateSharpness = 0.f;
    _float              m_fAttackRotateSharpness = 7.f;

    _uint               m_iAttackAnimClip = INVALID_ANIM_CLIP_INDEX;
    _bool               m_bAttackAnimPlaying = false;
    _bool               m_bUsePunch = false;
    _float3             m_vRockOffset = { 0.f, 15.f,  0.f};
    _float              m_fThrowImpulse = 50.f;
    _float              m_fElapsedThrownTime = 0.f;
    _float              m_fTotalThrownTime = 6.f;

    _bool               m_bFlushThrow = false;

    static constexpr _uint  s_iTotalRockCnt = 20;
    const string m_strRocks[5] = { TITAN_ROCK_1, TITAN_ROCK_2, TITAN_ROCK_3, TITAN_ROCK_4, TITAN_ROCK_5 };

private :
    void Make_ThrowDirs(_vector vDirs[20], const DISPLACEMENT& tInfo);

public:
    static std::shared_ptr<CAbnormalTitanState_AttackEren> Create(Engine::CGameObject* goTitan, CTitan* scTitan, TITAN_STATE eState);
};


NS_END
