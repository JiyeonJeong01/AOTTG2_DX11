#pragma once
#include "Identity.h"

#include "Spec_Struct.h"

NS_BEGIN(Engine)
class CPrototype;
class CGameObject;

/**
 * @class CPrototype_System
 * @brief Manages 'Prototypes'—the blueprints for game objects, functionally equivalent to Unity's Prefabs.
 * * During the initialization phase, it parses prototype blueprints (JSON) to identify required
 * components and assets. It then instantiates and stores the original 'Master' CPrototype objects
 * in memory, which serve as the source for all future Clone() operations.
 */
class ENGINE_DLL CPrototype_System final
{
    DECLARE_SINGLETON(CPrototype_System)
public:
    HRESULT Initialize() { return S_OK; }
    void    Clear();

public:
    HRESULT Register_Prototype(const ASSET_GUID& tGUID, PROTOTYPE_SPEC&& spec);
    HRESULT Load_Prototype_From_File(const ASSET_GUID& tGUID);

    const CPrototype* Find(const ASSET_GUID& tGUID) const;
    CGameObject* Clone(const ASSET_GUID& tGUID,
                        Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER,
                        const string& strName = "GameObject_clone",
                        const INSTANCE_UUID& tUUID = INSTANCE_UUID{}) const;

private:
    std::unordered_map<ASSET_GUID, std::unique_ptr<CPrototype>, ASSET_GUID_HASHER> m_Prototypes;
};

NS_END
