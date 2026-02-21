#pragma once
#include "Engine_Define.h"
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagUIImageData
{
    OBJECT_HANDLE    hObject{};
    COMPONENT_HANDLE hCanvasRenderer{};

    uint32_t hTexture = INVALID_HANDLE_UINT;
    RECT_F   rcUV{};                    // UV (0~1)
    _float4  color{ 1,1,1,1 };

    uint8_t  visualPriority = 0;        // 0:기본, 버튼/상태가 더 높게 덮어쓰게 등
    _bool    dirty = true;
} UI_IMAGE_DATA;

class ENGINE_DLL CUIImage final : public CComponent_Proxy_Base<UI_IMAGE_DATA, CUIImage, COMPONENT_TYPE::UI_IMAGE>
{
public:
    CUIImage() : CComponent_Proxy_Base() {}
    CUIImage(DataType* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle) {}
    ~CUIImage() override = default;

    // --- setters (변경 + dirty) ---
    void Set_CanvasRenderer(COMPONENT_HANDLE hCanvas);
    void Set_Texture(uint32_t hTex);
    void Set_UV(const RECT_F& rcUV);
    void Set_Color(const _float4& vColor);
    void Set_VisualPriority(uint8_t p);
};
NS_END
