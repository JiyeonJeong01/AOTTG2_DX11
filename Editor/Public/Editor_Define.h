#pragma once

#include "Engine_SDK.h"
#include <psapi.h>

namespace Editor
{
    typedef struct tagFolderNode
    {
        std::filesystem::path path;
        std::string           name;
        std::vector<tagFolderNode> children;
    }FOLDER_NODE;

    typedef struct tagListAsset
    {
        std::filesystem::path   path;
        std::string             name;
        Engine::ASSET_TYPE      type = Engine::ASSET_TYPE::UNKNOWN;
        _bool                   isDirectory = false;
    }LIST_ASSET;

    typedef struct tagAssetSelection
    {
        std::filesystem::path   path;
        Engine::ASSET_TYPE      type = Engine::ASSET_TYPE::UNKNOWN;
        _bool                   isDirectory = false;

        _bool Is_Valid() const
        {
            return !path.empty();
        }
    }ASSET_SELECTION;


}

extern HWND g_hWnd;
extern HINSTANCE g_hInst;

#define GET_INSTANCE(CLASSNAME) CLASSNAME::GetInstance()
#define SYS_GUI			    	GET_INSTANCE(Editor::CGUI_System)

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

