//#pragma once
//#include <wrl/client.h>
//#include "Asset_Meta.h"
//
//struct ID3D11ShaderResourceView;
//
//NS_BEGIN(Engine)
//
//class ENGINE_DLL CTexture_Storage final : CBase
//{
//public:
//    using TEXTURE_SRV_HANDLE = uint32_t;
//    static constexpr TEXTURE_SRV_HANDLE INVALID_TEXTURE_HANDLE = 0xFFFFFFFFu;
//
//public:
//    CTexture_Storage() = default;
//    ~CTexture_Storage() override = default;
//
//public:
//    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
//    void    Free();
//
//public:
//    /* GUID -> SRV handle (단일 텍스처, TextureAsset 도입 전 임시) */
//    TEXTURE_SRV_HANDLE Load_Texture(const ASSET_GUID& guid);
//
//    ID3D11ShaderResourceView* Get_SRV(TEXTURE_SRV_HANDLE handle) const;
//
//    bool Is_Loaded(const ASSET_GUID& tGUID) const;
//
//private:
//    /* SRV 저장소 */
//    std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_SRVs;
//
//    /* GUID -> handle */
//    std::unordered_map<ASSET_GUID, TEXTURE_SRV_HANDLE, ASSET_GUID_HASHER> m_GUIDToHandle;
//
//    ID3D11Device* m_pDevice = nullptr;
//    ID3D11DeviceContext* m_pContext = nullptr;
//};
//
//NS_END
