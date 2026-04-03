#pragma once

#include "PlayerState.h"

class CPlayerState_Dodge final : public CPlayerState
{
public:
    CPlayerState_Dodge(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    virtual ~CPlayerState_Dodge();

public:
    virtual HRESULT Initialize() override;

    virtual void Setup_CachedPlayerContext() override;

    virtual void Priority_Update(_float fDT) override;
    virtual void Update(_float fDT) override;
    virtual void Late_Update(_float fDT) override;

    virtual void Enter(_uint iDetailFlag = 0) override;
    virtual void Exit() override;

private:
    void Decide_NextState();
    void Move_Dodge();

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_DodgeFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    _bool   m_bFinished = false;

    _float3 m_vDodgeDir{};
    _float  m_fDodgeImpulse = 18.f;
    _float  m_fDodgeAssistForce = 5.f;
    _float  m_fDodgeMaxSpeedMul = 1.15f;

public:
    static std::shared_ptr<CPlayerState_Dodge> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};
