#pragma once

#include "PlayerState.h"


class CPlayerState_Reload final : public CPlayerState
{
public:
    CPlayerState_Reload(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    virtual ~CPlayerState_Reload();

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
    void On_ReloadFinished(const Engine::ANIMATION_EVENT_DATA& tData);

public:
    static std::shared_ptr<CPlayerState_Reload> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);

private:
    RELOAD m_eReloadState = RELOAD::END;
};
