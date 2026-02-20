#pragma once
#include "Component_Processor.h"
#include "Component_Pool.h"

template<typename TProxy, COMPONENT_TYPE KType>
class CComponent_Processor_Impl : public CComponent_Processor
{
	/* Ensure that the TProxy type is derived from CComponent_Proxy_Base  at compile-time! */
	static_assert(std::is_base_of_v<CComponent_Proxy_Base<typename TProxy::DataType, TProxy, TProxy::ComponentType>, TProxy>,
		"Error: TProxy must inherit from CComponent_Proxy_Base!");

public:
    CComponent_Processor_Impl() = default;
    virtual ~CComponent_Processor_Impl() = default;

    CComponent_Processor_Impl(const CComponent_Processor_Impl&) = delete;
    CComponent_Processor_Impl& operator=(const CComponent_Processor_Impl&) = delete;
    CComponent_Processor_Impl(CComponent_Processor_Impl&&) noexcept = default;
    CComponent_Processor_Impl& operator=(CComponent_Processor_Impl&&) noexcept = default;

protected:
	CComponent_Pool<TProxy> m_Pool;

public :
    COMPONENT_HANDLE Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject) override
	{
        IF_TRUE_RETURN_MSG_BREAK(eComType != KType, COMPONENT_HANDLE{}, "Wrong component type for this processor.");

		COMPONENT_HANDLE hComponent = m_Pool.Allocate();
        auto pData = m_Pool.Get_Data_By_Handle(hComponent);
        pData->hObject = hObject;

        Initialize_Component_Data(hComponent);

        return hComponent;
	}

    void Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override
	{
        IF_TRUE_RETURN_MSG_BREAK(eComType != KType, , "Wrong component type for this processor.");

		m_Pool.Deallocate(hComponent);
	}

    TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
	{
        IF_TRUE_RETURN_MSG_BREAK(eComType != KType, TProxy{}, "Wrong component type for this processor.");

		return m_Pool.Get_Proxy(hComponent);
	}

protected :
    virtual void Initialize_Component_Data(COMPONENT_HANDLE hComponent) {};
    virtual HRESULT Initialize_From_Spec_Impl(COMPONENT_HANDLE, const COMPONENT_SPEC_BASE*) { return S_OK; }
};
