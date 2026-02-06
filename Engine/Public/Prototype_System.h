#pragma once
#include "Asset_GUID.h"
#include "Base.h"
#include "Prototype.h"

NS_BEGIN(Engine)

/**
 * @class CPrototype_System
 * @brief Manages 'Prototypes'—the blueprints for game objects, functionally equivalent to Unity's Prefabs.
 * * During the initialization phase, it parses prototype blueprints (JSON) to identify required
 * components and assets. It then instantiates and stores the original 'Master' CPrototype objects
 * in memory, which serve as the source for all future Clone() operations.
 */
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
    HRESULT Create_Prototype(const ASSET_GUID& tGUID, PROTOTYPE_SPEC&& spec);

    /* TODO ::::::::::::: 지워 지워ㅣ주어지이날어니ㅏㅇ러ㅣㄴ아러니아러ㅣㄴ아ㅓㄹ */
    HRESULT Create_Prototype_For_Test(const ASSET_GUID& tGUID, const string key, Layer::LAYER_ID layer, std::string name, COMPONENT_SPEC_BUNDLE&& bundle);


    const CPrototype* Find(const ASSET_GUID& tGUID) const;
    CGameObject* Clone(const ASSET_GUID& tGUID) const;

private:
    std::unordered_map<ASSET_GUID, CPrototype*, ASSET_GUID_HASHER> m_Prototypes;
};

NS_END
