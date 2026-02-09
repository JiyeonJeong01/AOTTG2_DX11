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
public:
    HRESULT Initialize() { return S_OK; }
    void    Clear();

public:
    HRESULT Create_Prototype(const ASSET_GUID& tGUID, PROTOTYPE_SPEC&& spec);

    const CPrototype* Find(const ASSET_GUID& tGUID) const;
    CGameObject* Clone(const ASSET_GUID& tGUID) const;

private:
    std::unordered_map<ASSET_GUID, CPrototype*, ASSET_GUID_HASHER> m_Prototypes;
};

NS_END
