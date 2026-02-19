#pragma once
#include "Event.h"

NS_BEGIN(Engine)

class ENGINE_DLL CEvent_System final
{
    DECLARE_SINGLETON(CEvent_System)
public:
    HRESULT Initialize();

    /* Subscribe for member function listeners */
    template <typename Object>
    ListenerID Subscribe(EVENT_TYPE eType, void(Object::* func)(EVENT_DATA&), Object* pInstance)
    {
        return m_Events[eType].Add_Listener(func, pInstance);
    }

    /* Subscribe for lambda of static function listeners */
    ListenerID Subscribe(EVENT_TYPE eType, std::function<void(EVENT_DATA&)> handler)
    {
        return m_Events[eType].Add_Listener(handler);
    }

    /* Unsubscribe using ListenerID */
    void Unsubscribe(EVENT_TYPE eType, ListenerID iID)
    {
        auto it = m_Events.find(eType);

        if (it != m_Events.end())
            it->second.Remove_Listener(iID);
    }

    void Trigger(EVENT_DATA& eventData)
    {
        auto it = m_Events.find(eventData.eType);

        if (it != m_Events.end())
            it->second.Invoke(eventData);
    }

    void Clear()
    {
        for (auto& pair : m_Events)
            pair.second.Clear();

        m_Events.clear();
    }

private:
    std::unordered_map<EVENT_TYPE, CEvent<EVENT_DATA&>> m_Events;
};

NS_END
