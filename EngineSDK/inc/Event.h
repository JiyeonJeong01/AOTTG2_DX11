#pragma once
#include "Engine_Define.h"

NS_BEGIN(Engine)

using ListenerID = size_t;


template <typename... Args>
class CEvent
{
public:
    using Listener = std::function<void(Args...)>;

    CEvent() = default;
    ~CEvent() {
        Clear();
    };

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
        if (it == m_listeners.end())
            return;

        /* Invoke 순회 중(콜백 안에서 해제)에는 지우지 않고 비워만 둔다. 순회가 끝나면 한꺼번에 지운다 */
        if (m_iInvokeDepth > 0)
        {
            it->second = nullptr;
            m_bPendingErase = true;
            return;
        }

        m_listeners.erase(it);
    }

    void Invoke(Args... args)
    {
        ++m_iInvokeDepth;
        for (auto const& [id, handler] : m_listeners)
        {
            if (handler)
                handler(args...);
        }
        --m_iInvokeDepth;

        if (m_iInvokeDepth == 0 && m_bPendingErase)
        {
            std::erase_if(m_listeners, [](const auto& pair) { return !pair.second; });
            m_bPendingErase = false;
        }
    }

    void Clear()
    {
        if (m_iInvokeDepth > 0)
        {
            for (auto& [id, handler] : m_listeners)
                handler = nullptr;
            m_bPendingErase = true;
            return;
        }

        m_listeners.clear();
    }

    size_t Get_ListenerCount() const
    {
        return m_listeners.size();
    }

private:
    std::map<ListenerID, Listener> m_listeners;
    ListenerID m_iNextID = 0;

    uint32_t m_iInvokeDepth = 0;    /* 콜백 안에서 다시 Invoke할 수 있어 깊이로 센다 */
    bool     m_bPendingErase = false;
};

NS_END
