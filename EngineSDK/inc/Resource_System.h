#pragma once
#include "Identity.h"
#include "Base.h"
#include "Material.h"
#include "Mesh.h"
#include "Render_Struct.h"
#include "Shader.h"
#include "Texture.h"
#include "Model.h"

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
    uint32_t Load_Model(const ASSET_GUID& desc);
    uint32_t Load_Material(const ASSET_GUID& tGUID);
    uint32_t Load_Material(const MATERIAL_ENTRY& desc);
    uint32_t Load_Shader(const ASSET_GUID& tGUID);
    uint32_t Load_Texture(const ASSET_GUID& tGUID);

    uint32_t Register_MeshEntry(MESH_ENTRY&& pEntry);

    MESH_ENTRY*           Get_Mesh(uint32_t handle);
    MODEL_ENTRY*          Get_Model(uint32_t handle);
    MATERIAL_ENTRY*       Get_Material(uint32_t handle);
    SHADER_ENTRY*         Get_Shader(uint32_t handle);
    TEXTURE_ENTRY*        Get_Texture(uint32_t handle);

    const ASSET_GUID&       Find_GUID_By_Handle(ASSET_TYPE eType, _uint iHandle);
    const std::string&      Find_Name_By_GUID(const ASSET_GUID& tGUID);


public:
    uint32_t Alloc_PerObjectParamBlock();
    void     Free_PerObjectParamBlock(uint32_t handle);
    PER_OBJECT_PARAM_BLOCK* Get_PerObjectParamBlock(uint32_t handle);

private :
    _bool Read_MetaFileDecl(const std::filesystem::path& metaPath, uint32_t& outDecl);

private:
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;

private :
    /* ---- MESH ---- */
    std::vector<MESH_ENTRY> m_Meshes;
    std::unordered_map<ASSET_GUID, uint32_t, ASSET_GUID_HASHER> m_MeshGUIDMap;


    /* ---- MODEL ---- */
    std::vector<MODEL_ENTRY> m_Models;
    std::unordered_map<ASSET_GUID, uint32_t, ASSET_GUID_HASHER> m_ModelGUIDMap;

    /* ---- MATERIAL ---- */
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
    static constexpr uint32_t ASSET_TAG_MESH = 0x00000000u;
    static constexpr uint32_t ASSET_TAG_MODEL = 0x80000000u;
    static constexpr uint32_t ASSET_INDEX_MASK = 0x7FFFFFFFu;
    inline uint32_t Make_MeshHandle(uint32_t idx) const noexcept { return (idx & ASSET_INDEX_MASK); }                     /* MSB = 0 */
    inline uint32_t Make_ModelHandle(uint32_t idx)const  noexcept { return (idx & ASSET_INDEX_MASK) | ASSET_TAG_MODEL; }  /* MSB = 1 */

public :
    inline _bool Is_ModelHandle(uint32_t h) const noexcept { return (h & ASSET_TAG_MODEL) != 0; }
    inline uint32_t Handle_Index(uint32_t h) const noexcept { return (h & ASSET_INDEX_MASK); }
};

NS_END
