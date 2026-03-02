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
    /* 컴포넌트 타입 슬롯*/
    std::array<std::vector<std::unique_ptr<COMPONENT_SPEC_BASE>>, COMPONENT_MAX> components;

    template<typename TSpec>
    const TSpec* Find_One() const
    {
        constexpr _uint slot = SCAST(_uint, TSpec::TYPE);

        static_assert(std::is_base_of_v<COMPONENT_SPEC_BASE, TSpec>, "TSpec must derive from COMPONENT_SPEC_BASE.");
        static_assert(TSpec::TYPE < COMPONENT_MAX, "TSpec::TYPE out of range.");

        const auto& v = components[slot];
        if (v.empty())
            return nullptr;

        return SCAST(const TSpec*, v.front().get());
    }

    template<typename TSpec>
    TSpec* Find_One()
    {
        return const_cast<TSpec*>(static_cast<const tagComponentSpecBundle*>(this)->Find_One<TSpec>());
    }

    /* 특정 컴포넌트 모든 슬롯에 접근하여 읽기용 */
    const std::vector<std::unique_ptr<COMPONENT_SPEC_BASE>>& Get_All(COMPONENT_TYPE eComType) const
    {
        return components[SCAST(_uint, eComType)];
    }

    /* 특정 컴포넌트 모든 슬롯에 접근하여 쓰기용 */
    std::vector<std::unique_ptr<COMPONENT_SPEC_BASE>>& Get_All(COMPONENT_TYPE eComType)
    {
        return components[SCAST(_uint, eComType)];
    }

    /* 슬롯에 스펙 추가 */
    void Add(COMPONENT_TYPE eComType, std::unique_ptr<COMPONENT_SPEC_BASE>&& pSpec)
    {
        components[SCAST(_uint, eComType)].emplace_back(std::move(pSpec));
    }

    void Clear(COMPONENT_TYPE eComType)
    {
        components[SCAST(_uint, eComType)].clear();
    }

    void Clear_All()
    {
        for (auto& v : components)
            v.clear();
    }

    tagComponentSpecBundle() = default;
    ~tagComponentSpecBundle() = default;
    tagComponentSpecBundle(const tagComponentSpecBundle&) = delete;
    tagComponentSpecBundle& operator=(const tagComponentSpecBundle&) = delete;
    tagComponentSpecBundle(tagComponentSpecBundle&& other) noexcept = default;
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
