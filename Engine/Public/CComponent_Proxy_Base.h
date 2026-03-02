#pragma once
#include "Engine_Define.h"
#include "Component_Struct.h"
#include "Object_Struct.h"

template <typename TProxy, COMPONENT_TYPE KType> class CComponent_Processor_Impl;

/**
 * Base class for all Component Proxies.
 * Uses CRTP (Curiously Recurring Template Pattern) to bind Data and Proxy types.
 * @tparam DATA_T  The raw struct containing component data (e.g., CTransform_Data)
 * @tparam TProxy_T The actual derived proxy class (e.g., CTransform_Proxy)
 */
template <typename DATA_T, typename TProxy_T, COMPONENT_TYPE KType>
class CComponent_Proxy_Base
{
public:
    /* Expose types for the Component System's type traits and Pool allocation */
    using DataType = DATA_T;
    using ProcessorType = CComponent_Processor_Impl<TProxy_T, KType>;
    static constexpr COMPONENT_TYPE ComponentType = KType;

    CComponent_Proxy_Base() = default;
    explicit CComponent_Proxy_Base(COMPONENT_TYPE eType) : m_eComType(eType) {}
    CComponent_Proxy_Base(DataType* pData, COMPONENT_HANDLE handle) : m_hHandle(handle), m_pData(pData){}
    virtual ~CComponent_Proxy_Base() {}


    void Initialize(COMPONENT_HANDLE hHandle, DataType* pData) noexcept
    {
        m_hHandle = hHandle;
        m_pData = pData;
    }

    bool Is_Valid() const noexcept
    {
        return m_pData != nullptr && m_hHandle.Is_Valid();
    }

    void Set_Enable(_bool bEnable)
    {
        m_pData->bEnable = bEnable;
    }

    _bool Get_Enable()const noexcept
    {
        return m_pData->bEnable;
    }

    COMPONENT_TYPE  Get_ComponentType() const noexcept
    {
	    return m_eComType;
    }

    COMPONENT_HANDLE    Get_Handle() const noexcept
    {
        return m_hHandle;
    }
    DataType* _Data() noexcept { return m_pData; }
    const DataType* _Data() const noexcept { return m_pData; }

    DataType* operator->() noexcept { return m_pData; }
    const DataType* operator->() const noexcept { return m_pData; }
protected:
    COMPONENT_HANDLE            m_hHandle{};    /* Unique identifier for version-safe access */
    DataType*                   m_pData{};      /* Direct pointer to the raw data in the pool */
    COMPONENT_TYPE              m_eComType = KType;

};
