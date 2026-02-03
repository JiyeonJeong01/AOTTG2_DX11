#include "GameObject.h"
#include "GameObject_System.h"

NS_BEGIN(Engine)

static GAMEOBJECT_DATA g_DummyData;

CGameObject::CGameObject(uint32_t iPoolIndex)
    : tagLabel("GameObject")
{
    m_hSelf.iIndex = iPoolIndex;
    m_hSelf.iVersion = 0;
}

void CGameObject::Render()
{
    if (!IsValid())
        return;
}

CGameObject* CGameObject::Get_Parent()
{
    if (!IsValid())
        return nullptr;

    GAMEOBJECT_HANDLE hParent = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf).hParent;

    if (!hParent.IsValid())
        return nullptr;

    return SYS_GAMEOBJECT->Get_Wrapper(hParent);
}

HRESULT CGameObject::Set_Parent(CGameObject* pNewParent)
{
    if (!IsValid())
        return E_FAIL;

    if (pNewParent == this)
        return E_FAIL;

    GAMEOBJECT_HANDLE hNewParent = pNewParent ? pNewParent->Get_Handle() : GAMEOBJECT_HANDLE{};

    GAMEOBJECT_DATA& tMyData = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);

    /* Already has the same parent; no changes needed. */
    if (tMyData.hParent == hNewParent)
        return S_OK;

    /* Remove this GameObject from the old parent's children list. */
    if (tMyData.hParent.IsValid())
    {
        CGameObject* pOldParentWrapper = SYS_GAMEOBJECT->Get_Wrapper(tMyData.hParent);
        if (pOldParentWrapper)
        {
            pOldParentWrapper->Remove_Child_Inner(m_hSelf);
        }
    }

    /* Register this GameObject to the new parent's children list. */
    tMyData.hParent = hNewParent;
    if (hNewParent.IsValid())
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

void CGameObject::Add_Child_Inner(GAMEOBJECT_HANDLE hChild)
{
    if (!IsValid())
        return;

    GAMEOBJECT_DATA& data = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);

    auto it = std::find(data.hChildren.begin(), data.hChildren.end(), hChild);
    if (it == data.hChildren.end())
    {
        data.hChildren.push_back(hChild);
    }
}

void CGameObject::Remove_Child_Inner(GAMEOBJECT_HANDLE hChild)
{
    if (!IsValid())
        return;

    GAMEOBJECT_DATA& data = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);

    auto it = std::find(data.hChildren.begin(), data.hChildren.end(), hChild);
    if (it != data.hChildren.end())
    {
        data.hChildren.erase(it);
    }
}

std::vector<CGameObject*> CGameObject::Get_Children() const
{
    std::vector<CGameObject*> result;

    if (!IsValid())
        return result;

    const GAMEOBJECT_DATA& data = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);
    result.reserve(data.hChildren.size());

    for (const auto& hChild : data.hChildren)
    {
        CGameObject* pChild = SYS_GAMEOBJECT->Get_Wrapper(hChild);
        if (pChild && pChild->IsValid())
            result.push_back(pChild);
    }

    return result;
}

GAMEOBJECT_HANDLE CGameObject::Get_Handle() const
{
    if (!IsValid())
        return GAMEOBJECT_HANDLE{};

    return m_hSelf;
}

_bool CGameObject::IsValid() const
{

    /* TODO ==================================================== */
    /* TODO : MAKE SURE TO MINIMIZE THE NUMBER OF CALLS TO THIS! */
    /* TODO ==================================================== */
    return SYS_GAMEOBJECT->Is_Valid_Handle(m_hSelf);
}

void CGameObject::Set_Active(_bool bActive)
{
    if (!IsValid())
        return;

    SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf).bActive = bActive;
}

_bool CGameObject::Get_Active() const
{
    if (!IsValid())
        return false;

    return SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf).bActive;
}

CGameObject* CGameObject::Create(uint32_t iLayer, std::string strName, CGameObject* pParent)
{
    CGameObject* pInstance = nullptr;

    if (FAILED(SYS_GAMEOBJECT->Create_Object(&pInstance, iLayer, strName)))
    {
        return nullptr;
    }

    if (pParent)
        pInstance->Set_Parent(pParent);

    return pInstance;
}

CGameObject* CGameObject::Clone()
{
    if (!IsValid())
        return nullptr;

    std::string cloneName = std::string(Get_Label()) + "_Clone";

    const GAMEOBJECT_DATA& tData = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);
    CGameObject* pClone = CGameObject::Create(tData.layer, cloneName, nullptr);
    if (!pClone)
        return nullptr;

    // 데이터 복사 (컴포넌트 등)
    // todo ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // [TODO] 컴포넌트 복사 로직 (Data Copy)
    // GAMEOBJECT_DATA& srcData  = SYS_GAMEOBJECT->Access_Data_Raw(m_hSelf);
    // GAMEOBJECT_DATA& destData = SYS_GAMEOBJECT->Access_Data_Raw(pClone->Get_Handle());
    //
    // destData.iComponentSlots ... 복사 필요
    // todo ++++++++++++++++++++++++++++++++++++++++++++++++++++++

    return pClone;
}

void CGameObject::Free()
{
    CBase::Free();
}

NS_END
