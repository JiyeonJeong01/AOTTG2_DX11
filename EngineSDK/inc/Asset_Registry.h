#pragma once
#include "Base.h"
#include "Script.h"

NS_BEGIN(Engine)

class CPrototype_Handler;
class CScript_Handler;

/**
 * @class CAsset_Registry
 * @brief Manages a comprehensive database of all raw files (Textures, Meshes, JSON, etc.) within the project.
 * * This class handles ASSET_RECORD entries. During engine initialization, it scans the entire
 * project directory to establish a mapping between unique ASSET_GUIDs and their respective file paths.
 */
class ENGINE_DLL CAsset_Registry : public CBase
{
    DECLARE_SINGLETON(CAsset_Registry)

public :
    HRESULT Initialize(const std::filesystem::path& assetRoot);
    void    Clear();

    /* Scan all assets, ensure meta and rebuild maps */
    void    Rebuild();
    void    Distribute_Assets_To_Handlers();

    CPrototype_Handler& Prototypes()    { return *m_upPrototype_Handler; }
    CScript_Handler&    Scripts()       { return *m_upScript_Handler; }
public :
    const std::filesystem::path& Get_Root() const;
    _bool Make_MetaPath_By_GUID(const ASSET_GUID& tGUID, std::filesystem::path& outPath) const;

    /* path -> GUID */
    _bool   Try_Get_GUID(const std::filesystem::path& inPath, ASSET_GUID& outGUID) const;
    static std::filesystem::path Get_Asset_Path(const ASSET_GUID& tGUID);

    /* builtin -> GUID */
    void Register_Builtin_Asset();
    void Register_Builtin_Inner(const ASSET_GUID& tGUID, ASSET_TYPE eType);

    /* Register individual file asset GUID (and create .meta file) */
    _bool Register_File_Asset(const std::filesystem::path& rawPath, ASSET_TYPE forcedType, ASSET_GUID forcedGuid = ASSET_GUID{});

    ASSET_GUID Ensure_GUID_For_Path(const std::filesystem::path& path);

    /* GUID -> ASSET_RECORD */
    const ASSET_RECORD* Find(const ASSET_GUID& tGUID) const;

    /* path -> ASSET_TYPE */
    static const ASSET_TYPE Detect_Type(const std::filesystem::path& path, _bool bDir);
    static const _char* AssetType_ToStr(ASSET_TYPE eType);

    static std::filesystem::path Normalize_Path(const std::filesystem::path& p);

private :
    std::unique_ptr<CPrototype_Handler> m_upPrototype_Handler{};
    std::unique_ptr<CScript_Handler>    m_upScript_Handler{};

    std::filesystem::path   m_assetRoot{};
    std::unordered_map<ASSET_GUID, ASSET_RECORD, ASSET_GUID_HASHER> m_byGUID;
    std::unordered_map<std::string, ASSET_GUID>                     m_byPathUtf8; /* canonical path string -> GUID */

};

NS_END

