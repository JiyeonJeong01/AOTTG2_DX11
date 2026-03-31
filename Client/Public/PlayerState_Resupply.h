#pragma once

#include "PlayerState.h"

class CPlayerState_Resupply final : public CPlayerState
{
public:
    CPlayerState_Resupply(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    virtual ~CPlayerState_Resupply();

public:
    virtual HRESULT Initialize() override;

    virtual void Setup_CachedPlayerInfos() override;

    virtual void Priority_Update(_float fDT) override;
    virtual void Update(_float fDT) override;
    virtual void Late_Update(_float fDT) override;

    virtual void Enter(_uint iDetailFlag = 0) override;
    virtual void Exit() override;

private:
    void Decide_NextState();

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_ResupplyFinished(const Engine::ANIMATION_EVENT_DATA& tData);

public:
    static std::shared_ptr<CPlayerState_Resupply> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);

private:
    _bool m_bFinished = false;
};
