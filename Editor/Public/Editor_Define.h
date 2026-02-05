#pragma once

#include <shellapi.h>
#include <filesystem>

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
        ASSET_TYPE              type = ASSET_TYPE::UNKNOWN;
        _bool                   isDirectory = false;
    }LIST_ASSET;

    typedef struct tagAssetSelection
    {
        std::filesystem::path   path;
        ASSET_TYPE              type = ASSET_TYPE::UNKNOWN;
        _bool                   isDirectory = false;

        _bool Is_Valid() const
        {
            return !path.empty();
        }
    }ASSET_SELECTION;

}


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

