#pragma once
#include "Base.h"
#include "Component_Processor_Impl.h"

NS_BEGIN(Engine)
class CGameObject;
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
    void Create_From_Spec(CGameObject* pObj, const COMPONENT_SPEC_BASE* pSpec);

	template <typename TProxy>
	TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle);

    template <typename TProxy, typename TSpec>
    void Register_Factory(COMPONENT_TYPE eComType);

public : /* CComponentGroup_Manager */
	uint32_t				Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew);
	void					Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew);
	const COMPONENT_GROUP&	Get_Group(uint32_t iGroupID);
	void					Free_Group(uint32_t iGroupID);

private :
    void Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pBase);

private :
	vector<CComponent_Processor*>	m_pComProcessors{ };
	CComponentGroup_Manager*		m_pComGroupMgr{ };

    using FACTORY_FN = void(*)(CComponent_System*, COMPONENT_TYPE, CGameObject*, const COMPONENT_SPEC_BASE*);
    FACTORY_FN  m_factory[SCAST(_uint, COMPONENT_TYPE::END)]{};
};

NS_END

#include "Component_System.inl"
