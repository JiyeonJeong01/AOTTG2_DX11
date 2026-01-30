#include "GameObject_Event.h"

CGameObject_Event CGameObject_Event::Create(EVENT_TYPE eType, CGameObject* pGameObject)
{
    return CGameObject_Event(eType, pGameObject);
}

void CGameObject_Event::Free()
{
    CEventData::Free();
}
