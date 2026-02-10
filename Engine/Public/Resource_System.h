//#pragma once
//#include "Identity.h"
//#include "Base.h"
//
//NS_BEGIN(Engine)
//
//class CTexture_Storage;
//class CMesh_Storage;
//
//class ENGINE_DLL CResource_System final : public CBase
//{
//    DECLARE_SINGLETON(CResource_System)
//
//private:
//    CResource_System() = default;
//    virtual ~CResource_System() override = default;
//
//public:
//    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
//    void    Clear();
//
//public:
//    uint32_t Load_Texture(const ASSET_GUID& tGUID);
//    uint32_t Load_Mesh(const ASSET_GUID& tGUID);
//
//public:
//    CTexture_Storage* Get_Texture_Storage() const { return m_pTextures; }
//
//private:
//    CTexture_Storage* m_pTextures = nullptr;
//    CMesh_Storage* m_pMeshes = nullptr;
//
//private :
//    ID3D11Device* m_pDevice = nullptr;
//    ID3D11DeviceContext* m_pContext = nullptr;
//};
//
//NS_END
