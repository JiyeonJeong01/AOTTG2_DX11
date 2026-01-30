#pragma once

#include "Editor_Define.h"

NS_BEGIN(Engine)
class CGameObject;
NS_END

NS_BEGIN(Editor_Util)

static bool Str_IContains(const std::string& haystack, const std::string& needle)
{
    if (needle.empty()) return true;
    if (haystack.size() < needle.size()) return false;

    auto it = std::search(
        haystack.begin(), haystack.end(),
        needle.begin(), needle.end(),
        [](unsigned char h, unsigned char n) {
            return std::tolower(h) == std::tolower(n);
        }
    );

    return it != haystack.end();
}

static uint64_t Stable_Id(CGameObject* pObj)
{
    return (uint64_t)(uintptr_t)pObj;
}

static bool Is_KeyChord_Pressed(bool ctrl, bool shift, bool alt, ImGuiKey key)
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl != ctrl) return false;
    if (io.KeyShift != shift) return false;
    if (io.KeyAlt != alt) return false;
    return ImGui::IsKeyPressed(key, false);
}

NS_END
