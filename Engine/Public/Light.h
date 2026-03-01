// Light.h
#pragma once
#include "Engine_Define.h"
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagLightData
{
    OBJECT_HANDLE hObject{};

    LIGHT_TYPE type = LIGHT_TYPE::DIRECTIONAL;

    _float4     vColor = { 1.f, 1.f, 1.f, 1.f };
    _float4     vDirection = { 0.f, -1.f, 0.f, 0.f }; /* for directional/spot */
    _float4     vPosition{};
    _float      fRange = 10.f; /* for point/spot */
    _float      spotAngle = 30.f;                // degrees

    _float4     vDiffuse{ 1.f, 1.f, 1.f , 1.f };
    _float4     vAmbient{ 1.f, 1.f, 1.f , 1.f };
    _float4     vSpecular{ 1.f, 1.f, 1.f , 1.f };

    uint8_t   bEnabled = 1;
    _bool     dirty = true;
} LIGHT_DATA;

class ENGINE_DLL CLight final : public CComponent_Proxy_Base<LIGHT_DATA, CLight, COMPONENT_TYPE::LIGHT>
{
public:
    CLight() : CComponent_Proxy_Base() {}
    CLight(DataType* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle) {}
    ~CLight() override = default;

public:
    void Set_Enabled(_bool bEnable);
    void Set_Type(LIGHT_TYPE eType);
    void Set_Color(const _float4& vColor);
    void Set_Range(_float fRange);
    void Set_SpotAngle(_float fSpotAngle);

    void Set_Diffuse(const _float4& vDiffuse);
    void Set_Ambient(const _float4& vAmbient);
    void Set_Specular(const _float4& vSpecular);
};

NS_END
