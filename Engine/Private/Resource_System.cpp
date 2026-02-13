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
    //if (hShader == 0) return 0;

    MATERIAL_ENTRY e{};
    e.hShader = hShader;
    e.passIndex = passIndex;

    return Load_Material(e); // 엔트리 버전
}

uint32_t CResource_System::Load_Mesh(const ASSET_GUID& tGUID)
{
    //auto it = m_MeshGUIDMap.find(tGUID);
    //if (it != m_MeshGUIDMap.end())
    //    return it->second;

    //MESH_ENTRY entry{};

    //// TODO: VB/IB 생성

    //uint32_t handle = (uint32_t)m_Meshes.size();
    //m_Meshes.push_back(entry);
    //m_MeshGUIDMap[tGUID] = handle;

    //return handle;

    // 1. 이미 로드된 메시인지 확인
    auto it = m_MeshGUIDMap.find(tGUID);
    if (it != m_MeshGUIDMap.end())
        return it->second;

    MESH_ENTRY entry{};
    HRESULT hr = S_OK;

    // 2. 임시 처리: 만약 요청한 GUID가 CUBE라면 빌더로 생성
    if (tGUID == DEFAULT_ASSET_GUID::MESH_CUBE)
    {
        // CMeshBuilder가 싱글톤이거나 정적 함수라고 가정합니다.
        hr = CMeshBuilder::Create_Cube_VtxCol(m_pDevice, entry);
    }
    else
    {
        // [TODO] 나중에 진짜 .mesh 파일을 읽는 로직이 들어갈 곳
        // std::filesystem::path path = SYS_ASSET.Get_Asset_Path(tGUID);
        // hr = Load_Mesh_From_File(path, entry); 
        return uint32_t(-1); // 지금은 파일 로드가 없으니 에러 리턴
    }

    if (FAILED(hr))
    {
        _DEBUG_ERROR_BREAK("Failed to Create/Load Mesh");
        return uint32_t(-1);
    }

    // 3. 컨테이너 등록
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
        return uint32_t(-1); // TODO : 에러 핸들 0이랑 -1 중 뭐로 할 건지 제대로 정해야함 
    }

    SHADER_ENTRY entry{};

    Microsoft::WRL::ComPtr<ID3DX11Effect> pEffect = nullptr;

    _uint			iHlslFlag = {};

