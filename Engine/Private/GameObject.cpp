#include "GameObject.h"
#include "GameObject_System.h"

NS_BEGIN(Engine)

static GAMEOBJECT_DATA g_DummyData;

CGameObject::CGameObject(uint32_t iPoolIndex)
    : tagLabel("GameObject")
{
    m_hSelf = OBJECT_HANDLE(iPoolIndex, 0, false);
}

CGameObject::~CGameObject()
{

}

void CGameObject::Render()
{
    if (!Is_Valid())
        return;
}

void CGameObject::Remove_Components(COMPONENT_TYPE eComType)
{
    if (!Is_Valid())
        return;

    const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    const uint32_t& iSlotData = data.iComponentSlots[SCAST(_uint, eComType)];

    if ((iSlotData & Component::GROUP_FLAG) == 0)
    {
        COMPONENT_HANDLE h;
        h.iHandle = iSlotData;
        SYS_COMPONENT.Remove_Component_By_Type(eComType, h);
    }
    else
    {
        const uint32_t iGroupID = iSlotData & Component::DATA_MASK;
        const auto& tGroup = SYS_COMPONENT.Get_Group(iGroupID);

        SYS_COMPONENT.Remove_Component_By_Type(eComType, tGroup.tPrimary);

        for (auto& h : tGroup.tExtras)
            SYS_COMPONENT.Remove_Component_By_Type(eComType, h);

        SYS_COMPONENT.Free_Group(iGroupID);
    }
}


void CGameObject::Remove_All_Components()
{
    if (!Is_Valid())
        return;

    GAMEOBJECT_DATA& tMyData = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    _int iTotalCom = SCAST(_int, COMPONENT_MAX);
    for (_int i = 0; i < iTotalCom; ++i)
    {
        COMPONENT_TYPE eType = SCAST(COMPONENT_TYPE, i);

        const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);
        if (data.iComponentSlots[i] == 0)
            continue;

        Remove_Components(eType);
    }

}

CGameObject* CGameObject::Get_Parent()
{
    if (!Is_Valid())
        return nullptr;

    OBJECT_HANDLE hParent = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf).hParent;

    if (!hParent.Is_Valid())
        return nullptr;

    return SYS_GAMEOBJECT.Get_Wrapper(hParent);
}

HRESULT CGameObject::Set_Parent(CGameObject* pNewParent)
{
    if (!Is_Valid())
        return E_FAIL;

    if (pNewParent == this)
        return E_FAIL;

    OBJECT_HANDLE hNewParent = pNewParent ? pNewParent->Get_Handle() : OBJECT_HANDLE{};

    GAMEOBJECT_DATA& tMyData = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    /* Already has the same parent; no changes needed. */
    if (tMyData.hParent == hNewParent)
        return S_OK;

    /* Remove this GameObject from the old parent's children list. */
    if (tMyData.hParent.Is_Valid())
    {
        CGameObject* pOldParentWrapper = SYS_GAMEOBJECT.Get_Wrapper(tMyData.hParent);
        if (pOldParentWrapper)
        {
            pOldParentWrapper->Remove_Child_Inner(m_hSelf);
        }
    }

    /* Register this GameObject to the new parent's children list. */
    tMyData.hParent = hNewParent;
    if (hNewParent.Is_Valid())
    {
        if (pNewParent)
        {
            pNewParent->Add_Child_Inner(m_hSelf);
        }
    }

    return S_OK;
}

HRESULT CGameObject::Add_Child(CGameObject* pChild)
{
    if (!pChild)
        return E_FAIL;

    return pChild->Set_Parent(this);
}

HRESULT CGameObject::Remove_Child(CGameObject* pChild)
{
    if (!pChild)
        return E_FAIL;

    return pChild->Set_Parent(nullptr);
}

void CGameObject::Add_Child_Inner(OBJECT_HANDLE hChild)
{
    GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    auto it = std::find(data.hChildren.begin(), data.hChildren.end(), hChild);
    if (it == data.hChildren.end())
    {
        data.hChildren.push_back(hChild);
    }
}

void CGameObject::Remove_Child_Inner(OBJECT_HANDLE hChild)
{
    GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    auto it = std::find(data.hChildren.begin(), data.hChildren.end(), hChild);
    if (it != data.hChildren.end())
    {
        data.hChildren.erase(it);
    }
}

