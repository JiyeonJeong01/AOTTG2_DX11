#include "Resource_System.h"

IMPLEMENT_SINGLETON(CResource_System)


CResource_System::~CResource_System()
{
}

CResource_System::CResource_System()
{
}

HRESULT CResource_System::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
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
    if (hShader == 0) return 0;

    MATERIAL_ENTRY e{};
    e.hShader = hShader;
    e.passIndex = passIndex;

    return Load_Material(e); // 엔트리 버전
}

uint32_t CResource_System::Load_Mesh(const ASSET_GUID& tGUID)
{
    auto it = m_MeshGUIDMap.find(tGUID);
    if (it != m_MeshGUIDMap.end())
        return it->second;

    MESH_ENTRY entry{};

    // TODO: VB/IB 생성

    uint32_t handle = (uint32_t)m_Meshes.size();
    m_Meshes.push_back(entry);
    m_MeshGUIDMap[tGUID] = handle;

    return handle;
}

uint32_t CResource_System::Load_Shader(const ASSET_GUID& tGUID)
{
    auto it = m_ShaderGUIDMap.find(tGUID);
    if (it != m_ShaderGUIDMap.end())
        return it->second;

    SHADER_ENTRY entry{};

    // TODO: Effect 로드 + Pass 캐싱

    uint32_t handle = (uint32_t)m_Shaders.size();
    m_Shaders.push_back(entry);
    m_ShaderGUIDMap[tGUID] = handle;

    return handle;
}

uint32_t CResource_System::Load_Material(const ASSET_GUID& guid)
{
    // 1) guid -> MaterialAsset(또는 json spec) 로드
// 예: material spec에 shaderGUID / passIndex / var names 등이 있음

// ---- 임시 예시 ----
    ASSET_GUID shaderGuid = {};   // material 파일에서 읽었다고 치고
    uint16_t passIndex = 0;

    const uint32_t hShader = Load_Shader(shaderGuid);
    if (hShader == 0) return 0;

    MATERIAL_ENTRY desc{};
    desc.hShader = hShader;
    desc.passIndex = passIndex;

    return Load_Material(desc); // <- 네가 만든 desc 버전 재사용
}

uint32_t CResource_System::Load_Material(const MATERIAL_ENTRY& tDesc)
{
    // 0은 INVALID라고 가정
    if (m_Materials.empty())
        m_Materials.push_back(MATERIAL_ENTRY{}); // dummy [0]

    if (tDesc.hShader == 0)
    {
        _DEBUG_ERROR_BREAK("Load_Material failed: invalid shader handle(0).");
        return 0;
    }

    // 캐시 키: shader(32) + pass(16)
    const uint64_t key = (uint64_t(tDesc.hShader) << 16) | uint64_t(tDesc.passIndex);

    if (auto it = m_MaterialKeyMap.find(key); it != m_MaterialKeyMap.end())
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

    entry.pWorld = FindMatVar({ "g_World", "gWorld", "World", "uWorld" });
    entry.pView = FindMatVar({ "g_View",  "gView",  "View",  "uView" });
    entry.pProj = FindMatVar({ "g_Proj",  "gProj",  "Proj",  "uProj" });

#ifdef _DEBUG
    if (!entry.pWorld || !entry.pView || !entry.pProj)
        _DEBUG_WARN("Material matrix variables missing. Check fx variable names.");
#endif

    const uint32_t handle = (uint32_t)m_Materials.size();
    m_Materials.push_back(entry);
    m_MaterialKeyMap.emplace(key, handle);

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
}
