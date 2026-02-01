#include "GameObject.h"
#include "GameObject_System.h"

Engine::CGameObject::CGameObject()
    : tagLabel("GameObject")
{
}

NS_BEGIN(Engine)

CGameObject::CGameObject(std::string str)
    : tagLabel(str)
{
}

CGameObject::CGameObject(const CGameObject& Clone)
    : tagLabel(Clone), m_pParent(Clone.m_pParent)
{
    /* TODO : Children은 따로 만들어줘야 할듯 */
}

void Engine::CGameObject::Render()
{

}

GAMEOBJECT_META& CGameObject::Access_Meta()
{
    return m_tMeta;
}

const GAMEOBJECT_META& CGameObject::Access_Meta() const
{
    return m_tMeta;
}

CGameObject* CGameObject::Get_Parent()
{
    return m_pParent;
}

HRESULT CGameObject::Set_Parent(CGameObject* pNewParent)
{
    if (pNewParent == this) return E_FAIL;
    if (pNewParent == m_pParent) return S_OK;

    if (m_pParent)
    {
        m_pParent->Remove_Child_Inner(this);
        Safe_Release(m_pParent);
    }

    m_pParent = pNewParent;

    if (m_pParent)
    {
        Safe_AddRef(m_pParent);
        m_pParent->Add_Child_Inner(this);
    }

    return S_OK;
}

HRESULT CGameObject::Add_Child(CGameObject* pChild)
{
    if (!pChild) return E_FAIL;

    return pChild->Set_Parent(this);
}

HRESULT CGameObject::Remove_Child(CGameObject* pChild)
{
    if (!pChild) return E_FAIL;

    return pChild->Set_Parent(nullptr);
}

void CGameObject::Add_Child_Inner(CGameObject* pChild)
{
    auto it = std::find(m_pChildren.begin(), m_pChildren.end(), pChild);
    if (it == m_pChildren.end())
    {
        m_pChildren.push_back(pChild);
        Safe_AddRef(pChild); 
    }
}

void CGameObject::Remove_Child_Inner(CGameObject* pChild)
{
    auto it = std::find(m_pChildren.begin(), m_pChildren.end(), pChild);
    if (it != m_pChildren.end())
    {
        Safe_Release(*it);
        m_pChildren.erase(it);
    }
}

const std::vector<CGameObject*>& CGameObject::Get_Children() const
{
    return m_pChildren;
}

COMPONENT_HANDLE Engine::CGameObject::Decode_Slot(uint32_t iSlotData) const
{
	COMPONENT_HANDLE h;
	h.iHandle = iSlotData & ComponentConfig::DATA_MASK;
	return h;
}

CGameObject* CGameObject::Create(uint32_t iLayer, string strName, CGameObject* pParent)
{
    CGameObject* pInstance = new CGameObject(strName);

    if (!pParent)
        pInstance->Set_Parent(pParent);

    if (FAILED(SYS_GAMEOBJECT->Create_Object(pInstance, iLayer)))
    {
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CGameObject::Clone()
{

    CGameObject* pInstance = new CGameObject(*this);
    if(FAILED(SYS_GAMEOBJECT->Create_Object(pInstance, m_tMeta.layer)))
    {
        Safe_Release(pInstance);
    }

    return pInstance;
}

void Engine::CGameObject::Free()
{
	CBase::Free();
}

NS_END
