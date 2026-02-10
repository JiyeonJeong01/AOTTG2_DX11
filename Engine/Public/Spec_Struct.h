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

    tagComponentSpecBundle() = default;
    ~tagComponentSpecBundle() = default;
    tagComponentSpecBundle(const tagComponentSpecBundle&) = delete;
    tagComponentSpecBundle& operator=(const tagComponentSpecBundle&) = delete;
    tagComponentSpecBundle(tagComponentSpecBundle&& other) = default;
    tagComponentSpecBundle& operator=(tagComponentSpecBundle&& other) = default;
} COMPONENT_SPEC_BUNDLE;


typedef struct ENGINE_DLL tagPrototypeSpec
{
    std::string  strName;
    COMPONENT_SPEC_BUNDLE tComponentBundle;

    tagPrototypeSpec() = default;
    ~tagPrototypeSpec() = default;
    tagPrototypeSpec(const tagPrototypeSpec&) = delete;
    tagPrototypeSpec& operator=(const tagPrototypeSpec&) = delete;
    tagPrototypeSpec(tagPrototypeSpec&&) = default;
    tagPrototypeSpec& operator=(tagPrototypeSpec&&) = default;
}PROTOTYPE_SPEC;

typedef struct tagSceneObjecSpec
{
    INSTANCE_UUID uuid;
    ASSET_GUID protoGuid;

    std::string name;
    Layer::LAYER_ID layer;
    INSTANCE_UUID parent;

    COMPONENT_SPEC_BUNDLE overrides;

    tagSceneObjecSpec() = default;
    ~tagSceneObjecSpec() = default;
    tagSceneObjecSpec(const tagSceneObjecSpec&) = delete;
    tagSceneObjecSpec& operator=(const tagSceneObjecSpec&) = delete;
    tagSceneObjecSpec(tagSceneObjecSpec&&) = default;
    tagSceneObjecSpec& operator=(tagSceneObjecSpec&&) = default;
}SCENE_OBJECT_SPEC;


NS_END
