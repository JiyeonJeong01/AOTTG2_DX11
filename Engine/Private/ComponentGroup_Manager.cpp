#include "ComponentGroup_Manager.h"

#include "Engine_Log.h"

NS_BEGIN(Engine)

uint32_t CComponentGroup_Manager::Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew)
{
	uint32_t iID = 0;
	if (false == m_FreeIndices.empty())
	{
		iID = m_FreeIndices.back();
		m_FreeIndices.pop_back();
	}
	else
	{
		iID = SCAST(uint32_t, m_Groups.size());
		m_Groups.emplace_back();
	}

	/* Allocate space for group promotion. */
	auto& tGroup = m_Groups[iID];
	tGroup.tPrimary = hOld;

	/* Prepare for reuse and append the new handle to the Extras container. */
	tGroup.tExtras.clear();
	tGroup.tExtras.push_back(hNew);

	return iID;
}

void CComponentGroup_Manager::Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew)
{
	/* Validation check for the provided Group ID. */
	if (iGroupID >= SCAST(uint32_t, m_Groups.size()))
	{
		_DEBUG_ERROR_BREAK("Invalid Group ID access in Add_To_Group.");
		return;
	}

	m_Groups[iGroupID].tExtras.push_back(hNew);
}

const COMPONENT_GROUP& CComponentGroup_Manager::Get_Group(uint32_t iGroupID) const
{
	return m_Groups[iGroupID];
}

void CComponentGroup_Manager::Free_Group(uint32_t iGroupID)
{
	m_Groups[iGroupID].tExtras.clear();
	m_FreeIndices.push_back(iGroupID);
}

CComponentGroup_Manager* CComponentGroup_Manager::Create()
{
	CComponentGroup_Manager* pInstance = new CComponentGroup_Manager();

	return pInstance;
}

void CComponentGroup_Manager::Free()
{
	CBase::Free();
}

NS_END
