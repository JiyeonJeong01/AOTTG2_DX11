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
    entry.pSRV = std::move(srv);

    const uint32_t handle = (uint32_t)m_Textures.size();
    m_Textures.push_back(std::move(entry));
    m_TextureGUIDMap[tGUID] = handle;

    return handle;
}
uint32_t CResource_System::Load_Material_Temp(const ASSET_GUID& materialGuidAsShaderGuid, uint16_t passIndex)
{
    const uint32_t hShader = Load_Shader(materialGuidAsShaderGuid);
    if (hShader == INVALID_HANDLE_UINT) return INVALID_HANDLE_UINT;

    MATERIAL_ENTRY e{};
    e.tGUID = materialGuidAsShaderGuid;
    e.hShader = hShader;
    e.passIndex = passIndex;

    return Load_Material(e); // 엔트리 버전
}

uint32_t CResource_System::Load_Mesh(const ASSET_GUID& tGUID)
{
    // 1. 이미 로드된 메시인지 확인
    auto it = m_MeshGUIDMap.find(tGUID);
    if (it != m_MeshGUIDMap.end())
        return it->second;

    MESH_ENTRY entry{};
    entry.tGUID = tGUID;
    HRESULT hr = S_OK;

    auto pRec = SYS_ASSET.Find(tGUID);
    if (!pRec || pRec->eType != ASSET_TYPE::MESH)
    {
        _DEBUG_ERROR_BREAK("such guid not exists");
        return INVALID_HANDLE_UINT;
    }


    if (pRec->eSrc == ASSET_SRC::BUILTIN)
    {
        if (tGUID == DEFAULT_ASSET_GUID::MESH_CUBE)
            hr = CMeshBuilder::Create_Cube_VtxCol(m_pDevice, entry);
        else if (tGUID == DEFAULT_ASSET_GUID::MESH_RECT)
            hr = CMeshBuilder::Create_Rect_VtxTex(m_pDevice, entry);
        else if (tGUID == DEFAULT_ASSET_GUID::MESH_SPHERE)
            hr = CMeshBuilder::Create_Sphere_VtxCol(m_pDevice, entry);
        else
            return INVALID_HANDLE_UINT;
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

    return handle;
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

    std::filesystem::path matPath = SYS_ASSET.Get_Asset_Path(tGUID);
    IF_TRUE_RETURN_MSG_BREAK(matPath.empty(), INVALID_HANDLE_UINT, "Load_Material failed: asset path empty.");

    /* .mat 파일에서 데이터를 읽어온다. */
    MATERIAL_ENTRY desc{};
    IF_FAIL_RETURN_MSG_BREAK(CMaterialBuilder::Load_MaterialDesc(matPath, desc), INVALID_HANDLE_UINT, "Load_Material failed: Load_MaterialDesc failed.");

    desc.hShader = Load_Shader(desc.shaderGUID);
    IF_TRUE_RETURN_MSG_BREAK(desc.hShader == INVALID_HANDLE_UINT, INVALID_HANDLE_UINT, "Load_Material failed: invalid shader handle.");

    /* 머테리얼의 기본 텍스쳐를 로드해온다. */
    desc.hBaseMap = Load_Texture(desc.baseMapGUID);
    IF_TRUE_RETURN_MSG_BREAK(desc.hBaseMap == INVALID_HANDLE_UINT, INVALID_HANDLE_UINT, "Load_Material failed: invalid base map handle.");

    /* param block에 반영한다. */
    desc.Sync_StandardParams();

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

    entry.pMainTex = pFx->GetVariableByName("g_BaseMap")->AsShaderResource();
    entry.pColor = pFx->GetVariableByName("g_BaseColor")->AsVector();

#ifdef _DEBUG
    IF_TRUE_RETURN_MSG_BREAK(!entry.pWorld || !entry.pWorld->IsValid(), INVALID_HANDLE_UINT, "Material matrix variable invalid: g_WorldMatrix");
    IF_TRUE_RETURN_MSG_BREAK(!entry.pView || !entry.pView->IsValid(), INVALID_HANDLE_UINT, "Material matrix variable invalid: g_ViewMatrix");
    IF_TRUE_RETURN_MSG_BREAK(!entry.pProj || !entry.pProj->IsValid(), INVALID_HANDLE_UINT, "Material matrix variable invalid: g_ProjMatrix");

    IF_TRUE_RETURN_MSG_BREAK(entry.pMainTex && !entry.pMainTex->IsValid(), INVALID_HANDLE_UINT, "Material var invalid: g_BaseMap");
#endif

    const uint32_t handle = (uint32_t)m_Materials.size();
    m_Materials.push_back(entry);
    return handle;
}

const MESH_ENTRY* CResource_System::Get_Mesh(uint32_t handle) const
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Meshes.size())
        return nullptr;

    return &m_Meshes[handle];
}

SHADER_ENTRY* CResource_System::Get_Shader(uint32_t handle)
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Shaders.size())
        return nullptr;

    return &m_Shaders[handle];
}

const TEXTURE_ENTRY* CResource_System::Get_Texture(uint32_t handle) const
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Textures.size())
        return nullptr;

    return &m_Textures[handle];
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
