#include "PlayerState.h"
#include "Animator.h"

NS_BEGIN(Client)

/* GROUNDED_MOVE, RELOAD, ATTACK */

class CPlayerState_AirborneMove final : public CPlayerState
{
    enum class AIRBORNE_STATE : uint8_t { AIR_BEGIN, AIR_LEFT, AIR_RIGHT, AIR_FRONT, AIR_BACK, AIR_FALL, END };
public:
    CPlayerState_AirborneMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
    ~CPlayerState_AirborneMove();

public:
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Decide_NextState() override;
    void Enter(_uint iDetailFlag) override;

    AIRBORNE_STATE          m_eState = AIRBORNE_STATE::AIR_BEGIN;

public:
    static std::shared_ptr<CPlayerState_AirborneMove> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
};

NS_END
