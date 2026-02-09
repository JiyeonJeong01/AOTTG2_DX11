#include "Component_System.h"
#include "Component_Processor.h"
#include "ComponentGroup_Manager.h"

//===============TEST===============
#include "TestComponentASystem.h"
#include "TestComponentBSystem.h"
//==================================

IMPLEMENT_SINGLETON(CComponent_System)

CComponent_System::CComponent_System() = default;
CComponent_System::~CComponent_System() = default;


HRESULT CComponent_System::Initialize()
{
    m_pComGroupMgr = CComponentGroup_Manager::Create();

    m_pComProcessors.resize(SCAST(_uint, COMPONENT_TYPE::END));
    m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::TEST_A)] = CTestComponentASystem::Create();
    m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::TEST_B)] = CTestComponentBSystem::Create();

	return S_OK;
}

void CComponent_System::Update(_float fDT)
{
	for (auto& pProcessor : m_pComProcessors)
		pProcessor->Update(fDT);
}

void CComponent_System::LateUpdate(_float fDT)
{
    for (auto& pProcessor : m_pComProcessors)
    {
        if (pProcessor)
            pProcessor->Update(fDT);
    }
}

void CComponent_System::FixedUpdate(_float fDT)
{
}

void CComponent_System::Render()
{
}

COMPONENT_HANDLE CComponent_System::Create_Component_By_Type(COMPONENT_TYPE eComType)
{
    const uint32_t iIndex = SCAST(_uint, eComType);
    if (iIndex >= SCAST(_uint, COMPONENT_TYPE::END) || !m_pComProcessors[iIndex])
    {
        _DEBUG_ERROR_BREAK("Processor not registered for this component type.");
        return COMPONENT_HANDLE{};
    }
    return m_pComProcessors[iIndex]->Create_Component_Data();
}

void CComponent_System::Create_From_Spec(CGameObject* pObj, const COMPONENT_SPEC_BASE* pSpec)
{
    if (!pObj || !pSpec)
    {
        _DEBUG_ERROR_BREAK("Create from spec failed: parameter is nullptr.");
        return;
    }

    const COMPONENT_TYPE eType = pSpec->Get_Type();
    auto fn = m_factory[SCAST(_uint, eType)];
    if (!fn)
    {
        _DEBUG_ERROR_BREAK("Factory not registered for this component type.");
        return;
    }

    fn(this, eType, pObj, pSpec);
}

void CComponent_System::Remove_Component_By_Type(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle)
{
    const uint32_t iIndex = SCAST(_uint, eComType);
    if (iIndex >= SCAST(_uint, COMPONENT_TYPE::END) || !m_pComProcessors[iIndex])
    {
        _DEBUG_ERROR_BREAK("Processor not registered for this component type.");
        return ;
    }
    m_pComProcessors[iIndex]->Remove_Component(handle);
}

void CComponent_System::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pBase)
{
    if (SCAST(_uint, eComType) >= SCAST(_uint, COMPONENT_TYPE::END))
    {
        _DEBUG_ERROR_BREAK("Invalid component type");
        return;
    }

    if (FAILED(m_pComProcessors[SCAST(_uint, eComType)]->Initialize_From_Spec(handle, pBase)))
    {
        _DEBUG_ERROR_BREAK("Failed initialize with spec");
    }
}

uint32_t CComponent_System::Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew)
{
	return m_pComGroupMgr->Promote(hOld, hNew);
}

void CComponent_System::Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew) 
{
	m_pComGroupMgr->Add_To_Group(iGroupID, hNew);
}

const COMPONENT_GROUP& CComponent_System::Get_Group(uint32_t iGroupID)
{
	return m_pComGroupMgr->Get_Group(iGroupID);
}

void CComponent_System::Free_Group(uint32_t iGroupID)
{
	m_pComGroupMgr->Free_Group(iGroupID);
}
