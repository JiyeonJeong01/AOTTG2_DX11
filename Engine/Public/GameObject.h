#pragma once
#include "Base.h"
#include "Component_System.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

class ENGINE_DLL CGameObject : public CBase, public LABEL
{
protected:
	CGameObject();
    CGameObject(std::string str);
    CGameObject(const CGameObject& Clone);
	~CGameObject() override = default;

public :
	/* 디버깅 등에 사용될 수 있다. */
	void Render();

protected:
    /* Components */
    map<COMPONENT_TYPE, COMPONENT_HANDLE> m_ComponentHandles;

    /* Hierarchy */
	uint32_t                m_iComponentSlots[MAX_COMPONENT] = { 0, };
    CGameObject*            m_pParent{};
    vector<CGameObject*>    m_pChildren{};

    /* etc */
    _bool                   m_bActive{ true };

public :
    /* Components */
    template <typename PROXY>
	PROXY Add_Component(COMPONENT_TYPE eComType);
	template <typename PROXY>
	PROXY Get_Component(COMPONENT_TYPE eComType);
	template <typename PROXY>
	vector<PROXY> Get_Components(COMPONENT_TYPE eComType);

    GAMEOBJECT_META&        Access_Meta();
    const GAMEOBJECT_META&  Access_Meta() const;

    /* Hierarchy */
    HRESULT                     Set_Parent(CGameObject* pNewParent);
    CGameObject*                Get_Parent();
    HRESULT                     Add_Child(CGameObject* pNewChild);
    const std::vector<CGameObject*>&  Get_Children() const;

    /* Layer */
    GAMEOBJECT_META             m_tMeta{};

    /* etc */
    void                        Set_Active(_bool bActive) { m_bActive = bActive; }
    _bool                       Get_Active() const        { return m_bActive; }

protected :
    /* Components */
	COMPONENT_HANDLE Decode_Slot(uint32_t iSlotData) const;

public :
	/* 임시로 열어둠 */
	static CGameObject* Create(uint32_t iLayer = Layer::DEFAULT_LAYER, string strName = "GameObject", CGameObject* pParent = nullptr);
    CGameObject* Clone();

private:
	void Free() override;
};

template <typename PROXY>
PROXY CGameObject::Add_Component(COMPONENT_TYPE eComType)
{
	COMPONENT_HANDLE hNewHandle = SYS_COM->Create_Component_By_Type(eComType);

	if (0 == hNewHandle.iHandle)
	{
		_DEBUG_ERROR_BREAK("GameObject can't add component!");
		return PROXY{};
	}

	uint32_t& iSlotData = m_iComponentSlots[SCAST(_uint, eComType)];
	if (0 == iSlotData) /* 처음 추가되는  컴포넌트 */
	{
		iSlotData = hNewHandle.iHandle;
	}
	else if ((iSlotData & ComponentConfig::GROUP_FLAG) == 0) /* 그룹으로 승격되는 경우 */
	{
		COMPONENT_HANDLE hOld;
		hOld.iHandle = iSlotData & ComponentConfig::DATA_MASK;

		const uint32_t iGroupID = SYS_COM->Promote(hOld, hNewHandle);
		iSlotData = iGroupID | ComponentConfig::GROUP_FLAG;
	}
	else
	{
		const uint32_t iGroupID = iSlotData & ComponentConfig::DATA_MASK; /* 기존 그룹에 추가되는 경우 */
		SYS_COM->Add_To_Group(iGroupID, hNewHandle);
	}

    return SYS_COM->Get_Proxy<PROXY>(eComType, hNewHandle);
}

template <typename PROXY>
PROXY CGameObject::Get_Component(COMPONENT_TYPE eComType)
{
	const uint32_t iSlotData = m_iComponentSlots[SCAST(_uint, eComType)];

	if (iSlotData == 0)
	{
		_DEBUG_ERROR_BREAK("GameObject can't get such component!");
		return PROXY{};
	}

	COMPONENT_HANDLE handle;
	if ((iSlotData & ComponentConfig::GROUP_FLAG) == 0)  /* 단일 컴포넌트 */
	{
		handle.iHandle = iSlotData;
	}
	else /* 그룹 컴포넌트 */
	{
		handle = SYS_COM->Get_Group((iSlotData & ComponentConfig::DATA_MASK)).tPrimary;
	}

	/* ====== TODO : 안정화시 바로 return ====== */
	PROXY component = SYS_COM->Get_Proxy<PROXY>(eComType, handle);
	return component;

	// return SYS_COM->Get_Proxy<T>(eComType, handle);
}

template <typename PROXY>
vector<PROXY> CGameObject::Get_Components(COMPONENT_TYPE eComType)
{
	const uint32_t iSlotData = m_iComponentSlots[SCAST(_uint, eComType)];

	if (iSlotData == 0)
	{
		_DEBUG_ERROR_BREAK("GameObject can't get such component!");
		return vector<PROXY>{};
	}

	if ((iSlotData & ComponentConfig::GROUP_FLAG) == 0) /* 단일 컴포넌트 */
	{
		COMPONENT_HANDLE h;
		h.iHandle = iSlotData;
		return { SYS_COM->Get_Proxy<PROXY>(eComType, h) };
	}
	else /* 그룹 컴포넌트 */
	{
		const uint32_t iGroupID = iSlotData & ComponentConfig::DATA_MASK;
		const auto& tGroup = SYS_COM->Get_Group(iGroupID);

		vector<PROXY> components;
		components.reserve(tGroup.tExtras.size() + 1);
		components.push_back(SYS_COM->Get_Proxy<PROXY>(eComType, tGroup.tPrimary));

		for (auto& h : tGroup.tExtras)
			components.push_back(SYS_COM->Get_Proxy<PROXY>(eComType, h));

		return components;
	}
}

NS_END
