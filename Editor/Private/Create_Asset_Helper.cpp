#include "Create_Asset_Helper.h"

#include "Editor_Util.h"

NS_BEGIN(Editor)

void CCreate_Asset_Helper::Build_Script_Source(const std::string& className,
                                                       const std::filesystem::path& headerPath, std::string& outHeader, std::string& outCpp)
{
    outHeader.clear();
    outCpp.clear();

    outHeader += "#pragma once\n";
    outHeader += "#include \"Client_Define.h\"\n";
    outHeader += "#include \"Script.h\"\n\n";
    outHeader += "NS_BEGIN(Client)\n\n";
    outHeader += "class " + className + " : public IScript\n";
    outHeader += "{\n";
    outHeader += "public:\n";
    outHeader += "    void Awake(void* pCtx) override;\n";
    outHeader += "    void Start(void* pCtx) override;\n\n";
    outHeader += "    void Priority_Update(void* pCtx, _float fDT) override;\n";
    outHeader += "    void Update(void* pCtx, _float fDT) override;\n";
    outHeader += "    void Late_Update(void* pCtx, _float fDT) override;\n";
    outHeader += "};\n\n";
    outHeader += "NS_END;\n";

    outCpp += "#include \"" + Editor_Util::To_UTF8(headerPath.filename()) + "\"\n\n";
    outCpp += "NS_BEGIN(Client)\n\n";
    outCpp += "void " + className + "::Awake(void* pCtx)\n{\n}\n\n";
    outCpp += "void " + className + "::Start(void* pCtx)\n{\n}\n\n";
    outCpp += "void " + className + "::Priority_Update(void* pCtx, _float fDT)\n{\n}\n\n";
    outCpp += "void " + className + "::Update(void* pCtx, _float fDT)\n{\n}\n\n";
    outCpp += "void " + className + "::Late_Update(void* pCtx, _float fDT)\n{\n}\n\n";
    outCpp += "NS_END;\n";
}

_bool CCreate_Asset_Helper::Write_Script_Source_Files(const std::filesystem::path& headerPath,
    const std::filesystem::path& cppPath, const std::string& header, const std::string& cpp)
{
    if (!Write_Text_File(headerPath, header))
        return false;

    if (!Write_Text_File(cppPath, cpp))
    {
        std::error_code ec;
        std::filesystem::remove(headerPath, ec);
        return false;
    }

    return true;
}

_bool CCreate_Asset_Helper::Write_Script_Asset_File(const std::filesystem::path& savePath, const std::string& className,
    const std::filesystem::path& headerPath)
{
    json j;
    j["ClassName"] = className;
    j["Header"] = Editor_Util::To_UTF8(headerPath.filename());

    const std::string scriptJson = j.dump(2) + "\n";
    if (!Write_Text_File(savePath, scriptJson))
        return false;

    SYS_ASSET.Ensure_GUID_For_Path(savePath);
    return true;
}

std::string CCreate_Asset_Helper::Make_Script_File_From_Stem(const std::string& stem)
{
    std::string out;
    out.reserve(stem.size() + 16);

    _bool bPrevUnderscore = false;

    for (char c : stem)
    {
        if (std::isalnum((unsigned char)c))
        {
            out.push_back(c);
            bPrevUnderscore = false;
        }
        else
        {
            // 구분자는 '_' 하나로만
            if (!bPrevUnderscore && !out.empty())
            {
                out.push_back('_');
                bPrevUnderscore = true;
            }
        }
    }

    // 끝에 '_' 붙었으면 제거
    while (!out.empty() && out.back() == '_')
        out.pop_back();

    // 비어 있으면 fallback
    if (out.empty())
        out = "New_Script";

    return out;
}

std::string CCreate_Asset_Helper::Make_Unique_File_Stem_Impl(const std::filesystem::path& parent,
    const std::string& baseStem, const std::filesystem::path& hdPath, const std::filesystem::path& implPath)
{
    std::string stem = baseStem;

    auto exists_pair = [&](const std::string& s)
        {
            std::filesystem::path scriptPath = parent / (s + ".script");
            std::filesystem::path headerPath = hdPath / (s + ".h");
            std::filesystem::path cppPath = implPath / (s + ".cpp");

            return std::filesystem::exists(scriptPath) ||
                std::filesystem::exists(headerPath) ||
                std::filesystem::exists(cppPath);
        };

    if (!exists_pair(stem))
        return stem;

    for (int i = 1; i < 9999; ++i)
    {
        stem = baseStem + " (" + std::to_string(i) + ")";
        if (!exists_pair(stem))
            return stem;
    }

    return baseStem + " (9999)";
}

_bool CCreate_Asset_Helper::Write_Text_File(const std::filesystem::path& p, const std::string& utf8)
{
    std::error_code ec;
    std::filesystem::create_directories(p.parent_path(), ec);

    std::ofstream ofs(p, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    ofs.write(utf8.data(), (std::streamsize)utf8.size());
    return ofs.good();
}


NS_END
