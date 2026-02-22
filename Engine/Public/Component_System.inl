#pragma once
#include "Component_System.h"
#include "GameObject.h"
#include "Engine_Log.h"
#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

template <typename TProxy>
TProxy CComponent_System::Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle)
{
    const uint32_t iComIdx = COM_TO_INT(eComType);
    const uint32_t iProcIdx = COM_TO_PID(eComType);

    IF_TRUE_RETURN_MSG_BREAK((iComIdx >= COMPONENT_MAX || !m_pComProcessors[iProcIdx]),
        TProxy(nullptr, COMPONENT_HANDLE{}),
        "Processor not registered for this component type.");

    using PROCESSOR_T = typename TProxy::ProcessorType;
    PROCESSOR_T* pProcessor = static_cast<PROCESSOR_T*>(m_pComProcessors[iProcIdx].get());

    return pProcessor->Get_Proxy(eComType, handle);
}

template <typename TProxy, typename TSpec>
void CComponent_System::Register_InitialSpecFactory(COMPONENT_TYPE eComType)
{
    IF_TRUE_RETURN_MSG_BREAK(TProxy::ComponentType != eComType, , "Wrong component type input.");
    const uint32_t iComIdx = COM_TO_INT(eComType);
    _DEBUG_ENGINE_ASSERT_MSG((m_InitialSpecFactory[iComIdx] == nullptr), "Already registered type.");

    m_InitialSpecFactory[iComIdx] = +[](CComponent_System* pSys, COMPONENT_TYPE eInType, CGameObject* pObj, const COMPONENT_SPEC_BASE* pBase)
        {

            TProxy proxy = pObj->template Add_Component<TProxy>(eInType);

            if (pBase)
            {
                /* To Keep the lambda capture-free */
                pSys->Initialize_From_Spec(eInType, proxy.Get_Handle(), pBase);
            }
        };
}

template <typename TProxy>
void CComponent_System::Register_BuildSpecFacotry(COMPONENT_TYPE eComType)
{
    IF_TRUE_RETURN_MSG_BREAK(TProxy::ComponentType != eComType, , "Wrong component type input.");
    const uint32_t iComIdx = COM_TO_INT(eComType);
    _DEBUG_ENGINE_ASSERT_MSG((m_BuildSpecFactory[iComIdx] == nullptr), "Already registered type.");

    m_BuildSpecFactory[iComIdx] = +[](CComponent_System* pSys, COMPONENT_TYPE eInType, COMPONENT_HANDLE hComponent)
        ->std::unique_ptr<COMPONENT_SPEC_BASE>
        {
            return pSys->m_pComProcessors[COM_TO_PID(eInType)]->Build_Spec(eInType, hComponent);
        };

}

NS_END
