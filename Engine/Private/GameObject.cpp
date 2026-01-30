#include "GameObject.h"
#include "GameObject_System.h"

Engine::CGameObject::CGameObject()
    : tagLabel("GameObject")
{
}

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

HRESULT CGameObject::Set_Parent(CGameObject* pNewParent)
{
    if (!pNewParent)
        return E_FAIL;

    Safe_Release(m_pParent);
    m_pParent = pNewParent;
    Safe_AddRef(m_pParent);

    return S_OK;
}

CGameObject* CGameObject::Get_Parent()
{
    return m_pParent;
}

HRESULT CGameObject::Add_Child(CGameObject* pNewChild)
{
    if (!pNewChild)
        return E_FAIL;
    m_pChildren.push_back(pNewChild);

    return S_OK;
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
