#include "GameObject_Event.h"

NS_BEGIN(Engine)

CGameObject_Event CGameObject_Event::Create(EVENT_TYPE eType, CGameObject* pGameObject)
{
    return CGameObject_Event(eType, pGameObject);
}

void CGameObject_Event::Free()
{
    CEventData::Free();
}

NS_END
