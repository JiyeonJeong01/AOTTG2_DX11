#pragma once
#include "Component_System.h"
#include "GameObject.h"
#include "Engine_Log.h"
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

template <typename TProxy>
TProxy CComponent_System::Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle)
{
    const uint32_t iIndex = SCAST(_uint, eComType);
    if (iIndex >= SCAST(_uint, COMPONENT_TYPE::END) || !m_pComProcessors[iIndex])
    {
        _DEBUG_ERROR_BREAK("Processor not registered for this component type.");
        return TProxy(nullptr, COMPONENT_HANDLE{});
    }

    using PROCESSOR_T = typename TProxy::ProcessorType;
    PROCESSOR_T* pProcessor = static_cast<PROCESSOR_T*>(m_pComProcessors[iIndex].get());

    return pProcessor->Get_Proxy(handle);
}

template <typename TProxy, typename TSpec>
void CComponent_System::Register_Factory(COMPONENT_TYPE eComType)
{
    _DEBUG_ENGINE_ASSERT_MSG((m_factory[SCAST(_uint, eComType)] == nullptr), "Already registered type!");

    m_factory[SCAST(_uint, eComType)] = +[](CComponent_System* pSys, COMPONENT_TYPE eInType, CGameObject* pObj, const COMPONENT_SPEC_BASE* pBase)
        {

            TProxy proxy = pObj->template Add_Component<TProxy>(eInType);

            if (pBase)
            {
                /* To Keep the lambda capture-free */
                pSys->Initialize_From_Spec(eInType, proxy.Get_Handle(), pBase);
            }
        };
}

NS_END
