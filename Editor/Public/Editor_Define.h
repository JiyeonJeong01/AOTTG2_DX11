#pragma once

namespace Editor
{
    
}

#include <queue>
#include "Engine_Define.h"

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

/* -------- ImGui -------- */
#ifdef new
#undef new  
#endif
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_stdlib.h"

#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#endif
#define new DBG_NEW  

