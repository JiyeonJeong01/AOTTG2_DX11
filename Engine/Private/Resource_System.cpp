#include "Resource_System.h"
#include "Asset_Registry.h"
#include "MeshBuilder.h"
#include "Render_Struct.h"

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
    m_SRVs.emplace_back();

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

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;

    /* TODO TODO 실제 로딩 로직 필요  */

    uint32_t handle = (uint32_t)m_SRVs.size();
    m_SRVs.push_back(pSRV);
    m_TextureGUIDMap[tGUID] = handle;

    return handle;
}

uint32_t CResource_System::Load_Material_Temp(const ASSET_GUID& materialGuidAsShaderGuid, uint16_t passIndex)
{
    const uint32_t hShader = Load_Shader(materialGuidAsShaderGuid);
    if (hShader == INVALID_HANDLE_UINT) return INVALID_HANDLE_UINT;

    MATERIAL_ENTRY e{};
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
    HRESULT hr = S_OK;

    auto pRec = SYS_ASSET.Find(tGUID);
    if (!pRec || pRec->eType != ASSET_TYPE::MESH)
        return INVALID_HANDLE_UINT;

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
        /* -------------------------------------------------- */
        /* [TODO] 나중에 진짜.mesh 파일을 읽는 로직이 들어갈 곳  */
        /* -------------------------------------------------- */
        return INVALID_HANDLE_UINT; // 지금은 파일 로드가 없으니 에러 리턴
    }

    if (FAILED(hr))
    {
        _DEBUG_ERROR_BREAK("Failed to Create/Load Mesh");
        return INVALID_HANDLE_UINT;
    }

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
    if (shaderPath.empty())
    {
        LOG_ERROR("Shader Load Failed: GUID not found in Registry.");
        return INVALID_HANDLE_UINT;
    }

    _uint			iHlslFlag = {};
#ifdef _DEBUG
    iHlslFlag |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
    iHlslFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;

#endif	
    SHADER_ENTRY entry{};
    if(FAILED(D3DX11CompileEffectFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0, m_pDevice, entry.pEffect.GetAddressOf(), nullptr)))
    {
        _DEBUG_ERROR_BREAK("Create Effect file failed");
        return INVALID_HANDLE_UINT;
    }

    /* NOTE : Tech는 한 개인 경우만 고려한다. */
    entry.pTech = entry.pEffect->GetTechniqueByIndex(0);
    if (!entry.pTech || !entry.pTech->IsValid())
        return INVALID_HANDLE_UINT;

    const uint32_t decl = SCAST(_uint, entry.eDecl);
    if (decl >= _countof(g_IL_TABLE))
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

        if (FAILED(m_pDevice->CreateInputLayout(g_IL_TABLE[SCAST(_uint, entry.eDecl)].pDesc, g_IL_TABLE[SCAST(_uint, entry.eDecl)].iCount, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, cache.pInputLayout.GetAddressOf())))
        {
            _DEBUG_ERROR_BREAK("Create InputLayout failed! Shader: %ls, Pass Index: %d", shaderPath.c_str(), i);
            return INVALID_HANDLE_UINT;
        }

        entry.pPasses.push_back(cache);
    }

    uint32_t handle = (uint32_t)m_Shaders.size();
    m_Shaders.push_back(std::move(entry));
    m_ShaderGUIDMap[tGUID] = handle;

    return handle;
}

uint32_t CResource_System::Load_Material(const ASSET_GUID& tGUID)
{
    auto it = m_MaterialGUIDMap.find(tGUID);
    if (it != m_MaterialGUIDMap.end())
        return it->second;

    MATERIAL_ENTRY desc{};
    /* ---------------------------------------------------------------------------- */
    /* TODO : 셰이더, 패스 추가 시 아래 로직 변경. 현재는 기본 셰이더, 0번 패스만 가져옴 */
    /* ---------------------------------------------------------------------------- */
    desc.hShader = Load_Shader(DEFAULT_ASSET_GUID::SHADER_VTXCOL);
    desc.passIndex = 0;

    if (desc.hShader == INVALID_HANDLE_UINT)
        return INVALID_HANDLE_UINT;

    uint32_t hMaterial = Load_Material(desc);
    m_MaterialGUIDMap[tGUID] = hMaterial;

    return hMaterial;
}

uint32_t CResource_System::Load_Material(const MATERIAL_ENTRY& tDesc)
{
    if (tDesc.hShader == INVALID_HANDLE_UINT)
    {
        _DEBUG_ERROR_BREAK("Load_Material failed: invalid shader handle(0).");
        return INVALID_HANDLE_UINT;
    }

    // 캐시 키: shader(32) + pass(16)
    const uint64_t key = (uint64_t(tDesc.hShader) << 16) | uint64_t(tDesc.passIndex);

    if (auto it = m_MaterialComboMap.find(key); it != m_MaterialComboMap.end())
        return it->second;

    const SHADER_ENTRY* pShader = Get_Shader(tDesc.hShader);
    if (!pShader || !pShader->Is_Valid())
    {
        _DEBUG_ERROR_BREAK("Load_Material failed: shader entry invalid.");
        return INVALID_HANDLE_UINT;
    }

    if (tDesc.passIndex >= pShader->pPasses.size())
    {
        _DEBUG_ERROR_BREAK("Load_Material failed: passIndex out of range.");
        return INVALID_HANDLE_UINT;
    }

    MATERIAL_ENTRY entry = tDesc;
    ID3DX11Effect* pFx = pShader->pEffect.Get();
    if (!pFx)
        return INVALID_HANDLE_UINT;

    auto FindMatVar = [&](std::initializer_list<const char*> names) -> ID3DX11EffectMatrixVariable*
        {
            for (auto* n : names)
            {
                if (!n) continue;
                auto* v = pFx->GetVariableByName(n);
                if (!v) continue;
                auto* m = v->AsMatrix();
                if (m && m->IsValid())
                    return m;
            }
            return nullptr;
        };

    entry.pWorld = pFx->GetVariableByName("g_WorldMatrix")->AsMatrix();
    entry.pView = pFx->GetVariableByName("g_ViewMatrix")->AsMatrix();
    entry.pProj = pFx->GetVariableByName("g_ProjMatrix")->AsMatrix();

#ifdef _DEBUG
    if (!entry.pWorld || !entry.pView || !entry.pProj)
        _DEBUG_ERROR_BREAK("Material matrix variables missing. Check fx variable names.");

    if (!entry.pWorld->IsValid() || !entry.pView->IsValid() || !entry.pProj->IsValid())
    {
        _DEBUG_ERROR_BREAK("Matrix variables not found in shader.");
        return INVALID_HANDLE_UINT;
    }
#endif

    const uint32_t handle = (uint32_t)m_Materials.size();
    m_Materials.push_back(entry);
    m_MaterialComboMap.emplace(key, handle);

    return handle;
}


ID3D11ShaderResourceView* CResource_System::Get_SRV(uint32_t handle) const
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_SRVs.size())
        return nullptr;

    return m_SRVs[handle].Get();
}

const MESH_ENTRY* CResource_System::Get_Mesh(uint32_t handle) const
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Meshes.size())
        return nullptr;

    return &m_Meshes[handle];
}

const SHADER_ENTRY* CResource_System::Get_Shader(uint32_t handle) const
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Shaders.size())
        return nullptr;

    return &m_Shaders[handle];
}

MATERIAL_ENTRY* CResource_System::Get_Material(uint32_t handle)
{
    if (handle == INVALID_HANDLE_UINT || handle >= m_Materials.size())
        return nullptr;

    return &m_Materials[handle];
}
