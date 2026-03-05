#pragma once
#include "Identity.h"
#include "Base.h"
#include "Material.h"
#include "Mesh.h"
#include "Render_Struct.h"
#include "Shader.h"
#include "Texture.h"

NS_BEGIN(Engine)
    /*
* @class CResource_System
* @brief Manages the lifecycle of live objects loaded into memory(CPU / GPU).
* Converts raw assets identified by GUIDs into usable engine resources(Textures, Meshes, Shaders).
* It handles resource loading, caching(to prevent duplicates), and storage management.
*/
class ENGINE_DLL CResource_System final : public CBase
{
    DECLARE_SINGLETON(CResource_System)
public:
    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    void    Clear();

public:
    uint32_t Load_Mesh(const ASSET_GUID& tGUID);
    uint32_t Load_Material(const ASSET_GUID& tGUID);
    uint32_t Load_Material(const MATERIAL_ENTRY& desc);
    uint32_t Load_Shader(const ASSET_GUID& tGUID);
    uint32_t Load_Texture(const ASSET_GUID& tGUID);

    uint32_t Load_Material_Temp(const ASSET_GUID& materialGuidAsShaderGuid, uint16_t passIndex);


    const MESH_ENTRY*           Get_Mesh(uint32_t handle) const;
    MATERIAL_ENTRY*             Get_Material(uint32_t handle);
    SHADER_ENTRY*               Get_Shader(uint32_t handle);
    const TEXTURE_ENTRY*        Get_Texture(uint32_t handle) const;

public:
    uint32_t Alloc_PerObjectParamBlock();
    void     Free_PerObjectParamBlock(uint32_t handle);
    PER_OBJECT_PARAM_BLOCK* Get_PerObjectParamBlock(uint32_t handle);

private :
    _bool Read_MetaFileDecl(const std::filesystem::path& metaPath, uint32_t& outDecl);


private :
    /* ---- MESH ---- */
    std::vector<MESH_ENTRY> m_Meshes;
    std::unordered_map<ASSET_GUID, uint32_t, ASSET_GUID_HASHER> m_MeshGUIDMap;

    /* ---- MATERIAL_VTXTEX ---- */
    std::vector<MATERIAL_ENTRY> m_Materials;
    std::unordered_map<ASSET_GUID, uint32_t, ASSET_GUID_HASHER> m_MaterialGUIDMap;
    std::unordered_map<uint64_t, uint32_t> m_MaterialComboMap;

    /* ---- SHADER ---- */
    std::vector<SHADER_ENTRY> m_Shaders;
    std::unordered_map<ASSET_GUID, uint32_t, ASSET_GUID_HASHER> m_ShaderGUIDMap;

    /* ---- TEXTURE ---- */
    std::vector<TEXTURE_ENTRY> m_Textures;
    std::unordered_map<ASSET_GUID, uint32_t, ASSET_GUID_HASHER> m_TextureGUIDMap;

    /* ---- Object Param ---- */
    CPerObjectParamPool m_PerObjectParamPool;

private :
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;
};

NS_END
