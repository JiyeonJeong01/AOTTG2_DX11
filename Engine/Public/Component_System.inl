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

    CComponent_Processor* pBase = m_pComProcessors[iProcIdx].get();
    void* p = pBase->Get_DataPtr(eComType, handle);
    IF_NULL_RETURN_MSG_BREAK(p, TProxy(nullptr, COMPONENT_HANDLE{}), "Get_DataPtr returned null");

    return TProxy(SCAST(typename TProxy::DataType*, p), handle);
}

template <typename TProxy, typename TSpec>
void CComponent_System::Register_InitialSpecFactory(COMPONENT_TYPE eComType)
{
    IF_TRUE_RETURN_MSG_BREAK(TProxy::ComponentType != eComType, , "Wrong component type input.");
    const uint32_t iComIdx = COM_TO_INT(eComType);
    _DEBUG_ENGINE_ASSERT_MSG((m_InitialSpecFactory[iComIdx] == nullptr), "Already registered type.");

    m_InitialSpecFactory[iComIdx] = +[](CComponent_System* pSys, COMPONENT_TYPE eInType, CGameObject* pObj, const COMPONENT_SPEC_BASE* pBase)
        {

            TProxy proxy = pObj->template Add_Component<TProxy>();

            if (pBase)
            {
                /* To Keep the lambda capture-free */
                return pSys->Initialize_From_Spec(eInType, proxy.Get_Handle(), pBase);
            }
            return E_FAIL;
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
