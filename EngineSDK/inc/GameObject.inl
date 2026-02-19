#pragma once
#include "GameObject.h"
#include "Component_System.h"

template <typename TProxy>
TProxy CGameObject::Add_Component(COMPONENT_TYPE eComType)
{
    if (!Is_Valid())
        return TProxy{};

    if (!Component::Is_Multi_Allowed(eComType))
    {
        auto existing = Get_Component<TProxy>(eComType);
        if (existing.Is_Valid())
            return existing;
    }

    GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);
    COMPONENT_HANDLE hNewHandle = SYS_COMPONENT.Create_Component_By_Type(eComType, m_hSelf);

    /* A handle value of 0 means an invalid component */
    if (hNewHandle.iHandle == Component::INVALID_COMPONENT_SLOT)
    {
        _DEBUG_ERROR_BREAK("INVALID COMPONENT HANDLE : GameObject can't add component!");
        return TProxy{};
    }

    uint32_t& iSlotData = data.iComponentSlots[SCAST(_uint, eComType)];
    /* A slot data value of 0 means an empty slot */
    if (0 == iSlotData)
    {
        iSlotData = hNewHandle.iHandle;
    }
    else if ((iSlotData & Component::GROUP_FLAG) == 0)
    {
        COMPONENT_HANDLE hOld;
        hOld.iHandle = iSlotData & Component::DATA_MASK;

        const uint32_t iGroupID = SYS_COMPONENT.Promote(hOld, hNewHandle);
        iSlotData = iGroupID | Component::GROUP_FLAG;
    }
    else
    {
        const uint32_t iGroupID = iSlotData & Component::DATA_MASK;
        SYS_COMPONENT.Add_To_Group(iGroupID, hNewHandle);
    }

    Component::COMPONENT_MASK mask = Get_ComponentMask();
    mask |= Component::Component_Bit(eComType);
    Set_ComponentMask(mask);

    return SYS_COMPONENT.Get_Proxy<TProxy>(eComType, hNewHandle);
}

template <typename TProxy>
TProxy CGameObject::Get_Component(COMPONENT_TYPE eComType)
{
    if (!Is_Valid())
        return TProxy{};

    const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    const uint32_t iSlotData = data.iComponentSlots[SCAST(_uint, eComType)];

    /* A slot data value of 0 means the component does not exist */
    if (iSlotData == Component::INVALID_COMPONENT_SLOT)
    {
        _DEBUG_INFO("INVALID COMPONENT HANDLE : GameObject can't get such component!");
        return TProxy{};
    }

    COMPONENT_HANDLE handle;
    if ((iSlotData & Component::GROUP_FLAG) == 0)
    {
        handle.iHandle = iSlotData;
    }
    else
    {
        handle = SYS_COMPONENT.Get_Group((iSlotData & Component::DATA_MASK)).tPrimary;
    }

    /* ====== TODO : 안정화시 바로 return ====== */
    TProxy component = SYS_COMPONENT.Get_Proxy<TProxy>(eComType, handle);
    return component;

    // return SYS_COMPONENT->Get_Proxy<T>(eComType, handle);
}

template <typename TProxy>
std::vector<TProxy> CGameObject::Get_Components(COMPONENT_TYPE eComType)
{
    if (!Is_Valid())
        return std::vector<TProxy>{};

    const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    const uint32_t iSlotData = data.iComponentSlots[SCAST(_uint, eComType)];

    /* A slot data value of 0 means the component does not exist */
    if (iSlotData == Component::INVALID_COMPONENT_SLOT)
    {
        _DEBUG_INFO("GameObject can't get such component!");
        return std::vector<TProxy>{};
    }

    if ((iSlotData & Component::GROUP_FLAG) == 0)
    {
        COMPONENT_HANDLE h;
        h.iHandle = iSlotData;
        return { SYS_COMPONENT.Get_Proxy<TProxy>(eComType, h) };
    }
    else
    {
        const uint32_t iGroupID = iSlotData & Component::DATA_MASK;
        const auto& tGroup = SYS_COMPONENT.Get_Group(iGroupID);

        std::vector<TProxy> components;
        components.reserve(tGroup.tExtras.size() + 1);
        components.push_back(SYS_COMPONENT.Get_Proxy<TProxy>(eComType, tGroup.tPrimary));

        for (auto& h : tGroup.tExtras)
            components.push_back(SYS_COMPONENT.Get_Proxy<TProxy>(eComType, h));

        return components;
    }
}
