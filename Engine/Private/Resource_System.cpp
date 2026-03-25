#include "Resource_System.h"
#include "Asset_Registry.h"
#include "BuiltIn_GUID.h"
#include "MeshBuilder.h"
#include "Render_Struct.h"
#include "Load_Helper.h"
#include "MaterialBuilder.h"

IMPLEMENT_SINGLETON(CResource_System)


CResource_System::~CResource_System()
{
}

CResource_System::CResource_System()
{
}

HRESULT CResource_System::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pDevice = pDevice;
    m_pContext = pContext;

    /* index 0 dummy */
    m_Meshes.emplace_back();
    m_Materials.emplace_back();
    m_Shaders.emplace_back();
    m_Textures.emplace_back();
    m_Models.emplace_back();
    m_Fonts.emplace_back();

    return S_OK;
}

void CResource_System::Clear()
{

}

uint32_t CResource_System::Load_Texture(const ASSET_GUID& tGUID)
{
    auto it = m_TextureGUIDMap.find(tGUID);
    if (it != m_TextureGUIDMap.end())
        return it->second;

    auto* pAsset = SYS_ASSET.Find(tGUID);
    IF_NULL_RETURN_MSG_BREAK(pAsset, INVALID_HANDLE_UINT, "Texture load failed: asset not found.");

    filesystem::path path = pAsset->path;
    wstring ext = path.extension().wstring();

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

    HRESULT hr = {};
    if (ext == L".dds")
        hr = CreateDDSTextureFromFile(m_pDevice, path.c_str(), nullptr, srv.GetAddressOf());
    else if (ext == L".tga")
        hr = E_FAIL;
    else
        hr = CreateWICTextureFromFile(m_pDevice, path.c_str(), nullptr, srv.GetAddressOf());
    IF_FAIL_RETURN_MSG_BREAK(hr, INVALID_HANDLE_UINT, "Texture create failed");

    TEXTURE_ENTRY entry{};
    entry.tGUID = tGUID;
    entry.pSRV = std::move(srv);

    const uint32_t handle = (uint32_t)m_Textures.size();
    m_Textures.push_back(std::move(entry));
    m_TextureGUIDMap[tGUID] = handle;

    return handle;
}

uint32_t CResource_System::Load_Font(const ASSET_GUID& tGUID)
{
    auto it = m_FontGUIDMap.find(tGUID);
    if (it != m_FontGUIDMap.end())
        return it->second;

    auto pRec = SYS_ASSET.Find(tGUID);
    if (!pRec || pRec->eType != ASSET_TYPE::FONT)
    {
        _DEBUG_ERROR_BREAK("such guid not exists");
        return INVALID_HANDLE_UINT;
    }

    FONT_ENTRY entry{};
    entry.tGUID = tGUID;

    try
    {
#ifdef UNICODE
        std::wstring wPath = pRec->path.wstring();
        entry.pFont = std::make_unique<DirectX::SpriteFont>(m_pDevice, wPath.c_str());
#else
        entry.pFont = std::make_unique<DirectX::SpriteFont>(m_pDevice, pRec->path.c_str());
#endif
    }
    catch (...)
    {
        _DEBUG_ERROR_BREAK("Load font failed");
        return INVALID_HANDLE_UINT;
    }

    IF_TRUE_RETURN_MSG_BREAK(entry.Is_Valid() == false, INVALID_HANDLE_UINT, "Invalid font entry");

    uint32_t handle = static_cast<uint32_t>(m_Fonts.size());
    m_Fonts.push_back(std::move(entry));
    m_FontGUIDMap[tGUID] = handle;

    return handle;
}

FONT_ENTRY* CResource_System::Get_Font(uint32_t handle)
{
    const uint32_t iIndex = Handle_Index(handle);
    if (handle == INVALID_HANDLE_UINT || iIndex >= m_Fonts.size())
        return nullptr;

    return &m_Fonts[iIndex];
}

uint32_t CResource_System::Register_MeshEntry(MESH_ENTRY&& pEntry)
{
    if (!pEntry.Is_Valid())
        return INVALID_HANDLE_UINT;

    const uint32_t handle = (uint32_t)m_Meshes.size();
    m_Meshes.push_back(std::move(pEntry));

    return handle;
}

