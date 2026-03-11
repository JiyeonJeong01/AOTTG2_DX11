#pragma once

#include "Client_Define.h"
#include "Player_Struct.h"

NS_BEGIN(Engine)
    class CGameObject;
NS_END

NS_BEGIN(Client)

class CPlayer_InputController final
{
public :
    CPlayer_InputController();
    ~CPlayer_InputController();

public :
    const PLAYER_INPUT_COMMAND& Update_InputCommand();

private :
    PLAYER_INPUT_COMMAND    m_tInputCommand{};

public :
    static std::unique_ptr<CPlayer_InputController> Create();

};

NS_END
