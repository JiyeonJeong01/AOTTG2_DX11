#pragma once
#include "Component_Processor_Impl.h"
#include "Render_Struct.h"

NS_BEGIN(Engine)

class CGameObject;
class CComponent_Processor;
class CComponentGroup_Manager;
struct tagDrawCmd;
typedef struct tagDrawCmd DRAW_CMD;

class ENGINE_DLL CComponent_System final
{
    DECLARE_SINGLETON(CComponent_System)
public :
	HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void Update(_float fDT);
	void LateUpdate(_float fDT);
	void FixedUpdate(_float fDT);
    void Build_RenderQueue(vector<DRAW_CMD>& cmds);
	void Render();

    void Update_Debug();

public : /* Component Processor */
    /* Create component with default value */
	COMPONENT_HANDLE Create_Component_By_Type(COMPONENT_TYPE eComType, OBJECT_HANDLE hGameObject);

    /* Build spec with exsiting component handle */
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec_By_Type(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent);

    /* Create component with spec value  */
    HRESULT Create_Component_From_Spec(CGameObject* pObj, const COMPONENT_SPEC_BASE* pSpec);

    void Remove_Component_By_Type(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle);
    void Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, _bool bEnable);

    void Get_Component_Handle_By_Type(COMPONENT_TYPE eComType, OBJECT_HANDLE hObj, vector<COMPONENT_HANDLE>& outHandles);

public : /* CComponentGroup_Manager */
	uint32_t				Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew);
	void					Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew);
	const COMPONENT_GROUP&	Get_Group(uint32_t iGroupID);
	void					Free_Group(uint32_t iGroupID);

private :
    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pBase);

private :

    ID3D11Device*           m_pDevice{};
    ID3D11DeviceContext*    m_pContext{};
    vector<std::unique_ptr<CComponent_Processor>>	m_pComProcessors{ };

    std::unique_ptr<CComponentGroup_Manager> 		m_pComGroupMgr{ };
    static std::array <PROCESSOR_ID, COMPONENT_MAX> m_TypeToProcessorIndex;

    using I_FACTORY_FN = HRESULT(*)(CComponent_System*, COMPONENT_TYPE, CGameObject*, const COMPONENT_SPEC_BASE*);
    I_FACTORY_FN  m_InitialSpecFactory[COMPONENT_MAX]{};

    using B_FACTORY_FN = std::unique_ptr<COMPONENT_SPEC_BASE>(*)(CComponent_System*, COMPONENT_TYPE, COMPONENT_HANDLE);
    B_FACTORY_FN  m_BuildSpecFactory[COMPONENT_MAX]{};

public :
    template <typename TProxy>
    TProxy Get_Proxy(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle);

    /* Register a callback for Initialize_From_Spec() */
    template <typename TProxy, typename TSpec>
    void Register_InitialSpecFactory();

    /* Register a callback for Build_Spec_By_Type */
    template<typename TProxy>
    void Register_BuildSpecFacotry();

    template<typename TPRoc>
    TPRoc* Bind_Processor();
};


NS_END

#include "Component_System.inl"
