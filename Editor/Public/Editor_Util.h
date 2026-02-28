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

static _bool Write_TextFile(const std::filesystem::path& p, const std::string& s)
{
    std::filesystem::create_directories(p.parent_path());
    std::ofstream ofs(p, std::ios_base::binary);
    if (!ofs.is_open()) return false;
    ofs.write(s.data(), (std::streamsize)s.size());
    return true;
}

static std::string Make_HeaderText(const std::string& className)
{
    // 최소 템플릿: ctx/dt 시그니처는 당신 Binder에 맞춘 형태
    std::string s;
    s += "#pragma once\n";
    s += "#include \"Script.h\"\n\n";
    s += "NS_BEGIN(Engine)\n\n";
    s += "class " + className + "\n";
    s += "{\n";
    s += "public:\n";
    s += "    void Awake(SCRIPT_CTX& ctx) {}\n";
    s += "    void Start(SCRIPT_CTX& ctx) {}\n";   
    s += "    void Update(SCRIPT_CTX& ctx, _float dt) {(void)ctx; (void)dt;}\n";
    s += "};\n\n";
    s += "NS_END\n";
    return s;
}

static std::string MakeCppText(const std::string& className)
{
    std::string s;
    s += "#include \"" + className + ".h\"\n";
    s += "#include \"Script_Type_Register.h\"\n\n";
    s += "REGISTER_SCRIPT_TYPE(" + className + ", \"Scripts/" + className + ".script\");\n";
    return s;
}

// className: "PlayerMove" 같은 C++ 식별자
// outGuid: 생성된 .script 에셋 GUID
static bool CreateScriptFilesAndAsset(const std::string& className,
    const std::filesystem::path& sourceScriptsDir,
    ASSET_GUID& outGuid)
{
    outGuid = ASSET_GUID{};

    // 1) .script 에셋(슬롯) 생성: AssetsRoot 기준 상대경로
    // "Scripts/Name.script"
    const std::filesystem::path scriptAssetRel = std::filesystem::path("Scripts") / (className + ".script");
    outGuid = SYS_ASSET.Ensure_GUID_For_Path(scriptAssetRel); // 당신이 만든 Ensure_GUID_For_Path 사용

    if (!outGuid.Is_Valid())
        return false;

    // 2) C++ 파일 생성
    const std::filesystem::path hPath = sourceScriptsDir / (className + ".h");
    const std::filesystem::path cppPath = sourceScriptsDir / (className + ".cpp");

    //if (!WriteTextFile(hPath, MakeHeaderText(className))) return false;
    //if (!WriteTextFile(cppPath, MakeCppText(className)))   return false;

    _DEBUG_INFO("Created script: %s / %s (guid=%s)",
        hPath.string().c_str(), cppPath.string().c_str(), outGuid.To_String_Utf8().c_str());

    return true;
}

NS_END
