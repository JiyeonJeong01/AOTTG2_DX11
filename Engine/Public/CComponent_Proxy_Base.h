#pragma once
#include "Engine_Define.h"
#include "Component_Struct.h"
#include "Object_Struct.h"

template <typename TProxy, COMPONENT_TYPE KType, uint32_t PageSize = 1024> class CComponent_Processor_Impl;

/* 핸들이 아직 같은 컴포넌트를 가리키는지 풀에 확인 (CComponent_Proxy_Base.cpp). 해제되었거나 칸이 재사용되었으면 false */
ENGINE_DLL bool Component_Is_Alive(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const void* pData);

/* 컴포넌트 풀(페이지)이 살아 있는 동안만 true (Component_System.cpp). 종료 중 캐싱한 버전 주소를 읽지 않기 위한 표시 */
ENGINE_DLL extern bool g_bComponent_Pools_Alive;

/**
 * Base class for all Component Proxies.
 * @tparam DATA_T  The raw struct containing component data (e.g., TRANSFORM_DATA)
 */
template <typename DATA_T, COMPONENT_TYPE KType>
class CComponent_Proxy_Base
{
public:
    /* Expose types for the Component System's type traits and Pool allocation */
    using DataType = DATA_T;
    static constexpr COMPONENT_TYPE ComponentType = KType;

    CComponent_Proxy_Base() = default;
    explicit CComponent_Proxy_Base(COMPONENT_TYPE eType) : m_eComType(eType) {}
    CComponent_Proxy_Base(DataType* pData, COMPONENT_HANDLE handle) : m_hHandle(handle), m_pData(pData){}

    void Initialize(COMPONENT_HANDLE hHandle, DataType* pData) noexcept
    {
        m_hHandle = hHandle;
        m_pData = pData;
        m_pVersion = nullptr;
    }

    /* 풀이 Proxy를 만들 때 이 칸의 버전 주소를 넘겨준다 */
    void Bind_Version(const uint32_t* pVersion) noexcept
    {
        m_pVersion = pVersion;
    }

    /*
        이 Proxy가 만들어질 때 가리키던 그 컴포넌트가 아직 살아 있다.
        해제하면 칸의 버전이 오르고 버전을 다 쓴 칸은 재사용하지 않으므로, 버전이 같으면 같은 컴포넌트다.
        버전 주소 없이 만들어진 Proxy는 풀에 다시 조회한다 (느린 경로).
    */
    bool Is_Valid() const noexcept
    {
        if (m_pData == nullptr || !m_hHandle.Is_Valid())
            return false;

        if (m_pVersion)
            return g_bComponent_Pools_Alive && *m_pVersion == m_hHandle.Get_Version();

        return Component_Is_Alive(m_eComType, m_hHandle, m_pData);
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
    const uint32_t*             m_pVersion{};   /* 풀 페이지의 이 칸 버전 (Is_Valid 빠른 경로) */
    COMPONENT_TYPE              m_eComType = KType;

};
