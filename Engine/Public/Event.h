#pragma once
#include "Base.h"
#include <map>
#include <functional>

NS_BEGIN(Engine)

using ListenerID = size_t;


template <typename... Args>
class CEvent : public CBase
{
public:
    using Listener = std::function<void(Args...)>;

    CEvent() = default;
    ~CEvent() override = default;

public:

    /* For lambda or std::function listeners */
    ListenerID Add_Listener(Listener handler)
    {
        if (!handler) return 0;

        ListenerID id = ++m_iNextID;
        m_listeners[id] = handler;
        return id;
    }

    /* For member function listeners */
    template <typename T>
    ListenerID Add_Listener(void(T::* func)(Args...), T* pInstance)
    {
        if (!pInstance || !func) return 0;

        return Add_Listener([pInstance, func](Args... args) {
            (pInstance->*func)(std::forward<Args>(args)...);
            });
    }

    void Remove_Listener(ListenerID iID)
    {
        auto it = m_listeners.find(iID);
        if (it != m_listeners.end())
        {
            m_listeners.erase(it);
        }
    }

    void Invoke(Args... args)
    {
        for (auto const& [id, handler] : m_listeners)
        {
            if (handler)
                handler(args...);
        }
    }

    void Clear()
    {
        m_listeners.clear();
    }

    size_t Get_ListenerCount() const
    {
        return m_listeners.size();
    }

private:
    std::map<ListenerID, Listener> m_listeners;
    ListenerID m_iNextID = 0;

private:
    void Free() override { Clear(); }
};

NS_END
