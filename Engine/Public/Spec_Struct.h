#pragma once

#include "Identity.h"
#include "Engine_Enum.h"

NS_BEGIN(Engine)


#define COMPONENT_SPEC_TYPE(_TYPE)                                      \
    static constexpr COMPONENT_TYPE TYPE = _TYPE;                       \
    COMPONENT_TYPE Get_Type() const noexcept override { return TYPE; }

typedef struct ENGINE_DLL tagCOmponentSpecBase
{
    virtual ~tagCOmponentSpecBase() = default;
    virtual COMPONENT_TYPE Get_Type() const noexcept = 0;

    virtual std::unique_ptr<tagCOmponentSpecBase> Clone() const = 0;
    virtual void ToJson(json& j) const = 0;
    virtual _bool FromJson(const json& j) = 0;

}COMPONENT_SPEC_BASE;

typedef struct tagComponentSpecBundle
{
    std::vector<std::unique_ptr<COMPONENT_SPEC_BASE>> components;
    template<typename TSpec>
    const TSpec* Find_One() const
    {
        for (const auto& pSpec : components)
        {
            if (pSpec->Get_Type() == TSpec::TYPE)
                return SCAST(const TSpec*, pSpec.get());
        }

        return nullptr;
    }

    tagComponentSpecBundle()
    {
        components.resize(COMPONENT_MAX);
    };
    ~tagComponentSpecBundle() = default;
    tagComponentSpecBundle(const tagComponentSpecBundle&) = delete;
    tagComponentSpecBundle& operator=(const tagComponentSpecBundle&) = delete;
    tagComponentSpecBundle(tagComponentSpecBundle&& other) noexcept  = default;
    tagComponentSpecBundle& operator=(tagComponentSpecBundle&& other) noexcept = default;
} COMPONENT_SPEC_BUNDLE;


typedef struct ENGINE_DLL tagPrototypeSpec
{
    std::string  strName;
    _bool           isUI = false;
    COMPONENT_SPEC_BUNDLE tComponentBundle;

    std::vector<struct tagPrototypeSpec> vecChildren;

    tagPrototypeSpec() = default;
    ~tagPrototypeSpec() = default;
    tagPrototypeSpec(const tagPrototypeSpec&) = delete;
    tagPrototypeSpec& operator=(const tagPrototypeSpec&)noexcept = delete;
    tagPrototypeSpec(tagPrototypeSpec&&) noexcept = default;
    tagPrototypeSpec& operator=(tagPrototypeSpec&&) = default;
}PROTOTYPE_SPEC;

typedef struct tagSceneObjecSpec
{
    INSTANCE_UUID uuid;
    ASSET_GUID protoGuid;

    _bool       isUI;

    std::string name;
    Layer::LAYER_ID layer;
    INSTANCE_UUID parent;

    Component::COMPONENT_MASK hasMask;
    COMPONENT_SPEC_BUNDLE overrides;

    tagSceneObjecSpec() = default;
    ~tagSceneObjecSpec() = default;
    tagSceneObjecSpec(const tagSceneObjecSpec&) = delete;
    tagSceneObjecSpec& operator=(const tagSceneObjecSpec&) = delete;
    tagSceneObjecSpec(tagSceneObjecSpec&&) = default;
    tagSceneObjecSpec& operator=(tagSceneObjecSpec&&) = default;

    tagSceneObjecSpec(
        const INSTANCE_UUID& _uuid,
        const ASSET_GUID& _protoGuid,
        const std::string& _name,
        Layer::LAYER_ID _layer,
        const INSTANCE_UUID& _parent,
        COMPONENT_SPEC_BUNDLE&& _overrides
    )
        : uuid(_uuid)
        , protoGuid(_protoGuid)
        , name(_name)
        , isUI(false)
        , layer(_layer)
        , parent(_parent)
        , hasMask(0)
        , overrides(std::move(_overrides))
    {
    }
}SCENE_OBJECT_SPEC;


NS_END
