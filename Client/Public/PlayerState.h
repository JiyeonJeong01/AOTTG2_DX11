#pragma once
#include "State.h"
#include "Player_Struct.h"

NS_BEGIN(Client)

class CPlayer;
class CPlayerStateMachine;

class CPlayerState : public CState
{

public :
    CPlayerState(Engine::CGameObject* goPlayer, CPlayer* scPlayer) : m_goPlayer(goPlayer) , m_scPlayer(scPlayer){}
    ~CPlayerState() = default;

public :
    virtual HRESULT Initialize();
    virtual void    Priority_Update(_float fDT);;
    virtual void    Update(_float fDT);
    virtual void    Late_Update(_float fDT);;

    virtual void    Control_Camera();
    virtual void    Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd);

    void Cache_PlayerInfos(const PLAYER_COMPONENTS& tComponents, const PLAYER_RUNTIME_REF& tRef);
    void Bind_PlayerRef(const PLAYER_RUNTIME_REF& tRef);

private :
    virtual void    Decide_NextState() {};

protected:
    Engine::CGameObject*    m_goPlayer{};
    CPlayer*                m_scPlayer{};
    CPlayerStateMachine*    m_pFSM{};

    PLAYER_INPUT_COMMAND    m_tInputCmd{};
    PLAYER_COMPONENTS       m_tComponents{};
    PLAYER_RUNTIME_REF      m_tRef{};
};

NS_END
