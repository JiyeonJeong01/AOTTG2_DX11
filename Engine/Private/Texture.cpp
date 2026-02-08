//#include "Texture.h"
//
//#include "Resource_System.h"
//#include "Texture_Storage.h"
//
//NS_BEGIN(Engine)
//
//uint16_t CTexture::Get_NumSRVs() const
//{
//    if (!m_pData) return 0;
//    return m_pData->iNumSRVs;
//}
//
//uint8_t CTexture::Get_DefaultSlot() const
//{
//    if (!m_pData) return 0;
//    return m_pData->iDefaultSlot;
//}
//
//ID3D11ShaderResourceView* CTexture::Get_SRV(_uint iIndex) const
//{
//    if (!m_pData)
//        return nullptr;
//
//    uint16_t idx = static_cast<uint16_t>(iIndex);
//    if (m_pData->iNumSRVs > 0 && idx >= m_pData->iNumSRVs)
//        idx = m_pData->iCurrentIndex;
//
//    auto* pRes = CResource_System::GetInstance();
//    auto* pTexStorage = pRes->Get_Texture_Storage();
//    if (!pTexStorage)
//        return nullptr;
//
//    // NOTE: TextureAsset을 나중에 도입하면 여기서 (firstHandle + idx) 같은 방식으로 바뀜
//    const auto h = pRes->Load_Texture(m_pData->tGUID);
//    return pTexStorage->Get_SRV(h /* + idx (멀티 SRV면 여기 반영) */);
//}
//
//void CTexture::Bind_PS(_uint iSlot, _uint iIndex) const
//{
//    if (!m_pData)
//        return;
//
//    auto* pRes = CResource_System::GetInstance();
//    auto* pCtx = pRes->Get_Context();
//    if (!pCtx)
//        return;
//
//    ID3D11ShaderResourceView* pSRV = Get_SRV(iIndex);
//    if (!pSRV)
//        return;
//
//    pCtx->PSSetShaderResources(iSlot, 1, &pSRV);
//}
//
//NS_END
//
//
