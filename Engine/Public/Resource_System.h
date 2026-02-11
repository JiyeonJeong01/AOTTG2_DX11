#pragma once
#include "Identity.h"
#include "Base.h"

NS_BEGIN(Engine)

class CTexture_Storage;
class CMesh_Storage;

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
    uint32_t Load_Texture(const ASSET_GUID& tGUID);
    uint32_t Load_Mesh(const ASSET_GUID& tGUID);

public:
    CTexture_Storage* Get_Texture_Storage() const { return m_pTextures; }

private:
    CTexture_Storage* m_pTextures = nullptr;
    CMesh_Storage* m_pMeshes = nullptr;

private :
    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;
};



NS_END