uint32_t CResource_System::Load_Mesh(const ASSET_GUID& tGUID)
{
    /* 이미 로드된 모델인지 확인 */
    auto it = m_MeshGUIDMap.find(tGUID);
    if (it != m_MeshGUIDMap.end())
        return it->second;

    MESH_ENTRY entry{};
    entry.tGUID = tGUID;
    HRESULT hr = S_OK;

    auto pRec = SYS_ASSET.Find(tGUID);
    if (!pRec || pRec->eType != ASSET_TYPE::MESH)
    {
        if (pRec->eType == ASSET_TYPE::MODEL) /* .model이 .mesh 경로로 들어오는 경우 LoadModel로 다시 보내준다. */
            return Load_Model(tGUID);
        _DEBUG_ERROR_BREAK("such guid not exists");
        return INVALID_HANDLE_UINT;
    }

    /* Built-in 처리 */
    if (pRec->eSrc == ASSET_SRC::BUILTIN)
    {
        IF_FAIL_RETURN_MSG_BREAK(CMeshBuilder::Create_Builtin(m_pDevice, entry, tGUID), INVALID_HANDLE_UINT,
            "Create built-in mesh failed");
    }
    else
    {
        _bool bLoaded = Load_Mesh_Header(m_pDevice, pRec->path, entry);
        IF_TRUE_RETURN_MSG_BREAK(bLoaded == false, INVALID_HANDLE_UINT, "Load mesh failed");
    }

    IF_FAIL_RETURN_MSG_BREAK(hr, INVALID_HANDLE_UINT, "Failed to Create/Load Mesh");

    uint32_t handle = (uint32_t)m_Meshes.size();
    m_Meshes.push_back(std::move(entry));
    m_MeshGUIDMap[tGUID] = handle;

    return (handle & ASSET_TAG_MESH); /* MSB = 0 */
}

uint32_t CResource_System::Load_Model(const ASSET_GUID& tGUID)
{
    auto it = m_ModelGUIDMap.find(tGUID);
    if (it != m_ModelGUIDMap.end())
        return (it->second | ASSET_TAG_MODEL);

    auto pRec = SYS_ASSET.Find(tGUID);

    /* anim / non-anim 에 따른 분기 처리 */
    const MODEL_TYPE eModelType = CMeshBuilder::Peek_ModelType(pRec->path);

    MODEL_ENTRY model{};
    model.tGUID = tGUID;

    if (eModelType == MODEL_TYPE::NONANIM)
    {
        /* .model 파일로부터 해당 모델이 소유한 메쉬 GUID를 읽어온다. */
        MODEL_DESC desc{};
        IF_FAIL_RETURN_MSG_BREAK(CMeshBuilder::Load_NonAnim_ModelDesc(pRec->path, desc), E_FAIL, "Load_Model failed");

        model.parts.reserve(desc.parts.size());

        /* 모델이 소유한 파트(=메쉬) 정보 채우기 */
        for (const auto& partDesc : desc.parts)
        {
            const uint32_t hMesh = Load_Mesh(partDesc.tMeshGUID);
            if (hMesh == INVALID_HANDLE_UINT)
                continue;

            MODEL_PART part{};
            part.hMesh = hMesh;
            part.materialGUID = partDesc.tMaterialGUID;

            if (!part.materialGUID.Is_Valid())
                part.hMaterial = INVALID_HANDLE_UINT; /* 또는 default material로 설정 고려 */
            else
                part.hMaterial = Load_Material(part.materialGUID);

            model.parts.push_back(part);
        }
    }
    else /* --- Anim --- */
    {
        ANIM_MODEL_DESC desc{};
        IF_FAIL_RETURN_MSG_BREAK(
            CMeshBuilder::Load_Anim_ModelDesc(pRec->path, desc),
            INVALID_HANDLE_UINT,
            "Load_Model failed : Load_Anim_ModelDesc failed");

        model.tSkeleton = desc.tSkeleton;
        model.vecAnimClips = desc.vecAnimClips;

        model.parts.reserve(desc.parts.size());

        for (const auto& partDesc : desc.parts)
        {
            const uint32_t hMesh = Load_Mesh(partDesc.tMeshGUID);
            if (hMesh == INVALID_HANDLE_UINT)
                continue;

            MODEL_PART part{};
            part.hMesh = hMesh;
            part.materialGUID = partDesc.tMaterialGUID;

            if (!part.materialGUID.Is_Valid())
                part.hMaterial = INVALID_HANDLE_UINT;
            else
                part.hMaterial = Load_Material(part.materialGUID);

            model.parts.push_back(std::move(part));
        }
    }

    IF_TRUE_RETURN_MSG_BREAK(model.parts.empty(), INVALID_HANDLE_UINT, "Load_Model failed: no valid parts");   

    const uint32_t handle = (uint32_t)m_Models.size();
    m_Models.push_back(std::move(model));
    m_ModelGUIDMap[tGUID] = handle;

    return (handle | ASSET_TAG_MODEL);
}

