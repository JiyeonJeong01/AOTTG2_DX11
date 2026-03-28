#pragma once

#include <Windows.h>
#include "Engine_SDK.h"
#include "Engine_Component.h"
#include "Engine_Math.h"
#include "GameInstance.h"

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

namespace Client
{
    static unsigned int		g_iWinSizeX = 1920;
    static unsigned int		g_iWinSizeY = 1080;

    static unsigned int     g_SceneX = 1920;
    static unsigned int     g_SceneY = 1080;

    enum class SIDE {
        NONE = 0,
        LEFT = 1 << 0, // 1
        RIGHT = 1 << 1, // 2
        BOTH = To<_uint>(LEFT) | To<_uint>(RIGHT) // 3
    };
}



using namespace Client;
