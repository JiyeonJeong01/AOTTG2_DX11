#pragma once
#include "Base.h"

#include "Asset_Meta.h"

NS_BEGIN(Engine)

class ENGINE_DLL CAsset_Registry : public CBase
{
    DECLARE_SINGLETON(CAsset_Registry)

private :
    CAsset_Registry() = default;
    ~CAsset_Registry() override = default;

public :
    HRESULT Initialize(const std::filesystem::path& assetRoot);
    void    Clear();

    /* Scan all assets, ensure meta and rebuild maps */
    void    Rebuild();

public :
    const std::filesystem::path& Get_Root() const;

    /* path -> GUID */
    _bool   Try_Get_GUID(const std::filesystem::path& inPath, ASSET_GUID& outGUID) const;

    /* GUID -> ASSET_RECORD */
    const ASSET_RECORD* Find(const ASSET_GUID& tGUID) const;

    /* path -> ASSET_TYPE */
    static const ASSET_TYPE Detect_Type(const std::filesystem::path& path, _bool bDir);

private :
    static std::filesystem::path Normalize_Path(const std::filesystem::path& p);

private :
    std::filesystem::path   m_assetRoot{};
    std::unordered_map<ASSET_GUID, ASSET_RECORD, ASSET_GUID_HASHER> m_byGUID;
    std::unordered_map<std::string, ASSET_GUID>                     m_byPathUtf8; /* canonical path string -> GUID */

};

NS_END