uint32_t CResource_System::Load_Shader(const ASSET_GUID& tGUID)
{
    auto it = m_ShaderGUIDMap.find(tGUID);
    if (it != m_ShaderGUIDMap.end())
        return it->second;

    std::filesystem::path shaderPath = SYS_ASSET.Get_Asset_Path(tGUID);
    IF_TRUE_RETURN_MSG_BREAK(shaderPath.empty(), INVALID_HANDLE_UINT, "Shader Load Failed: GUID not found in Registry.");

    _uint			iHlslFlag = {};
#ifdef _DEBUG
    iHlslFlag |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
    iHlslFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;

#endif	
    SHADER_ENTRY entry{};
    entry.tGUID = tGUID;

    /* .meta 파일에 정의된 decl을 셰이더 파일 당 한 번씩만 읽어온다. */
    std::filesystem::path metaPath;
    IF_TRUE_RETURN_MSG_BREAK(!SYS_ASSET.Make_MetaPath_By_GUID(tGUID, metaPath), INVALID_HANDLE_UINT,
        "Read meta file failed; set entry's decl as default.");
    uint32_t metaDecl = SCAST(uint32_t, VERTEX_DECL::END);

    if (Read_MetaFileDecl(metaPath, metaDecl))
        entry.eDecl = SCAST(VERTEX_DECL, metaDecl);

    if (metaDecl >= _countof(g_IL_TABLE))
        metaDecl = SCAST(uint32_t, VERTEX_DECL::VTXTEX);

    ID3DBlob* pBlob = nullptr;
    HRESULT hr = D3DX11CompileEffectFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0, m_pDevice, entry.pEffect.GetAddressOf(), &pBlob);
    if (FAILED(hr))
    {
        OutputDebugStringA((char*)pBlob->GetBufferPointer());
        _DEBUG_ERROR_BREAK("Create Effect file failed");
        return INVALID_HANDLE_UINT;
    }

    /* NOTE : Tech는 한 개인 경우만 고려한다. */
    entry.pTech = entry.pEffect->GetTechniqueByIndex(0);
    if (!entry.pTech || !entry.pTech->IsValid())
        return INVALID_HANDLE_UINT;

    D3DX11_TECHNIQUE_DESC TechniqueDesc{};
    entry.pTech->GetDesc(&TechniqueDesc);

    for(uint32_t i = 0; i < TechniqueDesc.Passes; ++i)
    {
        SHADER_ENTRY::PASS_CACHE cache{};
        cache.pPass = entry.pTech->GetPassByIndex(i);
        if (!cache.pPass || !cache.pPass->IsValid())
            return INVALID_HANDLE_UINT;

        D3DX11_PASS_DESC PassDesc{};
        cache.pPass->GetDesc(&PassDesc);
        {
            _DEBUG_INFO("PassDesc: SigPtr=%p SigSize=%u", PassDesc.pIAInputSignature, (uint32_t)PassDesc.IAInputSignatureSize);

            if (!PassDesc.pIAInputSignature || PassDesc.IAInputSignatureSize == 0)
            {
                _DEBUG_ERROR("No IA signature in pass. Shader=%ls Pass=%u", shaderPath.c_str(), i);
                return INVALID_HANDLE_UINT; // 또는 continue;
            }
        }

        HRESULT hr = m_pDevice->CreateInputLayout(g_IL_TABLE[SCAST(_uint, entry.eDecl)].pDesc, g_IL_TABLE[SCAST(_uint, entry.eDecl)].iCount, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, cache.pInputLayout.GetAddressOf());
        IF_FAIL_RETURN_MSG_BREAK(hr, INVALID_HANDLE_UINT, "Create InputLayout failed! Shader: %ls, Pass Index: %d", shaderPath.c_str(), i);

        entry.pPasses.push_back(cache);
    }

    uint32_t handle = (uint32_t)m_Shaders.size();
    m_Shaders.push_back(std::move(entry));
    m_ShaderGUIDMap[tGUID] = handle;

    return handle;
}

