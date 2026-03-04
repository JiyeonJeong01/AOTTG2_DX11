#pragma once

#include "Editor_Define.h"
#include "Asset_Registry.h"

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

static void OpenFile_In_OS(const std::filesystem::path& p)
{
#if defined(_WIN32)
    const std::wstring w = p.wstring();
    ShellExecuteW(nullptr, L"open", w.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
    (void)p;
#endif
}

static void RevealFile_In_Explorer(const std::filesystem::path& p)
{
#if defined(_WIN32)
    std::wstring cmd = L"/select,\"" + p.wstring() + L"\"";
    ShellExecuteW(nullptr, L"open", L"explorer.exe", cmd.c_str(), nullptr, SW_SHOWNORMAL);
#else
    (void)p;
#endif
}

static void OpenFile_In_VisualStudio(const std::filesystem::path& p)
{
    if (p.empty() || !std::filesystem::exists(p))
        return;

    std::string vsPath = "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\devenv.exe";
    ShellExecuteA(NULL, "edit", p.string().c_str(), NULL, NULL, SW_SHOWNORMAL);
}

static void Transpose16(const float* src16, float* dst16)
{
    float t[16];
    memcpy(t, src16, sizeof(float) * 16);

    dst16[0] = t[0];  dst16[1] = t[4];  dst16[2] = t[8];  dst16[3] = t[12];
    dst16[4] = t[1];  dst16[5] = t[5];  dst16[6] = t[9];  dst16[7] = t[13];
    dst16[8] = t[2];  dst16[9] = t[6];  dst16[10] = t[10]; dst16[11] = t[14];
    dst16[12] = t[3];  dst16[13] = t[7];  dst16[14] = t[11]; dst16[15] = t[15];
}

// =============================================================
// Drag & Drop 공용 유틸
// - Payload가 ASSET_GUID로 오면 그대로 사용한다.
// - Payload가 ASSET_SELECTION(경로 포함)로 오면 경로 -> GUID로 Resolve한다.
// - 타입 체크는 caller가 원하는 대로(Shader/Texture 등) 걸어줄 수 있게 분리한다.
// =============================================================

/* NOTE:
 * payloadName 은 ProjectPanel에서 SetDragDropPayload에 쓰는 이름과 반드시 같아야 한다.
 * 예) ImGui::SetDragDropPayload("ASSET_GUID", &guid, sizeof(ASSET_GUID));
 */

inline _bool Try_Get_GUID_From_Payload(const ImGuiPayload* payload, ASSET_GUID& outGUID)
{
    if (!payload || !payload->Data || payload->DataSize <= 0)
        return false;

    /* Payload가 GUID 자체인 경우 */
    if (payload->DataSize == sizeof(ASSET_GUID))
    {
        outGUID = *reinterpret_cast<const ASSET_GUID*>(payload->Data);
        return outGUID.Is_Valid();
    }

    /* Payload가 ASSET_SELECTION인 경우 */
    if (payload->DataSize == sizeof(Editor::ASSET_SELECTION))
    {
        const Editor::ASSET_SELECTION& sel = *reinterpret_cast<const Editor::ASSET_SELECTION*>(payload->Data);

        if (sel.path.empty())
            return false;

        /* 경로 -> GUID : 이미 레지스트리에 meta가 있고 guid 매핑이 있으니 여기서 resolve */
        return SYS_ASSET.Try_Get_GUID(sel.path, outGUID);
    }

    return false;
}

template<typename TOnAccept>
inline _bool Draw_DropTarget_GUID(
    const char* label,
    const char* payloadName,
    TOnAccept&& onAccept,
    const char* tooltip = nullptr
)
{
    _bool changed = false;

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadName))
        {
            ASSET_GUID dropped{};
            if (Try_Get_GUID_From_Payload(payload, dropped))
            {
                onAccept(dropped);
                changed = true;
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (tooltip && ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", tooltip);

    return changed;
}

/* 타입 체크(optional) */
template<typename TOnAccept>
inline _bool Draw_DropTarget_GUID_Typed(
    const char* label,
    const char* payloadName,
    ASSET_TYPE allowedType,
    TOnAccept&& onAccept,
    const char* tooltip = nullptr
)
{
    return Draw_DropTarget_GUID(label, payloadName,
        [&](const ASSET_GUID& dropped)
        {
            const ASSET_TYPE t = SYS_ASSET.Find(dropped)->eType;
            if (t != allowedType)
                return;

            onAccept(dropped);
        },
        tooltip
    );
}

NS_END
