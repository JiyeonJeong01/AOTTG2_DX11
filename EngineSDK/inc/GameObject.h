#pragma once
#include "Base.h"
#include "Component_System.h"
#include "GameObject_System.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

class CGameObject_System;

class ENGINE_DLL CGameObject : public CBase, public LABEL
{
    friend class CGameObject_System;

protected:
    CGameObject(uint32_t iPoolIndex);
    ~CGameObject() override = default;

public:
    void Render();

protected:
    GAMEOBJECT_HANDLE     m_hSelf{};

public:
    /* Components */
    template <typename TProxy>
    TProxy Add_Component(COMPONENT_TYPE eComType);

    template <typename TProxy>
    TProxy Get_Component(COMPONENT_TYPE eComType);

    template <typename TProxy>
    std::vector<TProxy> Get_Components(COMPONENT_TYPE eComType);

    /* Hierarchy */
    CGameObject*                Get_Parent();
    HRESULT                     Set_Parent(CGameObject* pNewParent);
    HRESULT                     Add_Child(CGameObject* pChild);
    HRESULT                     Remove_Child(CGameObject* pChild);

    std::vector<CGameObject*>   Get_Children() const;

    GAMEOBJECT_HANDLE           Get_Handle() const;
    _bool                       IsValid() const;

    /* etc */
    void                        Set_Active(_bool bActive);
    _bool                       Get_Active() const;

private:
    void Add_Child_Inner(GAMEOBJECT_HANDLE hChild);
    void Remove_Child_Inner(GAMEOBJECT_HANDLE hChild);

public:
    static CGameObject* Create(uint32_t iLayer = Layer::DEFAULT_LAYER, std::string strName = "GameObject", CGameObject* pParent = nullptr);
    CGameObject* Clone();

private:
    void Free() override;
};

template <typename TProxy>
TProxy CGameObject::Add_Component(COMPONENT_TYPE eComType)
{
    if (!IsValid())
        return TProxy{};

    GAMEOBJECT_DATA& data = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);
    COMPONENT_HANDLE hNewHandle = SYS_COMPONENT->Create_Component_By_Type(eComType);

    /* A handle value of 0 means an invalid component */
    if (hNewHandle.iHandle == ComponentConfig::INVALID_COMPONENT_SLOT)
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
    else if ((iSlotData & ComponentConfig::GROUP_FLAG) == 0)
    {
        COMPONENT_HANDLE hOld;
        hOld.iHandle = iSlotData & ComponentConfig::DATA_MASK;

        const uint32_t iGroupID = SYS_COMPONENT->Promote(hOld, hNewHandle);
        iSlotData = iGroupID | ComponentConfig::GROUP_FLAG;
    }
    else
    {
        const uint32_t iGroupID = iSlotData & ComponentConfig::DATA_MASK;
        SYS_COMPONENT->Add_To_Group(iGroupID, hNewHandle);
    }

    return SYS_COMPONENT->Get_Proxy<TProxy>(eComType, hNewHandle);
}

template <typename TProxy>
TProxy CGameObject::Get_Component(COMPONENT_TYPE eComType)
{
    if (!IsValid())
        return TProxy{};

    const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);

    const uint32_t iSlotData = data.iComponentSlots[SCAST(_uint, eComType)];

    /* A slot data value of 0 means the component does not exist */
    if (iSlotData == ComponentConfig::INVALID_COMPONENT_SLOT)
    {
        _DEBUG_ERROR_BREAK("INVALID COMPONENT HANDLE : GameObject can't get such component!");
        return TProxy{};
    }

    COMPONENT_HANDLE handle;
    if ((iSlotData & ComponentConfig::GROUP_FLAG) == 0) 
    {
        handle.iHandle = iSlotData;
    }
    else
    {
        handle = SYS_COMPONENT->Get_Group((iSlotData & ComponentConfig::DATA_MASK)).tPrimary;
    }

    /* ====== TODO : 안정화시 바로 return ====== */
    TProxy component = SYS_COMPONENT->Get_Proxy<TProxy>(eComType, handle);
    return component;

    // return SYS_COMPONENT->Get_Proxy<T>(eComType, handle);
}

template <typename TProxy>
std::vector<TProxy> CGameObject::Get_Components(COMPONENT_TYPE eComType)
{
    if (!IsValid())
        return std::vector<TProxy>{};

    const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);

    const uint32_t iSlotData = data.iComponentSlots[SCAST(_uint, eComType)];

    /* A slot data value of 0 means the component does not exist */
    if (iSlotData == ComponentConfig::INVALID_COMPONENT_SLOT)
    {
        _DEBUG_INFO("GameObject can't get such component!");
        return std::vector<TProxy>{};
    }

    if ((iSlotData & ComponentConfig::GROUP_FLAG) == 0)
    {
        COMPONENT_HANDLE h;
        h.iHandle = iSlotData;
        return { SYS_COMPONENT->Get_Proxy<TProxy>(eComType, h) };
    }
    else
    {
        const uint32_t iGroupID = iSlotData & ComponentConfig::DATA_MASK;
        const auto& tGroup = SYS_COMPONENT->Get_Group(iGroupID);

        std::vector<TProxy> components;
        components.reserve(tGroup.tExtras.size() + 1);
        components.push_back(SYS_COMPONENT->Get_Proxy<TProxy>(eComType, tGroup.tPrimary));

        for (auto& h : tGroup.tExtras)
            components.push_back(SYS_COMPONENT->Get_Proxy<TProxy>(eComType, h));

        return components;
    }
}

NS_END
