// RectTransform.h
#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagRectTransformData final
{
    OBJECT_HANDLE hObject{};

    // --- Input (authoring) ---
    _float2 vPosPx{ 0.f, 0.f };      // UI X,Y (px) from top-left
    _float2 vSizePx{ 100.f, 100.f }; // UI W,H (px)

    // --- Cache ---
    _float4x4 matWorld{};           // World for UI quad (screen->centered)

    _bool bDirty{ true };
}RECTTRANSFORM_DATA;

class ENGINE_DLL CRectTransform final
    : public CComponent_Proxy_Base<RECTTRANSFORM_DATA, CRectTransform, COMPONENT_TYPE::RECT_TRANSFORM>
{
public:
    CRectTransform() : CComponent_Proxy_Base() { }
    CRectTransform(DataType* pData, COMPONENT_HANDLE h) : CComponent_Proxy_Base(pData, h) { }
    ~CRectTransform() override = default;

public:
    // --- Setters ---
    void Set_PositionPx(_float x, _float y); 
    void Set_SizePx(_float w, _float h); 
public:
    // --- Getters ---
    const _float4x4& Get_World(); 
    const _float4x4& Get_View();
    const _float4x4& Get_Proj();

    _float2 Get_PositionPx() const;
    _float2 Get_SizePx() const;
};

NS_END
