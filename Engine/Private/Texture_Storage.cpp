//#include "Texture_Storage.h"
//#include "Asset_Registry.h"
//#include "Resource_System.h"
//
//NS_BEGIN(Engine)
//
//HRESULT CTexture_Storage::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
//{
//    m_pDevice = pDevice;
//    m_pContext = pContext;
//
//    return S_OK;
//}
//
//void CTexture_Storage::Free()
//{
//    __super::Free();
//
//    m_SRVs.clear();
//    m_GUIDToHandle.clear();
//}
//
//bool CTexture_Storage::Is_Loaded(const ASSET_GUID& tGUID) const
//{
//    return (m_GUIDToHandle.find(tGUID) != m_GUIDToHandle.end());
//}
//
//CTexture_Storage::TEXTURE_SRV_HANDLE CTexture_Storage::Load_Texture(const ASSET_GUID& tGUID)
//{
//    auto it = m_GUIDToHandle.find(tGUID);
//    if (it != m_GUIDToHandle.end())
//        return it->second;
//
//    /* Get path from registry */
//    const ASSET_RECORD* pRec = CAsset_Registry::GetInstance()->Find(tGUID);
//    if (!pRec)
//        return INVALID_TEXTURE_HANDLE;
//
//    const std::filesystem::path& path = pRec->path;
//
//
//    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
//    HRESULT hr = E_FAIL;
//
//    const auto ext = path.extension().wstring();
//    if (ext == L".tga")
//        hr = E_FAIL;
//    else if (ext == L".dds" || ext == L".DDS")
//        hr = DirectX::CreateDDSTextureFromFile(m_pDevice, path.c_str(), nullptr, srv.GetAddressOf());
//    else
//        hr = DirectX::CreateWICTextureFromFile(m_pDevice, path.c_str(), nullptr, srv.GetAddressOf());
//
//    if (FAILED(hr) || !srv)
//        return INVALID_TEXTURE_HANDLE;
//
//    /* Store */
//    TEXTURE_SRV_HANDLE handle = SCAST(TEXTURE_SRV_HANDLE, m_SRVs.size());
//    m_SRVs.push_back(srv);
//    m_GUIDToHandle.emplace(tGUID, handle);
//
//    return handle;
//}
//
//ID3D11ShaderResourceView* CTexture_Storage::Get_SRV(TEXTURE_SRV_HANDLE handle) const
//{
//    if (handle == INVALID_TEXTURE_HANDLE)
//        return nullptr;
//    if (handle >= m_SRVs.size())
//        return nullptr;
//
//    return m_SRVs[handle].Get();
//}
//
//NS_END
