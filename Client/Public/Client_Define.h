#pragma once

#include <Windows.h>

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

namespace Client
{
    const unsigned int		g_iWinSizeX = { 1280 };
    const unsigned int		g_iWinSizeY = { 720 };
}

using namespace Client;

#include "Engine_Define.h"
#include "Engine_Log.h"
