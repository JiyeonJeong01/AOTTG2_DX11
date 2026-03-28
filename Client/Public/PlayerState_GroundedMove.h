#pragma once
#include "PlayerState.h"

#include "Rigidbody.h"

NS_BEGIN(Client)

class CPlayerState_GroundedMove final : public CPlayerState
{
public :
    CPlayerState_GroundedMove(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
    ~CPlayerState_GroundedMove();

public :
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Decide_NextState() override;
    void Enter(_uint iDetailFlag) override;

private :
    CRigidbody      m_rbPlayer;

private :
    void Move(_float fDT);

public :
    static std::shared_ptr<CPlayerState_GroundedMove> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
};

NS_END
