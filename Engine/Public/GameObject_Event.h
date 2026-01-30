#pragma once
#include "EventData.h"

NS_BEGIN(Engine)

class CGameObject;

class ENGINE_DLL CGameObject_Event : public CEventData
{
private :
    CGameObject_Event(EVENT_TYPE eType, CGameObject* pGameObject)
        : CEventData(eType), m_pGameObject(pGameObject) {}
    ~CGameObject_Event() override = default;

public :
    CGameObject* Get_GameObject() { return m_pGameObject; }

private :
    CGameObject* m_pGameObject{};

public :
    static CGameObject_Event Create(EVENT_TYPE eType, CGameObject* pGameObject);
private :
    void Free() override;
};

NS_END
