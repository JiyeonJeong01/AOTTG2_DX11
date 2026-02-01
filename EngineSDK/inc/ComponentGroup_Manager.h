#pragma once
#include "Base.h"

/**
 * @class CComponentGroup_Manager
 * @brief Manages multiple instances of the same component type for a single GameObject.
 */
NS_BEGIN(Engine)

class CComponentGroup_Manager : public CBase
{
private:
	CComponentGroup_Manager() = default;
	virtual ~CComponentGroup_Manager() = default;

public :
	/* Promotes a single component handle to a multi-component group. A unique Group ID, to be stored in the lower 31 bits of the component slot. */
	uint32_t				Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew);
	/* Appends an additional component handle to an existing group. */
	void					Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew);

	const COMPONENT_GROUP&	Get_Group(uint32_t iGroupID) const;
	void					Free_Group(uint32_t iGroupID);

private:
	vector<COMPONENT_GROUP>	m_Groups{ };
	vector<uint32_t>		m_FreeIndices{ };

public :
	static CComponentGroup_Manager* Create();
private:
	virtual void Free() override;
};

NS_END
