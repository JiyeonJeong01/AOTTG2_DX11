#pragma once
#include "Asset_GUID.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)
    inline std::filesystem::path Make_MetaPath(const std::filesystem::path& assetPath)
{
    return assetPath.string() + ".meta";
}

inline _bool Is_MetaFile(const std::filesystem::path& path)
{
    return path.extension() == ".meta";
}

inline _bool Write_MetaFile(const std::filesystem::path& metaPath, const ASSET_GUID& tGuid, const char* szTypeStr = nullptr)
{
    std::ofstream ofs(metaPath, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    ofs << "guid=" << tGuid.To_String_Utf8() << "\n";

    /* For faster filtering and loading by asset type */
    if (szTypeStr && szTypeStr[0])
        ofs << "type=" << szTypeStr << "\n";

    return true;
}

inline _bool Read_MetaFileGuid(const std::filesystem::path& metaPath, ASSET_GUID& outGuid)
{
    std::ifstream ifs(metaPath, std::ios_base::binary);
    if (!ifs.is_open())
        return false;

    std::string line;
    while (std::getline(ifs, line))
    {
        /* Remove carriage return character (\r) for cross-platform compatibility */
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        /* Check if the line starts with "guid=" */
        constexpr const char* k = "guid=";
        if (line.rfind(k, 0) == 0)
        {
            /* Extract the GUID string part (after "guid=") */
            std::string guidStr = line.substr(5);

            /* Convert the string back into a GUID structure */
            return ASSET_GUID::Try_Utf8_To_GUID(guidStr, outGuid);
        }
    }
    return false;
}

static ASSET_GUID Ensure_Asset_Has_Meta(const std::filesystem::path& assetPath, const char* szTypeStr = nullptr)
{
    if (Is_MetaFile(assetPath))
    {
        ASSET_GUID g{};
        Read_MetaFileGuid(assetPath, g);
        return g;
    }

    const std::filesystem::path metaPath = Make_MetaPath(assetPath);

    /* Existing File : runs a validity check. */
    if (std::filesystem::exists(metaPath))
    {
        ASSET_GUID g{};
        if (!Read_MetaFileGuid(metaPath, g) || !g.Is_Valid())
        {
            /* Regenerate a new GUID and overwrite the corrupted meta file */
            g = ASSET_GUID::New_GUID();
            Write_MetaFile(metaPath, g, szTypeStr);
            _DEBUG_WARN("Meta existed but invalid. Rewrote GUID: %s", metaPath.string().c_str());
        }
        return g;
    }

    /* Missing File : performs a fresh creation. */
    ASSET_GUID g = ASSET_GUID::New_GUID();
    if (Write_MetaFile(metaPath, g, szTypeStr))
        _DEBUG_INFO("Created meta: %s", metaPath.string().c_str());
    else
        _DEBUG_WARN("Failed to create meta: %s", metaPath.string().c_str());

    return g;
}

NS_END
