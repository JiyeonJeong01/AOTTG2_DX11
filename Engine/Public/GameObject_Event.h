#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

class CGameObject;

typedef struct ENGINE_DLL tagGameObjectEventData : public EVENT_DATA
{
public :
    tagGameObjectEventData(EVENT_TYPE eType, CGameObject* pGameObject)
        : EVENT_DATA(eType), m_pGameObject(pGameObject) {
    };
    ~tagGameObjectEventData() override = default;

    CGameObject* m_pGameObject{};
}GAMEOBJECT_EVENT_DATA;

NS_END
