#pragma once

#include "Client_Define.h"

typedef struct tagPlayerInputCommand
{
    _float2 vMouseDelta{};
    _float3 vMove{};

    _bool bLeftAnchorPressed = false;
    _bool bLeftAnchorHeld = false;
    _bool bRightAnchorPressed = false;
    _bool bRightAnchorHeld = false;
    _bool bBoostPressed = false;
    _bool bBoostHeld = false;
    _bool bNormalAttackPressed = false;
    _bool bStrongAttackPressed = false;
    /* TODO ::::::::::::::::::::::::::::::::: 확장 필수 ::::::::::::::::::::::::::::::::: */

}PLAYER_INPUT_COMMAND;


enum class PLAYER_STATE : uint8_t { IDLE, WALK, RUN, GRAPPLING, END };
