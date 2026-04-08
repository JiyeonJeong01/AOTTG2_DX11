#pragma once
#include "StateMachine.h"
#include "Titan_Struct.h"

NS_BEGIN(Client)
class CTitan;
class CTitanState;

class CTitanStateMachine abstract : public CStateMachine
{
protected :
    CTitanStateMachine();
    virtual ~CTitanStateMachine();
public:
    void Cache_TitanInfos(const TITAN_CONTEXT& tContext);
    void Change_State(_uint iStateKey, _uint iDetailFlag = 0);

protected:
    Engine::CGameObject*    m_goTitan{};
    CTitan*                 m_scTitan{};

    std::vector<std::shared_ptr<CTitanState>>          m_States;
    std::shared_ptr<CTitanState>                       m_spCurState;
    Engine::CEvent<std::shared_ptr<CTitanState>>       m_OnChanged_CurState;

public:
    template <typename T>
    ListenerID Subscribe_OnChangedCurState(void(T::* func)(std::shared_ptr<CTitanState>), T* pInstance)
    {
        return m_OnChanged_CurState.Add_Listener(func, pInstance);
    }
};



NS_END
