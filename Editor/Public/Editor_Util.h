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

/* ImGui is UTF-8 based, so file paths must be converted to UTF-8. */
static std::string To_UTF8(const std::filesystem::path& p)
{
    auto u8 = p.u8string();
    return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
}

static std::filesystem::path Get_AssetRoot() {
    return std::filesystem::path(Engine::ProjectConfig::PATH) / Engine::ProjectConfig::ROOT ;
}

static std::filesystem::path Get_SceneRoot() {
    // path / path 연산자는 폴더 구분자를 자동으로 관리합니다.
    return std::filesystem::path(Engine::ProjectConfig::PATH) / Engine::ProjectConfig::ROOT / "Scene";
}

static std::wstring OpenFileDialog(const wchar_t* filter, const wchar_t* initialDir)
{
    OPENFILENAMEW ofn;
    wchar_t szFile[260] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    ofn.lpstrInitialDir = initialDir;

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn) == TRUE)
    {
        return std::wstring(szFile);
    }

    return L"";
}


static std::wstring SaveFileDialog(const wchar_t* filter, const wchar_t* initialDir = nullptr)
{
    OPENFILENAMEW ofn;
    wchar_t szFile[260] = { 0 };

    std::wstring fixedInitialDir = L"";
    if (initialDir) {
        std::filesystem::path p(initialDir);

        if (!std::filesystem::exists(p)) {
            std::error_code ec;
            std::filesystem::create_directories(p, ec);
        }

        fixedInitialDir = p.make_preferred().wstring();
    }

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;

    if (!fixedInitialDir.empty() && fixedInitialDir.back() != L'\\') {
        fixedInitialDir += L'\\';
    }

    ofn.lpstrInitialDir = fixedInitialDir.empty() ? NULL : fixedInitialDir.c_str();

    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = L"scene";

    if (GetSaveFileNameW(&ofn) == TRUE)
    {
        return std::wstring(szFile);
    }

    return L"";
}

NS_END
