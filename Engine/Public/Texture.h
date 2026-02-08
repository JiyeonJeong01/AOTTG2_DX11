//#pragma once
//#include "CComponent_Proxy_Base.h"
//#include "Asset_Meta.h"
//
//NS_BEGIN(Engine)
//
//typedef struct ENGINE_DLL tagTextureData final
//{
//    ASSET_GUID  tGUID{};    /* Texture Asset */
//    uint16_t    iNumSRVs = 0;
//    uint16_t    iCurrentIndex = 0;
//    uint8_t     iDefaultSlot = 0;
//}TEXTURE_DATA;
//
//class CTexture_Processor;
//
//class ENGINE_DLL CTexture final : public CComponent_Proxy_Base<TEXTURE_DATA, CTexture>
//{
//public:
//    using ProcessorType = CTexture_Processor;
//    using DataType = TEXTURE_DATA;
//
//public:
//    CTexture() : CComponent_Proxy_Base(COMPONENT_TYPE::TEXTURE) {}
//    CTexture(COMPONENT_TYPE eType, DataType* pData, COMPONENT_HANDLE handle)
//        : CComponent_Proxy_Base(eType, pData, handle) {
//    }
//    ~CTexture() override = default;
//
//public:
//    uint16_t Get_NumSRVs() const;
//    uint8_t  Get_DefaultSlot() const;
//    ID3D11ShaderResourceView* Get_SRV(_uint iIndex = 0) const;
//    void Bind_PS(_uint iSlot, _uint iIndex = 0) const;
//};
//
//NS_END
