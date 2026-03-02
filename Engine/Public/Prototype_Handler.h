#pragma once
#include "Identity.h"
#include "Prototype.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)

class CGameObject;

/**
 * @class CPrototype_Handler
 * @brief Manages 'Prototypes'—the blueprints for game objects, functionally equivalent to Unity's Prefabs.
 * * During the initialization phase, it parses prototype blueprints (JSON) to identify required
 * components and assets. It then instantiates and stores the original 'Master' CPrototype objects
 * in memory, which serve as the source for all future Clone() operations.
 */
class ENGINE_DLL CPrototype_Handler final
{
public :
    CPrototype_Handler();
    ~CPrototype_Handler();

    CPrototype_Handler(const CPrototype_Handler&) = delete;
    CPrototype_Handler& operator=(const CPrototype_Handler&) = delete;
    CPrototype_Handler(CPrototype_Handler&&) noexcept = default;
    CPrototype_Handler& operator=(CPrototype_Handler&&) noexcept = default;

public:
    HRESULT Initialize();
    void    Clear();

public:
    HRESULT Register_Prototype(const ASSET_GUID& tGUID, PROTOTYPE_SPEC&& spec);
    HRESULT Load_Prototype_From_GUID(const ASSET_GUID& tGUID);

    const CPrototype* Find(const ASSET_GUID& tGUID) const;
    const CPrototype* Find(const std::string& strName);

    CGameObject* Clone(const ASSET_GUID& tGUID,
                        Layer::LAYER_ID iLayer = Layer::DEFAULT_LAYER,
                        const string& strName = "GameObject_clone",
                        const INSTANCE_UUID& tUUID = INSTANCE_UUID{}) const;
    _bool   Create_Prototype_Spec(CGameObject* pObj);
    void    Build_PrototypeSpec_From_Object(CGameObject* pObj, PROTOTYPE_SPEC& outSpec);

private :
    /* Json Helpers */
    _bool Save_PrototypeFile(const PROTOTYPE_SPEC& tSpec, const std::filesystem::path& path);
    _bool Load_PrototypeFile(PROTOTYPE_SPEC& outSpec, const std::filesystem::path& path);

    json Serialize_PrototypeSpec(const PROTOTYPE_SPEC& tSpec);
    _bool Deserialize_PrototypeSpec(const json& j, PROTOTYPE_SPEC& outSpec);

    /*  Spec 생성을 위한 팩토리 함수 필요 */
    std::unique_ptr<COMPONENT_SPEC_BASE> Create_Spec_By_Type(COMPONENT_TYPE eType);

private:
    std::unordered_map<ASSET_GUID, std::unique_ptr<CPrototype>, ASSET_GUID_HASHER> m_Prototypes;
    std::unordered_map<std::string, CPrototype*>        m_NameToProto;
    uint32_t                                            m_iDuplicatedProtos{};
    std::filesystem::path                               m_pathProto{};

public :
    static std::unique_ptr<CPrototype_Handler> Create();

};

NS_END
