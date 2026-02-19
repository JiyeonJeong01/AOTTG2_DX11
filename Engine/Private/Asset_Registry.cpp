#include "Asset_Registry.h"

#include "Engine_Log.h"
#include "magic_enum.hpp"
#include "String_Utils.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CAsset_Registry)

CAsset_Registry::CAsset_Registry()
{
    
}

CAsset_Registry::~CAsset_Registry()
{
    
}


HRESULT CAsset_Registry::Initialize(const std::filesystem::path& assetRoot)
{
    m_assetRoot = Normalize_Path(assetRoot);
    IF_TRUE_RETURN_MSG_BREAK((m_assetRoot.empty() || !std::filesystem::exists(m_assetRoot)), E_FAIL, "AssetRegistry init failed: assetsRoot not found.");

    Clear();

    /* Built-in */
    Register_Builtin_Asset();

    /* File */
    Rebuild();
    return S_OK;
}

void CAsset_Registry::Clear()
{
    m_byGUID.clear();
    m_byPathUtf8.clear();
}

void CAsset_Registry::Rebuild()
{
    if (m_assetRoot.empty() || !std::filesystem::exists(m_assetRoot))
        return;

    /* Root folder meta */
    {
        const auto typeStr = AssetType_ToStr(ASSET_TYPE::FOLDER);
        (void)Ensure_Asset_Has_Meta(m_assetRoot, typeStr);
    }

    auto addOne = [this](const std::filesystem::path& rawPath)
        {
            if (Is_MetaFile(rawPath))
                return;

            std::error_code ec;
            const _bool bDir = std::filesystem::is_directory(rawPath, ec) ? true : false;

            const auto normPath = Normalize_Path(rawPath);
            const ASSET_TYPE eType = Detect_Type(normPath, bDir);
            const char* szType = AssetType_ToStr(eType);

            /* Ensure meta data : Scan the root folder and all sub-iems recursively to ensure every asset has a valid meta file */
            ASSET_GUID tGUID = Ensure_Asset_Has_Meta(normPath, szType);
            if (!tGUID.Is_Valid())
                return;
            const std::string strKeyPath = StringUtils::Path_To_UTF8(normPath);

            /* Duplicate Filtering : File paht and GUID */
            auto itPath = m_byPathUtf8.find(strKeyPath);
            if (itPath != m_byPathUtf8.end())
            {
                /* Already registered */
                return;
            }

            auto itGUID = m_byGUID.find(tGUID);
            if (itGUID != m_byGUID.end())
            {
                if (itGUID->second.path == normPath)
                {
                    return;
                }

                ASSET_GUID ng = ASSET_GUID::New_GUID();
                Write_MetaFile(Make_MetaPath(normPath), ng, szType);
                tGUID = ng;
            }

            ASSET_RECORD tRec{ tGUID, eType, ASSET_SRC::FILE, normPath, bDir };

            /* Registry Logging */
            m_byPathUtf8.emplace(strKeyPath, tGUID);
            m_byGUID.emplace(tGUID, std::move(tRec));
        };

    addOne(m_assetRoot);

    for (auto it = std::filesystem::recursive_directory_iterator(m_assetRoot); it != std::filesystem::recursive_directory_iterator(); ++it)
    {
        const auto& p = it->path();
        addOne(p);
    }

    _DEBUG_INFO("AssetRegistry rebuilt. count=%llu", (unsigned long long)m_byGUID.size());
}

const std::filesystem::path& CAsset_Registry::Get_Root() const
{
    return m_assetRoot;
}

_bool CAsset_Registry::Try_Get_GUID(const std::filesystem::path& inPath, ASSET_GUID& outGUID) const
{
    outGUID = ASSET_GUID{};

    const auto normPath = Normalize_Path(inPath);
    const std::string strKeyPath = normPath.string();

    auto it = m_byPathUtf8.find(strKeyPath);
    if (it == m_byPathUtf8.end())
        return false;
    outGUID = it->second;

    return outGUID.Is_Valid();
}

const ASSET_RECORD* CAsset_Registry::Find(const ASSET_GUID& tGUID) const
{
    auto it = m_byGUID.find(tGUID);
    if (it == m_byGUID.end())
        return nullptr;
    return &it->second;
}

//     enum class ASSET_TYPE : uint8_t { UNKNOWN = 0, FOLDER, TEXTURE, MESH, MODEL, MATERIAL, SCENE, PREFAB, PROTOTYPE, SCRIPT, };
const ASSET_TYPE CAsset_Registry::Detect_Type(const std::filesystem::path& path, _bool bDir)
{
    if (bDir)
        return ASSET_TYPE::FOLDER;

    const auto szExt = path.extension().string();


    if (szExt == ".png" || szExt == ".dds" || szExt == ".jpg" || szExt == ".jpeg")
        return ASSET_TYPE::TEXTURE;

    if (szExt == ".mesh")
        return ASSET_TYPE::MESH;

    if (szExt == ".fbx" || szExt == ".obj" || szExt == ".gltf" || szExt == ".glb")
        return ASSET_TYPE::MODEL;

    if (szExt == ".mat")
        return ASSET_TYPE::MATERIAL;

    if (szExt == ".scene")
        return ASSET_TYPE::SCENE;

    if (szExt == ".prefab")
        return ASSET_TYPE::PREFAB;

    if (szExt == ".proto")
        return ASSET_TYPE::PROTOTYPE;

    if (szExt == ".cpp" || szExt == ".h" || szExt == ".hlsl")
        return ASSET_TYPE::SCRIPT;

    return ASSET_TYPE::UNKNOWN;

}

const _char* CAsset_Registry::AssetType_ToStr(ASSET_TYPE eType)
{
    const std::string_view typeView = magic_enum::enum_name(eType);
    return typeView.data();
}

/* Normalize different path strings pointing to the same file into a single, standard format. */
std::filesystem::path CAsset_Registry::Normalize_Path(const std::filesystem::path& p)
{
    std::error_code ec;

    auto canon = std::filesystem::weakly_canonical(p, ec);

    if (ec)
        return p;

    return canon;
}


std::filesystem::path CAsset_Registry::Get_Asset_Path(const ASSET_GUID& tGUID)
{
    auto pRecord = SYS_ASSET.Find(tGUID);
    if (pRecord)
        return pRecord->path; // ASSET_RECORD에 저장된 물리 경로 반환

    return {};
}


void CAsset_Registry::Register_Builtin_Asset()
{
    Register_Builtin_Inner(DEFAULT_ASSET_GUID::MESH_RECT, ASSET_TYPE::MESH);
    Register_Builtin_Inner(DEFAULT_ASSET_GUID::MESH_CUBE, ASSET_TYPE::MESH);
    Register_Builtin_Inner(DEFAULT_ASSET_GUID::MESH_SPHERE, ASSET_TYPE::MESH);

    //Register_Builtin_Inner(DEFAULT_ASSET_GUID::SHADER_VTXCOL, ASSET_TYPE::SHADER);
    Register_Builtin_Inner(DEFAULT_ASSET_GUID::MATERIAL, ASSET_TYPE::MATERIAL);
}

void CAsset_Registry::Register_Builtin_Inner(const ASSET_GUID& tGUID, ASSET_TYPE eType)
{
    if (Find(tGUID))
        return;

    ASSET_RECORD rec{};
    rec.eSrc = eType == ASSET_TYPE::SHADER ? ASSET_SRC::FILE : ASSET_SRC::BUILTIN;
    rec.eType = eType;
    rec.tGUID = tGUID;
    rec.bDirectory = false;

    m_byGUID.emplace(tGUID, std::move(rec));
}

NS_END

