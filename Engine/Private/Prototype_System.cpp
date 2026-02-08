#include "Prototype_System.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CPrototype_System)

CPrototype_System::CPrototype_System()
{
    
}
CPrototype_System::~CPrototype_System()
{
    
}

void CPrototype_System::Clear()
{
    m_Prototypes.clear();
}

/* Create a master prototype using the asset GUID and register it to the system for future cloning */
HRESULT CPrototype_System::Create_Prototype(const ASSET_GUID& tGUID, PROTOTYPE_SPEC&& spec)
{
    if (!tGUID.Is_Valid())
        return E_FAIL;

    auto it = m_Prototypes.find(tGUID);
    _DEBUG_ENGINE_ASSERT_MSG((it == m_Prototypes.end()), "Prototype key already exists!");
    if (it != m_Prototypes.end())
        return E_FAIL;

    CPrototype* pProto = CPrototype::Create();

    if (FAILED(pProto->Assemble(std::move(spec))))
        return E_FAIL;

    m_Prototypes.emplace(tGUID, std::move(pProto));
    return S_OK;
}

const CPrototype* CPrototype_System::Find(const ASSET_GUID& tGUID) const
{
    auto it = m_Prototypes.find(tGUID);
    if (it == m_Prototypes.end())
        return nullptr;
    return it->second;
}

CGameObject* CPrototype_System::Clone(const ASSET_GUID& tGUID) const
{
    const CPrototype* pProto = Find(tGUID);
    if (!pProto)
        return nullptr;

    return pProto->Clone();
}

NS_END
