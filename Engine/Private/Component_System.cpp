#include "Component_System.h"
#include "Component_Processor.h"
#include "ComponentGroup_Manager.h"

//===============TEST===============
#include "TestComponentASystem.h"
#include "TestComponentBSystem.h"
//==================================

IMPLEMENT_SINGLETON(CComponent_System)

HRESULT CComponent_System::Init()
{
	m_pComGroupMgr = CComponentGroup_Manager::Create();

	CTestComponentASystem* pASystem = new CTestComponentASystem();
	CTestComponentBSystem* pBSystem = new CTestComponentBSystem();
	m_pComProcessors.push_back(pASystem);
	m_pComProcessors.push_back(pBSystem);

	return S_OK;
}

void CComponent_System::Update(_float fDT)
{
	for (auto& pProcessor : m_pComProcessors)
		pProcessor->Update(fDT);
}

void CComponent_System::LateUpdate(_float fDT)
{
	// for (auto& pProcessor : m_ComProcessors)
}

void CComponent_System::FixedUpdate(_float fDT)
{
}

void CComponent_System::Render()
{
}

COMPONENT_HANDLE CComponent_System::Create_Component_By_Type(COMPONENT_TYPE eComType)
{
	return m_pComProcessors[SCAST(_uint, eComType)]->Create_Component_Data();
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
