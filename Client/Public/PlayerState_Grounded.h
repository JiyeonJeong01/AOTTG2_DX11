#pragma once
#include "PlayerState.h"

#include "Rigidbody.h"

NS_BEGIN(Client)

class CPlayerState_Grounded final : public CPlayerState
{
public :
    CPlayerState_Grounded(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
    ~CPlayerState_Grounded();

public :
    HRESULT Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

    void Decide_NextState() override;
    void Enter() override;

private :
    CRigidbody      m_rbPlayer;

private :
    void Walk(_float fDT);

public :
    static std::shared_ptr<CPlayerState_Grounded> Create(Engine::CGameObject* goPlayer, CPlayer* scPlayer);
};

NS_END
