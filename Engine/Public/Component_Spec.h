#pragma once
#include "Engine_Define.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagTransformSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TRANSFORM)

    _float3 vPosition{ 0,0,0 };
    _float4 vRotationQuat{ 0,0,0,1 };
    _float3 vScale{ 1,1,1 };

    [[nodiscard]]
    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagTransformSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["Position"] = { vPosition.x, vPosition.y, vPosition.z };
        j["Rotation"] = { vRotationQuat.x, vRotationQuat.y, vRotationQuat.z, vRotationQuat.w };
        j["Scale"] = { vScale.x, vScale.y, vScale.z };
    };

    _bool FromJson(const json& j) override
    {
        try
        {
            vPosition = { j["Position"][0], j["Position"][1], j["Position"][2] };
            vRotationQuat = { j["Rotation"][0], j["Rotation"][1], j["Rotation"][2], j["Rotation"][3] };
            vScale = { j["Scale"][0], j["Scale"][1], j["Scale"][2] };
            return true;

            // Position이 있을 때만 덮어쓰기 (없으면 원본/기본값 유지)!!!!!!!!!!!!!!!!!!!!! 이게 더 싼듯?
            if (j.contains("Position") && j["Position"].is_array()) {
                vPosition = { j["Position"][0], j["Position"][1], j["Position"][2] };
            }
            if (j.contains("Rotation") && j["Rotation"].is_array()) {
                vRotationQuat = { j["Rotation"][0], j["Rotation"][1], j["Rotation"][2], j["Rotation"][3] };
            }
            if (j.contains("Scale") && j["Scale"].is_array()) {
                vScale = { j["Scale"][0], j["Scale"][1], j["Scale"][2] };
            }
        }
        catch (...)
        {
            return false;
        }
    };

} TRANSFORM_SPEC;

typedef struct ENGINE_DLL tagRectTransformSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::RECT_TRANSFORM)

    _float2 vPosPx{ 0.f, 0.f };
    _float2 vSizePx{ 100.f, 100.f };

    [[nodiscard]]
    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagRectTransformSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["PosPx"] = { vPosPx.x, vPosPx.y };
        j["SizePx"] = { vSizePx.x, vSizePx.y };
    }

    _bool FromJson(const json& j) override
    {
        try
        {
            if (j.contains("PosPx") && j["PosPx"].is_array() && j["PosPx"].size() >= 2)
                vPosPx = { j["PosPx"][0], j["PosPx"][1] };

            if (j.contains("SizePx") && j["SizePx"].is_array() && j["SizePx"].size() >= 2)
                vSizePx = { j["SizePx"][0], j["SizePx"][1] };

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

} RECTTRANSFORM_SPEC;

typedef struct ENGINE_DLL tagCanvasRendererSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::CANVAS_RENDERER)

        ASSET_GUID materialGUID{};
    ASSET_GUID textureGUID{};

    _float4 vColor = { 1.f, 1.f, 1.f, 1.f };
    RECT_F  rcUV = { 0.f, 0.f, 1.f, 1.f };
    RECT_F  rcClip = { 0.f, 0.f, 0.f, 0.f };

    uint32_t flags = CF_NONE;
    RENDER_LAYER layer = RENDER_LAYER::UI;
    _float sortZ = 0.f;

    [[nodiscard]]
    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagCanvasRendererSpec>(*this);
    }

    void ToJson(json& j) const override {
        j["Type"] = SCAST(_uint, Get_Type());

        if (materialGUID.Is_Valid())
            j["MaterialGUID"] = materialGUID.To_String_Utf8();
        if (textureGUID.Is_Valid())
            j["TextureGUID"] = textureGUID.To_String_Utf8();

        if (vColor.x != 1.f || vColor.y != 1.f || vColor.z != 1.f || vColor.w != 1.f)
            j["vColor"] = { vColor.x, vColor.y, vColor.z, vColor.w };

        if (rcUV.fLeft != 0.f || rcUV.fTop != 0.f || rcUV.fRight != 1.f || rcUV.fBottom != 1.f)
            j["rcUV"] = { rcUV.fLeft, rcUV.fTop, rcUV.fRight, rcUV.fBottom };

        if (rcClip.fLeft != 0.f || rcClip.fTop != 0.f || rcClip.fRight != 0.f || rcClip.fBottom != 0.f)
            j["rcClip"] = { rcClip.fLeft, rcClip.fTop, rcClip.fRight, rcClip.fBottom };

        if (flags != CF_NONE)
            j["flags"] = flags;
        if (layer != RENDER_LAYER::UI)
            j["layer"] = SCAST(_uint, layer);
        if (sortZ != 0.f)
            j["sortZ"] = sortZ;
    }

    _bool FromJson(const json& j) override
    {
        try {
            if (j.contains("MaterialGUID"))
                ASSET_GUID::Try_Utf8_To_GUID(j["MaterialGUID"], materialGUID);
            if (j.contains("TextureGUID"))
                ASSET_GUID::Try_Utf8_To_GUID(j["TextureGUID"], textureGUID);

            if (j.contains("vColor") && j["vColor"].is_array() && j["vColor"].size() >= 4)
                vColor = { j["vColor"][0], j["vColor"][1], j["vColor"][2], j["vColor"][3] };

            if (j.contains("rcUV") && j["rcUV"].is_array() && j["rcUV"].size() >= 4)
                rcUV = { j["rcUV"][0], j["rcUV"][1], j["rcUV"][2], j["rcUV"][3] };

            if (j.contains("rcClip") && j["rcClip"].is_array() && j["rcClip"].size() >= 4)
                rcClip = { j["rcClip"][0], j["rcClip"][1], j["rcClip"][2], j["rcClip"][3] };

            flags = j.value("flags", flags);
            layer = (RENDER_LAYER)j.value("layer", (uint32_t)layer);
            sortZ = j.value("sortZ", sortZ);

            return true;
        }
        catch (...) {
            return false;
        }
    }
} CANVAS_RENDERER_SPEC;

typedef struct ENGINE_DLL tagMeshRendererSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::MESH_RENDERER)

    ASSET_GUID  meshGUID{};
    ASSET_GUID  materialGUID{};

    uint16_t passIndex = 0;

    uint32_t     flags = RF_NONE;
    RENDER_LAYER layer = RENDER_LAYER::NONBLEND;

    float sortZ = 0.f;

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagMeshRendererSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["MeshGUID"] = meshGUID.To_String_Utf8();
        j["MaterialGUID"] = materialGUID.To_String_Utf8();
        j["PassIndex"] = passIndex;
        j["Flags"] = flags;
        j["Layer"] = (uint8_t)layer;
        j["SortZ"] = sortZ;
    }
    _bool FromJson(const json& j) override
    {
        ASSET_GUID::Try_Utf8_To_GUID(j.value("MeshGUID", ""), meshGUID);
        ASSET_GUID::Try_Utf8_To_GUID(j.value("MaterialGUID", ""), materialGUID);
        passIndex = (uint16_t)j.value("PassIndex", 0);
        flags = (uint32_t)j.value("Flags", 0);
        layer = (RENDER_LAYER)j.value("Layer", (uint8_t)RENDER_LAYER::NONBLEND);
        sortZ = (float)j.value("SortZ", 0.0f);

        return meshGUID.Is_Valid() && materialGUID.Is_Valid();
    }
}MESH_RENDERER_SPEC;

NS_END
