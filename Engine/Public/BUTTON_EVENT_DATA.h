#pragma once
#include "Event.h"

NS_BEGIN(Engine)

class CGameObject;

typedef struct ENGINE_DLL tagButtonEventData : public EVENT_DATA
{
public :
    tagButtonEventData(CGameObject* pObj)
        : EVENT_DATA(EVENT_TYPE::BUTTON), pObject(pObj) {};

        CGameObject* pObject{};
}BUTTON_EVENT_DATA;


NS_END
