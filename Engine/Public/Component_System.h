#pragma once
#include "Base.h"
#include "Component_Processor_Impl.h"

NS_BEGIN(Engine)
class CComponent_Processor;
class CComponentGroup_Manager;

class ENGINE_DLL CComponent_System : public CBase
{
	DECLARE_SINGLETON(CComponent_System)
private :
	CComponent_System() = default;
	~CComponent_System() override = default;

public :
	HRESULT Initialize();
	void Update(_float fDT);
	void LateUpdate(_float fDT);
	void FixedUpdate(_float fDT);
	void Render();

public : /* Component Processor */
	COMPONENT_HANDLE Create_Component_By_Type(COMPONENT_TYPE eComType);
	template <typename TProxy>
	TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle);

public : /* CComponentGroup_Manager */
	uint32_t				Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew);
	void					Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew);
	const COMPONENT_GROUP&	Get_Group(uint32_t iGroupID);
	void					Free_Group(uint32_t iGroupID);

private :
	vector<CComponent_Processor*>	m_pComProcessors{ };
	CComponentGroup_Manager*		m_pComGroupMgr{ };
};

template <typename TProxy>
TProxy CComponent_System::Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle)
{
	using PROCESSOR_T = typename TProxy::ProcessorType;

	PROCESSOR_T* pProcessor = static_cast<PROCESSOR_T*>(m_pComProcessors[static_cast<uint32_t>(eComType)]);

	return pProcessor->Get_Proxy(handle);
}

NS_END