/* GUID -> .mat 에서 읽어온 데이터로 MATERIAL_ENTRY를 생성한다. */
/* 생성한 MATERIAL_ENTRY로 런타임 리소스를 할당받는다. */
uint32_t CResource_System::Load_Material(const ASSET_GUID& tGUID)
{
    auto it = m_MaterialGUIDMap.find(tGUID);
    if (it != m_MaterialGUIDMap.end())
        return it->second;

    auto pRec = SYS_ASSET.Find(tGUID);
    if (!pRec || pRec->eType != ASSET_TYPE::MATERIAL)
    {
        _DEBUG_ERROR_BREAK("such guid not exists");
        return INVALID_HANDLE_UINT;
    }

    HRESULT hr{};
    MATERIAL_ENTRY desc{};
    if (pRec->eSrc == ASSET_SRC::BUILTIN)
    {
        if (tGUID == DEFAULT_ASSET_GUID::MATERIAL_UI_DEFAULT)
            hr = CMaterialBuilder::Load_Default_UI(desc);
        else if (tGUID == DEFAULT_ASSET_GUID::MATERIAL_VTXTEX)
            hr = CMaterialBuilder::Load_Default_VTXTEX(desc);
        else
            return INVALID_HANDLE_UINT;
    }
    else
    {
        std::filesystem::path matPath = SYS_ASSET.Get_Asset_Path(tGUID);
        IF_TRUE_RETURN_MSG_BREAK(matPath.empty(), INVALID_HANDLE_UINT, "Load_Material failed: asset path empty.");

        /* .mat 파일에서 데이터를 읽어온다. */
        IF_FAIL_RETURN_MSG_BREAK(CMaterialBuilder::Load_MaterialDesc(matPath, desc), INVALID_HANDLE_UINT, "Load_Material failed: Load_MaterialDesc failed.");
    }

    desc.hShader = Load_Shader(desc.shaderGUID);
    IF_TRUE_RETURN_MSG_BREAK(desc.hShader == INVALID_HANDLE_UINT, INVALID_HANDLE_UINT, "Load_Material failed: invalid shader handle.");


    /* 머테리얼의 기본 텍스쳐를 로드해온다. */
    desc.hBaseMap = Load_Texture(desc.baseMapGUID);
    IF_TRUE_RETURN_MSG_BREAK(desc.hBaseMap == INVALID_HANDLE_UINT, INVALID_HANDLE_UINT, "Load_Material failed: invalid base map handle.");

  

    /* Effect 변수 포인터 캐싱 포함한 런타임 머테리얼 엔트리를 생성한다. */
    const uint32_t hMaterial = Load_Material(desc); // 기존 Load_Material(const MATERIAL_ENTRY&) 사용
    IF_TRUE_RETURN_MSG_BREAK(hMaterial == INVALID_HANDLE_UINT, INVALID_HANDLE_UINT, "Load_Material failed: Load_Material(desc) failed.");

    m_MaterialGUIDMap.emplace(tGUID, hMaterial);
    return hMaterial;
}

uint32_t CResource_System::Load_Material(const MATERIAL_ENTRY& tDesc)
{
    IF_TRUE_RETURN_MSG_BREAK((tDesc.hShader == INVALID_HANDLE_UINT), INVALID_HANDLE_UINT, "Load_Material failed: invalid shader handle(0).");

    const SHADER_ENTRY* pShader = Get_Shader(tDesc.hShader);
    {
        IF_NULL_RETURN_MSG_BREAK(pShader, INVALID_HANDLE_UINT, "Load_Material failed: pshader is nullptr");
        IF_TRUE_RETURN_MSG_BREAK(!pShader->Is_Valid(), INVALID_HANDLE_UINT, "Load_Material failed: pshader is invalid.");
        IF_TRUE_RETURN_MSG_BREAK(tDesc.passIndex >= pShader->pPasses.size(), INVALID_HANDLE_UINT, "Load_Material failed: passIndex out of range.");
    }

    ID3DX11Effect* pFx = pShader->pEffect.Get();
    IF_NULL_RETURN_MSG_BREAK(pFx, INVALID_HANDLE_UINT, "Load_Material failed: pFX is nullptr");

    MATERIAL_ENTRY entry = tDesc;
    entry.pWorld = pFx->GetVariableByName("g_WorldMatrix")->AsMatrix();
    entry.pView = pFx->GetVariableByName("g_ViewMatrix")->AsMatrix();
    entry.pProj = pFx->GetVariableByName("g_ProjMatrix")->AsMatrix();

    entry.pBaseMap = pFx->GetVariableByName("g_BaseMap")->AsShaderResource();
    entry.pBaseColor = pFx->GetVariableByName("g_BaseColor")->AsVector();

    entry.pBoneMatrices = pFx->GetVariableByName("g_BoneMatrices")->AsMatrix();

#ifdef _DEBUG
    IF_TRUE_RETURN_MSG_BREAK(!entry.pWorld || !entry.pWorld->IsValid(), INVALID_HANDLE_UINT, "Material matrix variable invalid: g_WorldMatrix");
    IF_TRUE_RETURN_MSG_BREAK(!entry.pView || !entry.pView->IsValid(), INVALID_HANDLE_UINT, "Material matrix variable invalid: g_ViewMatrix");
    IF_TRUE_RETURN_MSG_BREAK(!entry.pProj || !entry.pProj->IsValid(), INVALID_HANDLE_UINT, "Material matrix variable invalid: g_ProjMatrix");

    IF_TRUE_RETURN_MSG_BREAK(entry.pBaseMap && !entry.pBaseMap->IsValid(), INVALID_HANDLE_UINT, "Material var invalid: g_BaseMap");
#endif

    const uint32_t handle = (uint32_t)m_Materials.size();
    m_Materials.push_back(entry);
    return handle;
}

 MESH_ENTRY* CResource_System::Get_Mesh(uint32_t handle) 
{
    const uint32_t iIndex = Handle_Index(handle);
    if (handle == INVALID_HANDLE_UINT || iIndex >= m_Meshes.size())
        return nullptr;

    return &m_Meshes[iIndex];
}

