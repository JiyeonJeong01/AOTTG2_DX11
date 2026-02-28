#include "Script_Handler.h"

#include "Asset_Registry.h"
#include "Engine_Log.h"

std::vector<ASSET_GUID> CScript_Handler::m_CachedGUID;

CScript_Handler::CScript_Handler()
{
}

CScript_Handler::~CScript_Handler()
{
}

void CScript_Handler::Clear()
{
    m_VTableMap.clear();
    m_CachedGUID.clear();
}

void CScript_Handler::Cache_GUID(const ASSET_GUID& tGUID)
{
    m_CachedGUID.push_back(tGUID);
}

void CScript_Handler::Register_VTable(const ASSET_GUID& tGUID, const SCRIPT_VTABLE& vt)
{
    if(m_VTableMap.find(tGUID) == m_VTableMap.end())
        m_VTableMap[tGUID] = vt;
}

const SCRIPT_VTABLE* CScript_Handler::Find(const ASSET_GUID& tGUID) const
{
    auto it = m_VTableMap.find(tGUID);
    if (it == m_VTableMap.end())
        return nullptr;
    return &it->second;
}

HRESULT CScript_Handler::Generate(const std::filesystem::path& outCppPath)
{
    struct ScriptInfo
    {
        ASSET_GUID   tGUID{};
        std::string  assetPath;   // "Assets/....script"
        std::string  className;   // "CScript_Dog"
        std::string  headerPath;  // "CScript_Dog.h"
    };

    std::vector<ScriptInfo> scripts;
    scripts.reserve(128);

    for (const auto& tGUID : m_CachedGUID)
    {
        auto pRec = SYS_ASSET.Find(tGUID);
        if (!pRec) continue;

        /* 에셋 경로(.script) */
        std::u8string u8Str = pRec->path.u8string();
        std::string assetPathUtf8(u8Str.begin(), u8Str.end());

        /* .script 파일을 열어서 JSON에서 ClassName/Header를 읽는다 */
        std::ifstream ifs(pRec->path, std::ios_base::binary);
        if (!ifs.is_open())
            continue;

        json j;
        try
        {
            ifs >> j;
        }
        catch (...)
        {
            continue;
        }

        if (!j.contains("ClassName") || !j["ClassName"].is_string())
            continue;
        if (!j.contains("Header") || !j["Header"].is_string())
            continue;

        ScriptInfo info;
        info.tGUID = tGUID;
        info.assetPath = assetPathUtf8;
        info.className = j["ClassName"].get<std::string>();
        info.headerPath = j["Header"].get<std::string>();

        if (info.className.empty() || info.headerPath.empty())
            continue;

        scripts.push_back(std::move(info));
    }

    std::sort(scripts.begin(), scripts.end(),
        [](const ScriptInfo& a, const ScriptInfo& b) { return a.className < b.className; });

    /* NOTE : Script_Registry 생성하기 */
    std::ofstream ofs(outCppPath, std::ios_base::binary);
    IF_TRUE_RETURN_MSG_BREAK(!ofs.is_open(), E_FAIL, "Register script failed.");

    ofs <<
        "// AUTO-GENERATED. DO NOT EDIT.\n"
        "#include \"Script_Registry.h\"\n"
        "#include \"Asset_Registry.h\"\n"
        "#include \"Script_Handler.h\"\n"
        "#include \"Script_Register.h\"\n\n"; /* Script_Register.h 안의  template <typename TScript> static void Fill_VTable_Default(SCRIPT_VTABLE& vt)*/

    /* include 목록 */
    for (const auto& s : scripts)
    {
        const std::string escapedHeader = Escape_Cpp_String(s.headerPath);
        ofs << "#include \"" << escapedHeader << "\"\n";
    }

    ofs <<
        "\n"
        "NS_BEGIN(Client)"
        "\n"
        "void Register_AllScripts()\n"
        "{\n"
        "    auto& handler = SYS_ASSET.Scripts();\n";

    for (const auto& s : scripts)
    {
        const std::string escapedPath = Escape_Cpp_String(s.assetPath);

        ofs <<
            "    {\n"
            "        ASSET_GUID guid = SYS_ASSET.Ensure_GUID_For_Path(\"" << escapedPath << "\");\n"
            "        handler.Register_VTable(guid, ScriptBinder<" << s.className << ">::Build());\n"
            "    }\n";
    }

    ofs <<
        "}\n"
        "NS_END";

    return S_OK;
}

std::string CScript_Handler::To_Include_Line(const std::string& className)
{
    return "#include \"" + className + ".h\"";
}

std::string CScript_Handler::Escape_Cpp_String(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 16);
    for (char c : s)
    {
        if (c == '\\') out += "\\\\";
        else if (c == '"') out += "\\\"";
        else out += c;
    }
    return out;
}

std::unique_ptr<CScript_Handler> CScript_Handler::Create()
{
    return std::make_unique<CScript_Handler>();
}
