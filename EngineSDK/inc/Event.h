#pragma once
#include "Base.h"

NS_BEGIN(Engine)

/* Variadic template for multiple arguments. */
template <typename... Args>
class CEvent : public CBase
{
public :
    CEvent() {};
    virtual ~CEvent() = default;

public :
    using Listener = std::function<void(Args ...)>;
    using ListenerID = size_t;

    /* For lambda or std::function listeners */
    ListenerID Add_Listener(Listener handler)
    {
        m_listeners.push_back(handler);
        return m_iNextID++;
    }

    /* For member function listeners */
    template<typename T>
    ListenerID Add_Listener(void(T::* func)(Args...), T* pInstance)
    {
        return Add_Listener([pInstance, func](Args... args) { (pInstance->*func)(std::forward<Args>(args)...); });
    }

    void Remove_Listener(ListenerID iID)
    {
        m_listeners.erase(iID);
    }

    void Invoke(Args... args)
    {
        for (auto& l : m_listeners)
            l(args...);
    }

    void Clear()
    {
        m_listeners.clear();
    }

    size_t Get_ListenrCount()
    {
        return m_listeners.size();
    }

private:
    std::vector<Listener>    m_listeners;
    size_t                          m_iNextID{};

private :
    void Free() override { m_listeners.clear(); };
};

NS_END