std::vector<CGameObject*> CGameObject::Get_Children() const
{
    std::vector<CGameObject*> result;

    const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);
    result.reserve(data.hChildren.size());

    for (const auto& hChild : data.hChildren)
    {
        CGameObject* pChild = SYS_GAMEOBJECT.Get_Wrapper(hChild);
        if (pChild && pChild->Is_Valid())
            result.push_back(pChild);
    }

    return result;
}

OBJECT_HANDLE CGameObject::Get_Handle() const
{
    return m_hSelf;
}

_bool CGameObject::Is_Valid() const
{

    /* TODO ==================================================== */
    /* TODO : MAKE SURE TO MINIMIZE THE NUMBER OF CALLS TO THIS! */
    /* TODO ==================================================== */
    return SYS_GAMEOBJECT.Is_Valid_Handle(m_hSelf);
}

void CGameObject::Set_Enable(_bool bActive)
{
    SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf).bEnable = bActive;

    Component::COMPONENT_MASK mask = Get_ComponentMask();

    for (_uint j = 0; j < COMPONENT_MAX; ++j)
    {
        const COMPONENT_TYPE eType = INT_TO_COM(j);

        if ((mask & Component::To_Bit(eType)) == 0)
            continue;

        vector<COMPONENT_HANDLE> hComponents;
        SYS_COMPONENT.Get_Component_Handle_By_Type(eType, Get_Handle(), hComponents);

        for (auto hCom : hComponents)
        {
            SYS_COMPONENT.Set_Enable(eType, hCom, bActive);
        }
    }
}

_bool CGameObject::Get_Enabled() const
{
    return SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf).bEnable;
}

void CGameObject::Set_ComponentMask(Component::COMPONENT_MASK mask)
{
    SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf).componentMask = mask;
}

Component::COMPONENT_MASK CGameObject::Get_ComponentMask() const
{
    return SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf).componentMask;
}

Layer::LAYER_ID CGameObject::Get_Layer() const
{
    const GAMEOBJECT_DATA& tData = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);
    return tData.layer;
}

const INSTANCE_UUID& CGameObject::Get_UUID() const
{
    const GAMEOBJECT_DATA& tData = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);
    return tData.tUUID;
}

const ASSET_GUID& CGameObject::Get_ProtoGUID() const
{
    return SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf).tProtoGUID;
}

void CGameObject::Set_ProtoGUID(const ASSET_GUID& tGUID)
{
    SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf).tProtoGUID = tGUID;
}

CGameObject* CGameObject::Clone()
{
    if (!Is_Valid())
        return nullptr;

    std::string cloneName = std::string(Get_Label()) + "_Clone";

    const GAMEOBJECT_DATA& srcObjData = SYS_GAMEOBJECT.Access_Data_Raw(m_hSelf);

    CGameObject* pClone = nullptr;
    if (false == m_hSelf.Is_UI())
        pClone = SYS_GAMEOBJECT.Create_GameObject(srcObjData.layer, cloneName, Get_Parent());
    else
        pClone = SYS_GAMEOBJECT.Create_GameObjectUI(srcObjData.layer, cloneName, Get_Parent());

    IF_NULL_RETURN_MSG_BREAK(pClone, nullptr, "Create GameObject failed");

    GAMEOBJECT_DATA& dstObjData = SYS_GAMEOBJECT.Access_Data_Raw(pClone->Get_Handle());

    dstObjData.tProtoGUID = srcObjData.tProtoGUID;
    dstObjData.layer = srcObjData.layer;

    const Component::COMPONENT_MASK srcMask = Get_ComponentMask();

    for (_uint j = 0; j < COMPONENT_MAX; ++j)
    {
        const COMPONENT_TYPE eComType = INT_TO_COM(j);
        if ((srcMask & Component::To_Bit(eComType)) == 0)
            continue;

        std::vector<COMPONENT_HANDLE> srcComps;
        SYS_COMPONENT.Get_Component_Handle_By_Type(eComType, m_hSelf, srcComps);

        for (const auto hSrcCom : srcComps)
        {
            auto upSpec = SYS_COMPONENT.Build_Spec_By_Type(eComType, hSrcCom);
            if (!upSpec)
                continue;
            SYS_COMPONENT.Create_Component_From_Spec(pClone, upSpec.get());
            __noop;
        }
    }

    return pClone;
}

NS_END
