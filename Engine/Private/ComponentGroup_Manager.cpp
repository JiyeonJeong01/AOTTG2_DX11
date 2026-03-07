#include "ComponentGroup_Manager.h"

#include "Engine_Log.h"

NS_BEGIN(Engine)

CComponentGroup_Manager::CComponentGroup_Manager()
{
}

CComponentGroup_Manager::~CComponentGroup_Manager()
{
}

uint32_t CComponentGroup_Manager::Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew)
{
	uint32_t iGroupID = 0;
	if (false == m_FreeIndices.empty())
	{
        iGroupID = m_FreeIndices.back();
		m_FreeIndices.pop_back();
	}
	else
	{
        iGroupID = SCAST(uint32_t, m_Groups.size());
		m_Groups.emplace_back();
	}

	/* Allocate space for group promotion. */
	auto& tGroup = m_Groups[iGroupID];
	tGroup.tPrimary = hOld;

	/* Prepare for reuse and append the new handle to the Extras container. */
	tGroup.tExtras.clear();
	tGroup.tExtras.push_back(hNew);

    _DEBUG_INFO("Promote to group. GroupID : [ %d ]", iGroupID);

	return iGroupID;
}

void CComponentGroup_Manager::Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew)
{
	/* Validation check for the provided Group ID. */
    IF_TRUE_RETURN_MSG_BREAK((iGroupID >= SCAST(uint32_t, m_Groups.size())), , "Invalid Group ID access in Add_To_Group.");

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

std::unique_ptr<CComponentGroup_Manager>  CComponentGroup_Manager::Create()
{
    return std::make_unique<CComponentGroup_Manager>();
}

NS_END
