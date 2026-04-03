#pragma once

#include "StateMachine.h"
#include "Player_Struct.h"
#include "Event.h"

NS_BEGIN(Client)

class CPlayerState;
class CPlayer;

class CPlayerStateMachine final : public CStateMachine
{
public :
    CPlayerStateMachine();
    ~CPlayerStateMachine();

public :
    HRESULT Initialize(CGameObject* goPlayer, CPlayer* scPlayer);
    void Priority_Update(_float fDT);
    void Update(_float fDT);
    void Late_Update(_float fDT);

    void Cache_PlayerInfos(const PLAYER_CONTEXT& tContext);
    void Change_State(_uint iStateKey, _uint iDetailFlag = 0) override;
    void Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd);

    template <typename T>
    ListenerID Subscribe_OnChangedCurState(void(T::* func)(std::shared_ptr<CPlayerState>), T* pInstance)
    {
        return m_OnChanged_CurState.Add_Listener(func, pInstance);
    }

private:
    Engine::CGameObject*    m_goPlayer{};
    CPlayer*                m_scPlayer{};

    std::vector<std::shared_ptr<CPlayerState>>          m_States;
    std::shared_ptr<CPlayerState>                       m_spCurState;   
    Engine::CEvent<std::shared_ptr<CPlayerState>>       m_OnChanged_CurState;
    PLAYER_INPUT_COMMAND                                m_tInputCmd{};

private :

public :
    static std::unique_ptr<CPlayerStateMachine> Create(CGameObject* goPlayer, CPlayer* scPlayer);
};

NS_END
