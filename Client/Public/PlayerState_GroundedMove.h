#pragma once
#include "PlayerState.h"

#include "Rigidbody.h"

NS_BEGIN(Client)

class CPlayerState_GroundedMove final : public CPlayerState
{
public :
    CPlayerState_GroundedMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    ~CPlayerState_GroundedMove();

public :
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag) override;
    void Exit() override;

    void Setup_CachedPlayerInfos() override;

private:
    void Decide_NextState() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_DashLandFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    void Move(_float fDT);

public :
    static std::shared_ptr<CPlayerState_GroundedMove> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};

NS_END
