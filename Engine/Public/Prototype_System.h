#pragma once
#include "Base.h"
#include "Prototype.h"

NS_BEGIN(Engine)

class ENGINE_DLL CPrototype_System final : public CBase
{
    DECLARE_SINGLETON(CPrototype_System)

private:
    CPrototype_System() = default;
    ~CPrototype_System() override = default;

public:
    HRESULT Initialize() { return S_OK; }
    void    Clear();

public:
    HRESULT Create_Prototype(PROTOTYPE_KEY key, PROTOTYPE_SPEC&& spec);
    HRESULT Create_Prototype(PROTOTYPE_KEY key, Layer::LAYER_ID layer, std::string name, COMPONENT_SPEC_BUNDLE&& bundle);

    const CPrototype* Find(const PROTOTYPE_KEY& key) const;
    CGameObject* Clone(const PROTOTYPE_KEY& key) const;

private:
    std::unordered_map<PROTOTYPE_KEY, CPrototype*> m_Prototypes;
};

NS_END
