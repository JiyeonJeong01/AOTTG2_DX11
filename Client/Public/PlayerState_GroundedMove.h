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

    void Cache_PlayerContext(const PLAYER_CONTEXT& tContext) override;
    void Setup_CachedPlayerContext() override;

private:
    void Decide_NextState() override;
    void Decide_NextAnim() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_DashLandFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private:
    GROUNDED_MOVE   m_eGroundedMoveState = GROUNDED_MOVE::RUN;
    _float          m_fOriginDrag = 0.f;
    const _float    m_fSlidingDrag = 0.1f;
    const _float    m_fRunCorrectionDT = 8.f;
    const _float    m_fSlideThreshold = 4.f;

public :
    static std::shared_ptr<CPlayerState_GroundedMove> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};

NS_END
