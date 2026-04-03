#pragma once
#include "ScoutBehavior.h"

NS_BEGIN(Client)

class CScoutBehavior_Runaway final : public CScoutBehavior
{
public:
    CScoutBehavior_Runaway(Engine::CGameObject* goPlayer, CScout* scScout, SCOUT_BEHAVIOR eBehavior);
    ~CScoutBehavior_Runaway();

public:
    void Initialize() override;
    void Priority_Update(_float fDT) override;
    void Update(_float fDT) override;
    void Late_Update(_float fDT) override;

private :
    _float3 m_vTargetPos{};

private :
    void Runaway(_float fDT);

public :
    void Set_TargetPos(_fvector vPosition);

    static std::shared_ptr<CScoutBehavior_Runaway> Create(Engine::CGameObject* goPlayer, CScout* scScout, SCOUT_BEHAVIOR eBehavior);
};



NS_END
