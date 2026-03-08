#pragma once
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagCanvasRendererData final
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    COMPONENT_HANDLE hRectTransform = INVALID_HANDLE;

    uint32_t hMaterial = INVALID_HANDLE_UINT;
    uint32_t hTexture = INVALID_HANDLE_UINT;

    // draw params
    _float4 vColor = { 1.f, 1.f, 1.f, 1.f };
    RECT_F  rcUV = { 0.f, 0.f, 1.f, 1.f };

    // clip rect in screen space
    RECT_F  rcClip = { 0.f, 0.f, 0.f, 0.f };

    uint32_t flags = CF_NONE;
    RENDER_LAYER layer = RENDER_LAYER::UI;

    _float   sortZ = 0.f;
    uint8_t             visualPriority = 0;
    _bool               dirty = true;

} CANVAS_RENDERER_DATA;

class ENGINE_DLL CCanvasRenderer final
    : public CComponent_Proxy_Base<CANVAS_RENDERER_DATA, CCanvasRenderer, COMPONENT_TYPE::CANVAS_RENDERER>
{
public:
    CCanvasRenderer();
    CCanvasRenderer(DataType* pData, COMPONENT_HANDLE handle);
    virtual ~CCanvasRenderer() override = default;

public:
    // Settings
    void Set_Material(uint32_t h);
    void Set_Texture(uint32_t h);

    void Set_Color(const _float4& vColor);
    void Set_UV(const RECT_F& rcUV);

    void Enable_ClipRect(_bool b);
    void Set_ClipRect(const RECT_F& rcClip);

    void Set_Layer(RENDER_LAYER elAYER);
    void Set_Flags(uint32_t iFlag);
    void Add_Flags(uint32_t iFlag);
    void Remove_Flags(uint32_t iFlag);
    void Set_SortZ(_float z);

public:
    // Getters
    COMPONENT_HANDLE Get_RectTransform() const;
    uint32_t Get_Material() const;
    uint32_t Get_Texture() const;
    uint32_t Get_Flags() const;
    RENDER_LAYER Get_Layer() const;
    float Get_SortZ() const;

    const _float4& Get_Color() const;
    const RECT_F& Get_UV() const;
    const RECT_F& Get_ClipRect() const;
};

NS_END
