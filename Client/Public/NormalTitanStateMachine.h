#pragma once
#include "StateMachine.h"
#include "Titan_Struct.h"

NS_BEGIN(Client)

class CTitanState;

class CNormalTitanStateMachine : public CStateMachine
{
public:
    CNormalTitanStateMachine();
    ~CNormalTitanStateMachine();

public:
    HRESULT Initialize(Engine::CGameObject* goTitan, CTitan* scTitan);
    void Priority_Update(_float fDT);
    void Update(_float fDT);
    void Late_Update(_float fDT);

    void Cache_TitanInfos(const TITAN_CONTEXT& tContext);
    void Change_State(_uint iStateKey, _uint iDetailFlag = 0) override;

    template <typename T>
    ListenerID Subscribe_OnChangedCurState(void(T::* func)(std::shared_ptr<CTitanState>), T* pInstance)
    {
        return m_OnChanged_CurState.Add_Listener(func, pInstance);
    }

private:
    Engine::CGameObject*    m_goTitan{};
    CTitan*                 m_scTitan{};

    std::vector<std::shared_ptr<CTitanState>>          m_States;
    std::shared_ptr<CTitanState>                       m_spCurState;
    Engine::CEvent<std::shared_ptr<CTitanState>>       m_OnChanged_CurState;

private:

public:
    static std::unique_ptr<CNormalTitanStateMachine> Create(CGameObject* goTitan, CTitan* scTitan);


};

NS_END
