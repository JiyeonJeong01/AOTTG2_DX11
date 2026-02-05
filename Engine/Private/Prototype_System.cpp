#include "Prototype_System.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

IMPLEMENT_SINGLETON(CPrototype_System)

void CPrototype_System::Clear()
{
    m_Prototypes.clear();
}

HRESULT CPrototype_System::Create_Prototype(PROTOTYPE_KEY key, PROTOTYPE_SPEC&& spec)
{
    if (key == PROTOTYPE_KEY{}) // TODO 여기 유효성 검사 추가 TODO 
        return E_FAIL;

    auto it = m_Prototypes.find(key);
    _DEBUG_ENGINE_ASSERT_MSG((it == m_Prototypes.end()), "Prototype key already exists!");
    if (it != m_Prototypes.end())
        return E_FAIL;

    CPrototype* pProto = CPrototype::Create();

    if (FAILED(pProto->Assemble(std::move(spec))))
        return E_FAIL;

    m_Prototypes.emplace(key, std::move(pProto));
    return S_OK;
}

HRESULT CPrototype_System::Create_Prototype(PROTOTYPE_KEY key, Layer::LAYER_ID layer, std::string name, COMPONENT_SPEC_BUNDLE&& bundle)
{
    PROTOTYPE_SPEC spec{};
    spec.layer = layer;
    spec.strName = std::move(name);
    spec.tComponentBundle = std::move(bundle);
    return Create_Prototype(key, std::move(spec));
}

const CPrototype* CPrototype_System::Find(const PROTOTYPE_KEY& key) const
{
    auto it = m_Prototypes.find(key);
    if (it == m_Prototypes.end())
        return nullptr;
    return it->second;
}

CGameObject* CPrototype_System::Clone(const PROTOTYPE_KEY& key) const
{
    const CPrototype* pProto = Find(key);
    if (!pProto)
        return nullptr;

    return pProto->Clone();
}

NS_END
