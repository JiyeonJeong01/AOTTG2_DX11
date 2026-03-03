#pragma once

#include <Windows.h>

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

namespace Client
{
    static unsigned int		g_iWinSizeX = 1920;
    static unsigned int		g_iWinSizeY = 1080;

    static unsigned int     g_SceneX = 1920;
    static unsigned int     g_SceneY = 1080;
}

#include "Engine_SDK.h"