#ifdef _DEBUG
    iHlslFlag |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
    iHlslFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif	

    if(FAILED(D3DX11CompileEffectFromFile(shaderPath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iHlslFlag, 0, m_pDevice, entry.pEffect.GetAddressOf(), nullptr)))
    {
        _DEBUG_ERROR_BREAK("Create Effect file failed");
        return uint32_t(-1); // TODO : 인밸리드 핸들 값 좀 제대로 정해라!!!!!!!!!!!!!!!!!
    }

    // TODO : TEch는 한 개짐나 혹시 모르니 
    entry.pTech = entry.pEffect->GetTechniqueByIndex(0);

    D3DX11_TECHNIQUE_DESC TechniqueDesc{};
    entry.pTech->GetDesc(&TechniqueDesc);

    for(uint32_t i = 0; i < TechniqueDesc.Passes; ++i)
    {
        SHADER_ENTRY::PASS_CACHE cache{};
        cache.pPass = entry.pTech->GetPassByIndex(i);

        // TODO 일단은 하드코딩!!!!!!!!!!!!!!
        D3DX11_PASS_DESC PassDesc{};
        cache.pPass->GetDesc(&PassDesc);

        if (FAILED(m_pDevice->CreateInputLayout(VTXCOL_LAYOUT, _countof(VTXCOL_LAYOUT), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, cache.pInputLayout.GetAddressOf())))
        {
            _DEBUG_ERROR_BREAK("ffffffffffaaaaaaaaaaaaaiiiiiiiiillllllllleeeeeeeeeeeedddddddddd");
            return uint32_t(-1);
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
    // 1) 이미 로드된 머티리얼인지 확인 (캐싱)
    auto it = m_MaterialGUIDMap.find(tGUID);
    if (it != m_MaterialGUIDMap.end())
        return it->second;

    std::filesystem::path matPath = SYS_ASSET.Get_Asset_Path(tGUID);

    ASSET_GUID shaderGuid = DEFAULT_ASSET_GUID::SHADER_VTXCOL; // 읽어왔다고 가정
    uint16_t passIndex = 0;

    // 4) 로드된 정보를 바탕으로 엔트리 생성
    MATERIAL_ENTRY desc{};
    desc.hShader = Load_Shader(shaderGuid);
    desc.passIndex = passIndex;

    if (desc.hShader == uint32_t(-1)) return uint32_t(-1);

    // 5) 실제 리소스로 등록 (기존 Load_Material 재사용)
    uint32_t hMaterial = Load_Material(desc);

    // 6) GUID 맵에 등록해서 다음번엔 바로 찾게 함
    m_MaterialGUIDMap[tGUID] = hMaterial;

    return hMaterial;
}

uint32_t CResource_System::Load_Material(const MATERIAL_ENTRY& tDesc)
{
    // 0은 INVALID라고 가정
    if (m_Materials.empty())
        m_Materials.push_back(MATERIAL_ENTRY{}); // dummy [0]

    //if (tDesc.hShader == 0)
    //{
    //    _DEBUG_ERROR_BREAK("Load_Material failed: invalid shader handle(0).");
    //    return 0;
    //}

    // 캐시 키: shader(32) + pass(16)
    const uint64_t key = (uint64_t(tDesc.hShader) << 16) | uint64_t(tDesc.passIndex);

    if (auto it = m_MaterialComboMap.find(key); it != m_MaterialComboMap.end())
        return it->second;

    const SHADER_ENTRY* pShader = Get_Shader(tDesc.hShader);
    if (!pShader || !pShader->Is_Valid())
    {
        _DEBUG_ERROR_BREAK("Load_Material failed: shader entry invalid.");
        return 0;
    }

    if (tDesc.passIndex >= pShader->pPasses.size())
    {
        _DEBUG_ERROR_BREAK("Load_Material failed: passIndex out of range.");
        return 0;
    }

    MATERIAL_ENTRY entry = tDesc;

    // Effect variable 캐시 (이름은 니 fx에 맞게 수정)
    ID3DX11Effect* pFx = pShader->pEffect.Get();
    if (!pFx)
        return 0;

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

    // .fx 파일의 변수명과 정확히 일치시켜야 합니다!
    entry.pWorld = FindMatVar({ "g_WorldMatrix", "g_World", "World" });
    entry.pView = FindMatVar({ "g_ViewMatrix",  "g_View",  "View" });
    entry.pProj = FindMatVar({ "g_ProjMatrix",  "g_Proj",  "Proj" });

#ifdef _DEBUG
    if (!entry.pWorld || !entry.pView || !entry.pProj)
        _DEBUG_WARN("Material matrix variables missing. Check fx variable names.");
#endif

    const uint32_t handle = (uint32_t)m_Materials.size();
    m_Materials.push_back(entry);
    m_MaterialComboMap.emplace(key, handle);

    return handle;
}


ID3D11ShaderResourceView* CResource_System::Get_SRV(uint32_t handle) const
{
    if (handle >= m_SRVs.size())
        return nullptr;

    return m_SRVs[handle].Get();
}

const MESH_ENTRY* CResource_System::Get_Mesh(uint32_t handle) const
{
    if (handle >= m_Meshes.size())
        return nullptr;

    return &m_Meshes[handle];
}

const SHADER_ENTRY* CResource_System::Get_Shader(uint32_t handle) const
{
    if (handle >= m_Shaders.size())
        return nullptr;

    return &m_Shaders[handle];
}

MATERIAL_ENTRY* CResource_System::Get_Material(uint32_t handle)
{
    if (handle >= m_Materials.size())
        return nullptr;

    return &m_Materials[handle];
    //if (handle >= m_Materials.size())
    //    return nullptr;

    //return &m_Materials[handle];
}
