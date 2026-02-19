#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagResizeEventData : public EVENT_DATA
{
public:
    tagResizeEventData(_uint iW, _uint iH)
    : EVENT_DATA(EVENT_TYPE::On_Window_Resize), iWidth(iW), iHeight(iH) {
    };
    ~tagResizeEventData() override = default;

    _uint   iWidth{}, iHeight{};

}RESIZE_EVENT_DATA;

NS_END
