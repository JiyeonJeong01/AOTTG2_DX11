#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagCinematicEventData : public EVENT_DATA
{
public:
    tagCinematicEventData(const std::string& strEvent)
        : EVENT_DATA(EVENT_TYPE::CINEMATIC), strEventName(strEvent) {
    };
    ~tagCinematicEventData() override = default;

    std::string strEventName = "";
} CINEMATIC_EVENT_DATA;

NS_END
