#pragma once
#include "Component_Processor.h"
#include "Component_Pool.h"

template<typename PROXY>
class CComponent_Processor_Impl : public CComponent_Processor
{
	/* Ensure that the PROXY type is derived from CComponent_Proxy_Base  at compile-time! */
	static_assert(std::is_base_of_v<CComponent_Proxy_Base<typename PROXY::DataType, PROXY>, PROXY>,
		"Error: PROXY must inherit from CComponent_Proxy_Base!");

protected:
	CComponent_Pool<PROXY> m_Pool;

public :
    COMPONENT_HANDLE Create_Component_Data() override
	{
		return m_Pool.Allocate();
	}

    void Remove_Component(COMPONENT_HANDLE hHandle) override
	{
		m_Pool.Deallocate(hHandle);
	}

    PROXY Get_Proxy(COMPONENT_HANDLE hHandle)
	{
		return m_Pool.Get_Proxy(hHandle);
	}
};