MODEL_ENTRY* CResource_System::Get_Model(uint32_t handle) 
{
    const uint32_t iIndex = Handle_Index(handle);
    if (handle == INVALID_HANDLE_UINT || iIndex >= m_Models.size())
        return nullptr;

    return &m_Models[iIndex];
}

SHADER_ENTRY* CResource_System::Get_Shader(uint32_t handle)
{

    if (handle == INVALID_HANDLE_UINT || handle >= m_Shaders.size())
        return nullptr;

    return &m_Shaders[handle];
}

TEXTURE_ENTRY* CResource_System::Get_Texture(uint32_t handle)
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Textures.size())
        return nullptr;

    return &m_Textures[handle];
}

const ASSET_GUID& CResource_System::Find_GUID_By_Handle(ASSET_TYPE eType, _uint iHandle)
{
    static ASSET_GUID s_tInvalid{};

    switch (eType)
    {
    case ASSET_TYPE::MESH:
    {
        MESH_ENTRY* pEntry = Get_Mesh(iHandle);
        return pEntry ? pEntry->tGUID : s_tInvalid;
    }

    case ASSET_TYPE::MODEL:
    {
        MODEL_ENTRY* pEntry = Get_Model(iHandle);
        return pEntry ? pEntry->tGUID : s_tInvalid;
    }

    case ASSET_TYPE::MATERIAL:
    {
        MATERIAL_ENTRY* pEntry = Get_Material(iHandle);
        return pEntry ? pEntry->tGUID : s_tInvalid;
    }

    case ASSET_TYPE::SHADER:
    {
        SHADER_ENTRY* pEntry = Get_Shader(iHandle);
        return pEntry ? pEntry->tGUID : s_tInvalid;
    }

    case ASSET_TYPE::TEXTURE:
    {
        TEXTURE_ENTRY* pEntry = Get_Texture(iHandle);
        return pEntry ? pEntry->tGUID : s_tInvalid;
    }

    default:
        return s_tInvalid;
    }
}

const std::string& CResource_System::Find_Name_By_GUID(const ASSET_GUID& tGUID)
{
    static const std::string s_strInvalid = "<invalid>";

    auto* pAsset = SYS_ASSET.Find(tGUID);
    if (!pAsset)
        return s_strInvalid;

    return pAsset->path.stem().string();
}

uint32_t CResource_System::Alloc_PerObjectParamBlock()
{
    return m_PerObjectParamPool.Alloc();
}

void CResource_System::Free_PerObjectParamBlock(uint32_t handle)
{
    m_PerObjectParamPool.Free(handle);
}

PER_OBJECT_PARAM_BLOCK* CResource_System::Get_PerObjectParamBlock(uint32_t handle)
{
    return m_PerObjectParamPool.Get(handle);
}

MATERIAL_ENTRY* CResource_System::Get_Material(uint32_t handle)
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Materials.size())
        return nullptr;

    return &m_Materials[handle];
}

/* 셰이더의 .meta 파일의 decl=을 읽어온다. */
_bool CResource_System::Read_MetaFileDecl(const std::filesystem::path& metaPath, uint32_t& outDecl)
{
    std::ifstream ifs(metaPath, std::ios_base::binary);
    if (!ifs.is_open())
        return false;

    std::string line;

    constexpr const char* k = "decl=";
    while (std::getline(ifs, line))
    {
        if (line.rfind(k, 0) == 0)
        {
            const std::string v = line.substr(5);
            try
            {
                outDecl = SCAST(uint32_t, std::stoul(v));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
    }

    return false;
}
