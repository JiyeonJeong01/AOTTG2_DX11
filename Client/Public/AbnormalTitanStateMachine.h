#pragma once
#include "Titan_Struct.h"
#include "TitanStateMachine.h"

NS_BEGIN(Client)

class CTitanState;

class CAbnormalTitanStateMachine : public CTitanStateMachine
{
public:
    CAbnormalTitanStateMachine();
    ~CAbnormalTitanStateMachine() override;

public:
    HRESULT Initialize(Engine::CGameObject* goTitan, CTitan* scTitan);
    void Priority_Update(_float fDT);
    void Update(_float fDT);
    void Late_Update(_float fDT);

public:
    static std::unique_ptr<CAbnormalTitanStateMachine> Create(CGameObject* goTitan, CTitan* scTitan);
};

NS_END
