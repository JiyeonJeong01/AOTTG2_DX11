#pragma once
#include "ScoutBehavior.h"

NS_BEGIN(Client)

class CPlayer;

class CScoutBehavior_RequestResupply final : public CScoutBehavior
{
public:
    CScoutBehavior_RequestResupply(Engine::CGameObject* goScout, CScout* scScout, SCOUT_BEHAVIOR eBehavior);
    ~CScoutBehavior_RequestResupply() override;

public:
    void Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

private:
    HRESULT SetUp_References();

    void OnTriggerEnter(const COLLISION_DESC& tCollisionDesc);
    void OnTriggerExit(const COLLISION_DESC& tCollisionDesc);

    _bool Is_Player(const CGameObject* pOther) const;
    void On_DetectedPlayer(_bool bDetected, CGameObject* pPlayer);
    void Try_Interact();

private:
    CGameObject* m_goDetectedPlayer = nullptr;
    _bool        m_bPlayerDetected = false;
    _float       m_fOutlineWidth = 3.f;
};

NS_END
