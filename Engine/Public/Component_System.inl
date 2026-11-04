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

    /* 모든 경로가 같은 변수를 반환해야 복사가 생기지 않는다 */
    TProxy proxy(nullptr, COMPONENT_HANDLE{});

    IF_TRUE_RETURN_MSG_BREAK((iComIdx >= COMPONENT_MAX || !m_pComProcessors[iProcIdx]),
        proxy,
        "Processor not registered for this component type.");

    CComponent_Processor* pBase = m_pComProcessors[iProcIdx].get();
    const uint32_t* pVersion = nullptr;
    void* p = pBase->Get_DataPtr(eComType, handle, &pVersion);
    IF_NULL_RETURN_MSG_BREAK(p, proxy, "Get_DataPtr returned null");

    proxy.Initialize(handle, SCAST(typename TProxy::DataType*, p));
    proxy.Bind_Version(pVersion);
    return proxy;
}

template <typename TProxy, typename TSpec>
void CComponent_System::Register_InitialSpecFactory()
{
    const uint32_t iComIdx = COM_TO_INT(TProxy::ComponentType);
    _DEBUG_ENGINE_ASSERT_MSG((m_InitialSpecFactory[iComIdx] == nullptr), "Already registered type.");

    m_InitialSpecFactory[iComIdx] = +[](CComponent_System* pSys, COMPONENT_TYPE eInType, CGameObject* pObj, const COMPONENT_SPEC_BASE* pBase)
        {

            TProxy proxy = pObj->template Add_Component<TProxy>();

            if (pBase)
            {
                /* To Keep the lambda capture-free */
                return pSys->Initialize_From_Spec(TProxy::ComponentType, proxy.Get_Handle(), pBase);
            }
            return E_FAIL;
        };
}

template <typename TProxy>
void CComponent_System::Register_BuildSpecFacotry()
{
    const uint32_t iComIdx = COM_TO_INT(TProxy::ComponentType);
    _DEBUG_ENGINE_ASSERT_MSG((m_BuildSpecFactory[iComIdx] == nullptr), "Already registered type.");

    m_BuildSpecFactory[iComIdx] = +[](CComponent_System* pSys, COMPONENT_TYPE eInType, COMPONENT_HANDLE hComponent)
        ->std::unique_ptr<COMPONENT_SPEC_BASE>
        {
            return pSys->m_pComProcessors[COM_TO_PID(eInType)]->Build_Spec(eInType, hComponent);
        };

}

template <typename TProc>
TProc* CComponent_System::Bind_Processor()
{
    PROCESSOR_ID PID = TProc::Get_Static_PID();
    return To<TProc*>(m_pComProcessors[PID_TO_INT(PID)].get());
}
NS_END
