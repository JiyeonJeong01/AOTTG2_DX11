#include "PlayerState.h"
#include "Animator.h"

NS_BEGIN(Client)

/* GROUNDED_MOVE, RELOAD, ATTACK */

class CPlayerState_AirborneMove final : public CPlayerState
{
public:
    CPlayerState_AirborneMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
    ~CPlayerState_AirborneMove();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Enter(_uint iDetailFlag) override;
    void Exit() override;

    void Setup_CachedPlayerInfos() override;

private :
    void Decide_NextState() override;

    void On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void On_AirHookFinished(const Engine::ANIMATION_EVENT_DATA& tData);

private :
    AIRBORNE_STATE          m_eAirborneState = AIRBORNE_STATE::AIR_BEGIN;

public:
    static std::shared_ptr<CPlayerState_AirborneMove> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer, PLAYER_STATE eState);
};



NS_END
