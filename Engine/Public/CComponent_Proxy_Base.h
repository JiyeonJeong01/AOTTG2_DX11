#pragma once
#include "Base.h"

template <typename TProxy> class CComponent_Processor_Impl;

/**
 * Base class for all Component Proxies.
 * Uses CRTP (Curiously Recurring Template Pattern) to bind Data and Proxy types.
 * @tparam DATA_T  The raw struct containing component data (e.g., CTransform_Data)
 * @tparam TProxy_T The actual derived proxy class (e.g., CTransform_Proxy)
 */
template <typename DATA_T, typename TProxy_T>
class CComponent_Proxy_Base : public CBase
{
public:
    /* Expose types for the Component System's type traits and Pool allocation */
    using DataType = DATA_T;
    using ProcessorType = CComponent_Processor_Impl<TProxy_T>;

    CComponent_Proxy_Base() = default;
    CComponent_Proxy_Base(COMPONENT_TYPE eType) : m_eComType(eType) {}
    CComponent_Proxy_Base(DATA_T* pData, COMPONENT_HANDLE handle) : m_hHandle(handle), m_pData(pData){}
    CComponent_Proxy_Base(COMPONENT_TYPE eType, DATA_T* pData, COMPONENT_HANDLE handle) : m_eComType(eType), m_hHandle(handle), m_pData(pData){}

    void Initialize(COMPONENT_HANDLE hHandle, DATA_T* pData)
    {
        m_hHandle = hHandle;
        m_pData = pData;
    }

    bool Is_Valid() const
    {
        return m_pData != nullptr && m_hHandle.Is_Valid();
    }

    COMPONENT_TYPE  Get_ComponentType() const
    {
	    return m_eComType;
    }

    COMPONENT_HANDLE    Get_Handle()
    {
        return m_hHandle;
    }
    DataType* _Data()
    {
        return m_pData;
    }
    const DataType* _Data() const
    {
        return m_pData;
    }
protected:
    COMPONENT_HANDLE            m_hHandle{};    /* Unique identifier for version-safe access */
    DATA_T*                     m_pData{};      /* Direct pointer to the raw data in the pool */
    COMPONENT_TYPE              m_eComType = COMPONENT_TYPE::END;

};
