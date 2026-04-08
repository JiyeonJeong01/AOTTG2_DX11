#pragma once
#include "Engine_Define.h"
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

class CUI_Processor;

typedef struct ENGINE_DLL tagUITextData
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    COMPONENT_HANDLE    hCanvasRenderer{};
    COMPONENT_HANDLE    hRectTransform{};

    uint32_t            hFont = INVALID_HANDLE_UINT;
    std::basic_string<_tchar> strText{};

    _float4             color{ 1.f, 1.f, 1.f, 1.f };

    _float              fScale = 1.f;
    _float2             vOffset = { 0.f, 0.f };
    _bool               bCenter = true;

    uint8_t             visualPriority = 0;
    _bool               dirty = true;

    uint32_t            flags = 0;
    _float              sortZ = 0.f;
    RECT_F              rcClip{ 0.f, 0.f, 0.f, 0.f };

} UI_TEXT_DATA;

class ENGINE_DLL CUIText final : public CComponent_Proxy_Base<UI_TEXT_DATA, CUIText, COMPONENT_TYPE::UI_TEXT>
{
public:
    using ProcessorType = CUI_Processor;

public:
    CUIText() : CComponent_Proxy_Base() {}
    CUIText(DataType* pData, COMPONENT_HANDLE handle) : CComponent_Proxy_Base(pData, handle) {}
    ~CUIText() override = default;

public:
    void Set_CanvasRenderer(COMPONENT_HANDLE hCanvas);
    void Set_Font(uint32_t hFont);
    void Set_Text(const std::basic_string<_tchar>& strText);
    void Set_Text(const _tchar* pText);
    void Set_Color(const _float4& vColor);
    void Set_Scale(_float fScale);
    void Set_CenterAlign(_bool bCenter);
    void Set_Offset(_float2 vOffset);
    void Set_VisualPriority(uint8_t p);

public:
    const std::basic_string<_tchar>& Get_Text() const;
};

NS_END
