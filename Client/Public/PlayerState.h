#pragma once
#include "State.h"
#include "Player_Struct.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Client)

class CPlayer;

class CPlayerState : public CState
{
public :
    CPlayerState(Engine::CGameObject* goPlayer, CPlayer* scPlayer) : m_goPlayer(goPlayer) , m_scPlayer(scPlayer){}
    ~CPlayerState() = default;

public :
    virtual HRESULT Initialize() { return S_OK; };
    virtual void    Update_PlayerInput(const PLAYER_INPUT_COMMAND& tInputCmd) { m_tInputCmd = tInputCmd; };
    virtual void    Priority_Update(_float fDT) {};
    virtual void    Update(_float fDT) {};
    virtual void    Late_Update(_float fDT) {};

private :
    virtual void    Decide_NextState() {};

protected:
    Engine::CGameObject*    m_goPlayer{};
    CPlayer*                m_scPlayer{};

    PLAYER_INPUT_COMMAND    m_tInputCmd{};

};

NS_END
