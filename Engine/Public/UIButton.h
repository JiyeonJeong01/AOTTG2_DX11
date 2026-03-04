#pragma once
#include "Engine_Define.h"
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

class CUI_Processor;

typedef struct tagUIButtonData
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    COMPONENT_HANDLE    hRectTransform{};
    COMPONENT_HANDLE    hTargetCanvas{};

    UI_BTN_STATE        eState = UI_BTN_STATE::Normal;
    _bool               bInteractable = true;

    /* 상태별로 색, 텍스쳐 바꾸기 */
    _float4 normal{ 1,1,1,1 };
    _float4 hover{ 1,1,1,1 };
    _float4 pressed{ 1,1,1,1 };
    _float4 disabled{ 1,1,1,1 };

    uint32_t normalTex = INVALID_HANDLE_UINT;
    RECT_F   normalUV{};
    uint32_t hoverTex = INVALID_HANDLE_UINT;
    RECT_F   hoverUV{};
    uint32_t pressedTex = INVALID_HANDLE_UINT;
    RECT_F   pressedUV{};

    uint8_t  visualPriority = 10;
    uint32_t onClickEventId = 0;
} UI_BUTTON_DATA;

class CUIButton final : public CComponent_Proxy_Base<UI_BUTTON_DATA, CUIButton, COMPONENT_TYPE::UI_BUTTON>
{
public:
    using ProcessorType = CUI_Processor;

public:
    CUIButton() : CComponent_Proxy_Base() {}
    CUIButton(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {
    }
    ~CUIButton() override = default;

    void Set_TargetCanvas(COMPONENT_HANDLE hCanvas);
    void Set_RectTransform(COMPONENT_HANDLE hRectTransform);
    void Set_Interactable(_bool bInteractable);
};

NS_END
