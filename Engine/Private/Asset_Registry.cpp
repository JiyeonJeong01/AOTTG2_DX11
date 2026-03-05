#include "Asset_Registry.h"

#include "Prototype_Handler.h"
#include "Script_Handler.h"
#include "Asset_Meta.h"
#include "BuiltIn_GUID.h"
#include "Engine_Log.h"
#include "magic_enum.hpp"
#include "String_Utils.h"
#include "Resource_System.h"
#include "Core_System.h"

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

    m_upPrototype_Handler = CPrototype_Handler::Create();
    IF_NULL_RETURN_MSG_BREAK(m_upPrototype_Handler, E_FAIL, "Prototype_Handler is nullptr");

    m_upScript_Handler = CScript_Handler::Create();
    IF_NULL_RETURN_MSG_BREAK(m_upScript_Handler, E_FAIL, "ScriptType_Handler is nullptr");

    Clear();

    /* Built-in 에셋 */
    Register_Builtin_Asset();

    /* File 에셋 */
    Rebuild();

    Distribute_Assets_To_Handlers();

    return S_OK;
}

void CAsset_Registry::Clear()
{
    m_byGUID.clear();
    m_byPathUtf8.clear();

    if (m_upScript_Handler)
        m_upScript_Handler->Clear();
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

void CAsset_Registry::Distribute_Assets_To_Handlers()
{
    /* 타입별 배치(스택) */
    std::vector<ASSET_GUID> vShader;
    std::vector<ASSET_GUID> vTexture;
    std::vector<ASSET_GUID> vMaterial;
    std::vector<ASSET_GUID> vMesh;
    std::vector<ASSET_GUID> vModel;

    std::vector<ASSET_GUID> vPrototype;
    std::vector<std::pair<ASSET_GUID, std::filesystem::path>> vScene;
    std::vector<ASSET_GUID> vScript;

    vShader.reserve(m_byGUID.size());
    vTexture.reserve(m_byGUID.size());
    vMaterial.reserve(m_byGUID.size());
    vMesh.reserve(m_byGUID.size());
    vModel.reserve(m_byGUID.size());

    vPrototype.reserve(m_byGUID.size());
    vScene.reserve(m_byGUID.size());
    vScript.reserve(m_byGUID.size());

    /* GUID->Record 스캔하면서 타입별로 모으기 */
    for (const auto& tGUID : m_byGUID)
    {
        const auto pRecord = tGUID.second;

        switch (pRecord.eType)
        {
        case ASSET_TYPE::PROTOTYPE:
            vPrototype.emplace_back(tGUID.first);
            break;

        case ASSET_TYPE::SCENE:
            vScene.emplace_back(tGUID.first, pRecord.path);
            break;

        case ASSET_TYPE::SCRIPT:
            vScript.emplace_back(tGUID.first);
            break;

        case ASSET_TYPE::SHADER:
            vShader.emplace_back(tGUID.first);
            break;

        case ASSET_TYPE::TEXTURE:
            vTexture.emplace_back(tGUID.first);
            break;

        case ASSET_TYPE::MATERIAL:
            vMaterial.emplace_back(tGUID.first);
            break;

        case ASSET_TYPE::MESH:
            vMesh.emplace_back(tGUID.first);
            break;

        case ASSET_TYPE::MODEL:
            vModel.emplace_back(tGUID.first);
            break;

        default:
            break;
        }
    }


   /* Resource: 의존성 고려해서 Shader/Texture -> Material -> Mesh */
    for (const auto& tGUID : vShader)
        SYS_RESOURCE.Load_Shader(tGUID);

    for (const auto& tGUID : vTexture)
        SYS_RESOURCE.Load_Texture(tGUID);

    for (const auto& tGUID : vMaterial)
        SYS_RESOURCE.Load_Material(tGUID);

    for (const auto& tGUID : vMesh)
        SYS_RESOURCE.Load_Mesh(tGUID);

    for (const auto& tGUID : vModel)
        SYS_RESOURCE.Load_Model(tGUID);

    /* Scene: GUID->Path 등록 */
    for (const auto& it : vScene)
        SYS_CORE.Register_Scenes(it.first, it.second);

    /* Script: GUID 캐시 */
    for (const auto& tGUID : vScript)
        m_upScript_Handler->Cache_GUID(tGUID);

    /* Prototype: GUID 기반 로드 */
    for (const auto& tGUID : vPrototype)
        m_upPrototype_Handler->Load_Prototype_From_GUID(tGUID);
}

const std::filesystem::path& CAsset_Registry::Get_Root() const
{
    return m_assetRoot;
}

_bool CAsset_Registry::Make_MetaPath_By_GUID(const ASSET_GUID& tGUID, std::filesystem::path& outPath) const
{
    static const std::filesystem::path s_empty{};

    auto it = m_byGUID.find(tGUID);
    if (it == m_byGUID.end())
        return false;

    const ASSET_RECORD& rec = it->second;
    if (rec.path.empty())
        return false;

    /* 모든 메타는 "<원본파일확장자>.meta" */
    /* 예: "MyShader.fx" -> "MyShader.fx.meta" */
    outPath = rec.path;
    outPath += ".meta";

    return true;
}

_bool CAsset_Registry::Try_Get_GUID(const std::filesystem::path& inPath, ASSET_GUID& outGUID) const
{
    outGUID = ASSET_GUID{};

    const auto normPath = Normalize_Path(inPath);
    const std::string strKeyPath = StringUtils::Path_To_UTF8(normPath);;

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

//     enum class ASSET_TYPE : uint8_t { UNKNOWN = 0, FOLDER, TEXTURE, MESH, MODEL, MATERIAL_VTXTEX, SCENE, PREFAB, PROTOTYPE, SCRIPT, };
const ASSET_TYPE CAsset_Registry::Detect_Type(const std::filesystem::path& path, _bool bDir)
{
    if (bDir)
        return ASSET_TYPE::FOLDER;

    const auto szExt = path.extension().string();


    if (szExt == ".png" || szExt == ".dds" || szExt == ".jpg" || szExt == ".jpeg")
        return ASSET_TYPE::TEXTURE;

    if (szExt == ".mesh")
        return ASSET_TYPE::MESH;

    if (szExt == ".model")
        return ASSET_TYPE::MODEL;

    if (szExt == ".mat")
        return ASSET_TYPE::MATERIAL;

    if (szExt == ".scene")
        return ASSET_TYPE::SCENE;

    if (szExt == ".proto")
        return ASSET_TYPE::PROTOTYPE;

    if (szExt == ".script")
        return ASSET_TYPE::SCRIPT;

    if (szExt == ".hlsl")
        return ASSET_TYPE::SHADER;

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
    //Register_Builtin_Inner(DEFAULT_ASSET_GUID::MATERIAL_VTXTEX, ASSET_TYPE::MATERIAL_VTXTEX);
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

/* CASE.1 : 런타임에 생성되어 레지스트리에 등록되지 않은 경우 (e.g., 에디터 실행 중에 추가) */
/* CASE.2 : 에셋 파일은 존재하지만 .meta가 사라졌거나 GUID와의 매핑이 깨진 경우 */
_bool CAsset_Registry::Register_File_Asset(const std::filesystem::path& rawPath, ASSET_TYPE forcedType, ASSET_GUID forcedGuid)
{
    if (rawPath.empty() || Is_MetaFile(rawPath))
        return false;

    std::error_code ec;
    const auto normPath = Normalize_Path(rawPath);
    const _bool bDir = std::filesystem::is_directory(normPath, ec);

    ASSET_TYPE eType = (forcedType != ASSET_TYPE::UNKNOWN) ? forcedType : Detect_Type(normPath, bDir);
    const char* szType = AssetType_ToStr(eType);

    /* 메타파일 확인 및 기본 GUID 보장 */
    ASSET_GUID tGUID = Ensure_Asset_Has_Meta(normPath, szType);
    if (!tGUID.Is_Valid())
        return false;

    auto itGUID = m_byGUID.end();

    /* GUID를 강제하는 경우 (e.g., Save As... ) */
    if (forcedGuid.Is_Valid() && forcedGuid != tGUID)
    {
        itGUID = m_byGUID.find(forcedGuid);

        if (itGUID != m_byGUID.end() && itGUID->second.path != normPath) /* 이미 사용 중인 GUID or 경로 */
            return false;

        Write_MetaFile(Make_MetaPath(normPath), forcedGuid, szType); /* forceGUID로 교체 */
        tGUID = forcedGuid;
    }

    /* Path <-> GUID 맵핑 갱신 */ 
    const std::string key = StringUtils::Path_To_UTF8(normPath);
    m_byPathUtf8[key] = tGUID;
    if (itGUID == m_byGUID.end())
        itGUID = m_byGUID.find(tGUID);

    if (itGUID != m_byGUID.end()) /* 해당 GUID로 이미 ASSET_RECORD 등록한 상태 */
    {
        /* GUID 충돌: 다른 경로가 동일 GUID를 사용 중이면 실패 처리 */
        IF_TRUE_RETURN_MSG_BREAK(itGUID->second.path != normPath, false, "Register_File_Asset failed: GUID collision");
        
        /* 기존 레코드 정보 갱신 */
        itGUID->second.eType = eType;
        itGUID->second.eSrc = ASSET_SRC::FILE;
        itGUID->second.path = normPath;
        itGUID->second.bDirectory = bDir;
    }
    else
    {
        /* 신규 레코드 등록 */
        ASSET_RECORD rec{ tGUID, eType, ASSET_SRC::FILE, normPath, bDir };
        m_byGUID.emplace(tGUID, std::move(rec));
    }

    return true;
}

/* Client::Register_AllScripts()에서 각 클래스마다 호출한다. */
ASSET_GUID CAsset_Registry::Ensure_GUID_For_Path(const std::filesystem::path& path)
{
    std::filesystem::path abs = path;
    if (!abs.is_absolute())
        abs = m_assetRoot / path;

    abs = Normalize_Path(abs);

    /* 없으면 빈 파일 생성 (.script 용) */ 
    if (!std::filesystem::exists(abs))
    {
        std::filesystem::create_directories(abs.parent_path());

        FILE* fp = nullptr;
#if defined(_WIN32)
        fopen_s(&fp, abs.string().c_str(), "wb");
#else
        fp = fopen(abs.string().c_str(), "wb");
#endif
        if (fp) fclose(fp);
    }

    /* 실제 디렉토리 여부로 타입 판정 */
    std::error_code ec;
    const _bool bDir = std::filesystem::is_directory(abs, ec) ? true : false;
    const ASSET_TYPE type = Detect_Type(abs, bDir);

    Register_File_Asset(abs, type, ASSET_GUID{});

    ASSET_GUID out{};
    (void)Try_Get_GUID(abs, out);
    return out;
}

NS_END

