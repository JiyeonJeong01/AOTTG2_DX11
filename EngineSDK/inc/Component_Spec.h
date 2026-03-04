#pragma once
#include "Spec_Util.h"

NS_BEGIN(Engine)

class CTransform;
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
        if (j.contains("Type"))
        {
            const auto t = j["Type"];
            if (!t.is_number_unsigned())
                return false;

            const _uint type = t.get<_uint>();
            if (type != SCAST(_uint, Get_Type()))
                return false;
        }

        auto read_vec3 = [&](const char* key, _float3& out) -> bool
            {
                if (!j.contains(key))
                    return true;
                const auto& a = j.at(key);
                if (!a.is_array() || a.size() != 3) return false;
                if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number()) return false;

                out = { a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>() };
                return true;
            };

        auto read_quat = [&](const char* key, _float4& out) -> bool
            {
                if (!j.contains(key)) return true;
                const auto& a = j.at(key);
                if (!a.is_array() || a.size() != 4) return false;
                for (int i = 0; i < 4; ++i) if (!a[i].is_number()) return false;

                out = { a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>(), a[3].get<_float>() };
                return true;
            };

        if (!read_vec3("Position", vPosition)) return false;
        if (!read_quat("Rotation", vRotationQuat)) return false;
        if (!read_vec3("Scale", vScale)) return false;

        // scale 0 방지
        const _float eps = 1e-6f;
        if (fabs(vScale.x) < eps) vScale.x = 1.f;
        if (fabs(vScale.y) < eps) vScale.y = 1.f;
        if (fabs(vScale.z) < eps) vScale.z = 1.f;

        const _float len2 =
            vRotationQuat.x * vRotationQuat.x +
            vRotationQuat.y * vRotationQuat.y +
            vRotationQuat.z * vRotationQuat.z +
            vRotationQuat.w * vRotationQuat.w;

        if (len2 < eps)
            vRotationQuat = { 0,0,0,1 };
        else
        {
            const _float invLen = 1.f / sqrt(len2);
            vRotationQuat.x *= invLen;
            vRotationQuat.y *= invLen;
            vRotationQuat.z *= invLen;
            vRotationQuat.w *= invLen;
        }

        return true;
    }

} TRANSFORM_SPEC;

class CRectTransform;
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
        if (j.contains("Type")) {
            const auto& t = j.at("Type");
            if (!t.is_number_unsigned()) return false;
            if (t.get<_uint>() != SCAST(_uint, Get_Type())) return false;
        }

        auto read_vec2 = [&](const char* key, _float2& out) -> bool
            {
                if (!j.contains(key)) return true;
                const auto& a = j.at(key);
                if (!a.is_array() || a.size() != 2) return false;
                if (!a[0].is_number() || !a[1].is_number()) return false;
                out = { a[0].get<_float>(), a[1].get<_float>() };
                return true;
            };

        if (!read_vec2("PosPx", vPosPx)) return false;
        if (!read_vec2("SizePx", vSizePx)) return false;

        // 사이즈 0/음수 방지
        const _float eps = 1e-3f;
        if (!(vSizePx.x > eps)) vSizePx.x = 100.f;
        if (!(vSizePx.y > eps)) vSizePx.y = 100.f;

        return true;
    }
} RECTTRANSFORM_SPEC;

class CCanvasRenderer;
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

    _bool bEnabled = true;

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

        // Color (Default: White)
        if (vColor.x != 1.f || vColor.y != 1.f || vColor.z != 1.f || vColor.w != 1.f)
            j["vColor"] = { vColor.x, vColor.y, vColor.z, vColor.w };

        // UV (Default: 0,0,1,1)
        if (rcUV.fLeft != 0.f || rcUV.fTop != 0.f || rcUV.fRight != 1.f || rcUV.fBottom != 1.f)
            j["rcUV"] = { rcUV.fLeft, rcUV.fTop, rcUV.fRight, rcUV.fBottom };

        // Clip (Default: 0,0,0,0)
        if (rcClip.fLeft != 0.f || rcClip.fTop != 0.f || rcClip.fRight != 0.f || rcClip.fBottom != 0.f)
            j["rcClip"] = { rcClip.fLeft, rcClip.fTop, rcClip.fRight, rcClip.fBottom };

        if (flags != CF_NONE)
            j["flags"] = flags;

        if (layer != RENDER_LAYER::UI)
            j["layer"] = SCAST(_uint, layer);

        if (sortZ != 0.f)
            j["sortZ"] = sortZ;

        if (!bEnabled)
            j["bEnabled"] = bEnabled;
    }

    _bool FromJson(const json& j) override
    {
        if (j.contains("Type"))
        {
            const auto& t = j.at("Type");
            if (!t.is_number_unsigned())
                return false;
            if (t.get<_uint>() != SCAST(_uint, Get_Type()))
                return false;
        }

        auto read_guid = [&](const char* key, ASSET_GUID& out) -> bool
            {
                if (!j.contains(key)) return true;
                const auto& v = j.at(key);
                if (!v.is_string()) return false;
                return ASSET_GUID::Try_Utf8_To_GUID(v, out);
            };

        auto read_vec4 = [&](const char* key, _float4& out) -> bool
            {
                if (!j.contains(key)) return true;
                const auto& a = j.at(key);
                if (!a.is_array() || a.size() != 4) return false;
                for (int i = 0; i < 4; ++i) if (!a[i].is_number()) return false;
                out = { a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>(), a[3].get<_float>() };
                return true;
            };

        auto read_rect4 = [&](const char* key, RECT_F& out) -> bool
            {
                _float4 v{};
                if (!read_vec4(key, v)) return false;
                if (!j.contains(key)) return true;
                out = { v.x, v.y, v.z, v.w };
                return true;
            };

        if (!read_guid("MaterialGUID", materialGUID)) return false;
        if (!read_guid("TextureGUID", textureGUID)) return false;

        if (!read_vec4("vColor", vColor)) return false;
        if (!read_rect4("rcUV", rcUV)) return false;
        if (!read_rect4("rcClip", rcClip)) return false;

        if (j.contains("flags"))
        {
            const auto& v = j.at("flags");
            if (!v.is_number_unsigned()) return false;
            flags = v.get<uint32_t>();
        }

        if (j.contains("layer"))
        {
            const auto& v = j.at("layer");
            if (!v.is_number_unsigned()) return false;
            layer = (RENDER_LAYER)v.get<uint32_t>();
        }

        if (j.contains("sortZ"))
        {
            const auto& v = j.at("sortZ");
            if (!v.is_number()) return false;
            sortZ = v.get<_float>();
        }

        if (j.contains("bEnabled"))
        {
            const auto& v = j.at("bEnabled");
            if (!v.is_boolean()) return false;
            bEnabled = v.get<_bool>();
        }
        else
        {
            bEnabled = true;
        }

        auto clamp01 = [](_float x) -> _float { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };

        vColor.x = clamp01(vColor.x);
        vColor.y = clamp01(vColor.y);
        vColor.z = clamp01(vColor.z);
        vColor.w = clamp01(vColor.w);

        rcUV.fLeft = clamp01(rcUV.fLeft);
        rcUV.fTop = clamp01(rcUV.fTop);
        rcUV.fRight = clamp01(rcUV.fRight);
        rcUV.fBottom = clamp01(rcUV.fBottom);
        if (rcUV.fLeft > rcUV.fRight)  std::swap(rcUV.fLeft, rcUV.fRight);
        if (rcUV.fTop > rcUV.fBottom) std::swap(rcUV.fTop, rcUV.fBottom);

        sortZ = clamp01(sortZ);

        return true;
    }
} CANVAS_RENDERER_SPEC;

class CMeshRenderer;
typedef struct ENGINE_DLL tagMeshRendererSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::MESH_RENDERER)

    ASSET_GUID  meshGUID{};
    ASSET_GUID  materialGUID{};

    uint32_t    flags = RF_NONE;
    RENDER_LAYER layer = RENDER_LAYER::NONBLEND;

    _float      sortZ = 0.f;
    _bool       bEnabled = true;

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagMeshRendererSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());

        if (meshGUID.Is_Valid())
            j["MeshGUID"] = meshGUID.To_String_Utf8();
        if (materialGUID.Is_Valid())
            j["MaterialGUID"] = materialGUID.To_String_Utf8();

        j["Flags"] = flags;
        j["Layer"] = SCAST(uint32_t, layer);
        j["SortZ"] = sortZ;

        if (!bEnabled)
            j["bEnabled"] = bEnabled;
    }

    _bool FromJson(const json& j) override
    {
        if (j.contains("Type"))
        {
            const auto& t = j.at("Type");
            if (!t.is_number_unsigned())
                return false;
            if (t.get<_uint>() != SCAST(_uint, Get_Type()))
                return false;
        }

        auto read_guid = [&](const char* key, ASSET_GUID& out) -> bool
            {
                if (!j.contains(key)) return true;
                const auto& v = j.at(key);
                if (!v.is_string()) return false;
                return ASSET_GUID::Try_Utf8_To_GUID(v, out);
            };

        auto read_vec4 = [&](const char* key, _float4& out) -> bool
            {
                if (!j.contains(key)) return true;
                const auto& a = j.at(key);
                if (!a.is_array() || a.size() != 4) return false;
                for (int i = 0; i < 4; ++i) if (!a[i].is_number()) return false;
                out = { a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>(), a[3].get<_float>() };
                return true;
            };

        auto read_rect4 = [&](const char* key, RECT_F& out) -> bool
            {
                _float4 v{};
                if (!read_vec4(key, v)) return false;
                if (!j.contains(key)) return true; 
                out = { v.x, v.y, v.z, v.w };
                return true;
            };

        if (!read_guid("MeshGUID", meshGUID)) return false; 
        if (!read_guid("MaterialGUID", materialGUID)) return false;

        if (j.contains("flags"))
        {
            const auto& v = j.at("flags");
            if (!v.is_number_unsigned()) return false;
            flags = v.get<uint32_t>();
        }

        if (j.contains("layer"))
        {
            const auto& v = j.at("layer");
            if (!v.is_number_unsigned()) return false;
            layer = (RENDER_LAYER)v.get<uint32_t>();
        }

        if (j.contains("sortZ"))
        {
            const auto& v = j.at("sortZ");
            if (!v.is_number()) return false;
            sortZ = v.get<_float>();
        }

        if (j.contains("bEnabled"))
        {
            const auto& v = j.at("bEnabled");
            if (!v.is_boolean()) return false;
            bEnabled = v.get<_bool>();
        }
        else
        {
            bEnabled = true;
        }

        auto clamp01 = [](_float x) -> _float { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };

        sortZ = clamp01(sortZ);

        return true;
    }
} MESH_RENDERER_SPEC;

typedef struct tagScriptSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::SCRIPT)

    ASSET_GUID      scriptGuid{};     // 스크립트 타입(또는 스크립트 에셋) GUID
    _bool           bEnable = true;
    uint8_t         pad[3] = {};

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagScriptSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["ScriptGuid"] = scriptGuid.To_String_Utf8();
        j["Enabled"] = bEnable;
    }

    _bool FromJson(const json& j) override
    {
        try
        {
            if (j.contains("ScriptGuid"))
                 ASSET_GUID::Try_Utf8_To_GUID(j["ScriptGuid"], scriptGuid);
            if (j.contains("Enabled"))
                bEnable = j["Enabled"];
            return true;
        }
        catch (...) { return false; }
    }
} SCRIPT_SPEC;

typedef struct ENGINE_DLL tagCameraSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::CAMERA)

    _bool   bOrthographic = false;
    _float  fovy = 60.f;          /* degrees */
    _float  orthoSize = 5.f;
    _float  aspect = 16.f / 9.f;
    _float  zNear = 0.1f;
    _float  zFar = 1000.f;

    uint32_t layerMask = 0xFFFFFFFFu;
    uint8_t  bEnabled = 1;

    [[nodiscard]]
    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagCameraSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["Ortho"] = bOrthographic;
        j["Fovy"] = fovy;
        j["OrthoSize"] = orthoSize;
        j["Aspect"] = aspect;
        j["Near"] = zNear;
        j["Far"] = zFar;
        j["LayerMask"] = layerMask;
        j["Enabled"] = bEnabled;
    }

    _bool FromJson(const json& j) override
    {
        try
        {
            if (j.contains("Ortho")) bOrthographic = j["Ortho"];
            if (j.contains("Fovy")) fovy = j["Fovy"];
            if (j.contains("OrthoSize")) orthoSize = j["OrthoSize"];
            if (j.contains("Aspect")) aspect = j["Aspect"];
            if (j.contains("Near")) zNear = j["Near"];
            if (j.contains("Far")) zFar = j["Far"];
            if (j.contains("LayerMask")) layerMask = j["LayerMask"];
            if (j.contains("Enabled")) bEnabled = j["Enabled"];
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
} CAMERA_SPEC;

typedef struct ENGINE_DLL tagLightSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::LIGHT)

        OBJECT_HANDLE hObject{};

    LIGHT_TYPE type = LIGHT_TYPE::DIRECTIONAL;

    _float4     vColor = { 1.f, 1.f, 1.f, 0.f };
    _float4     vDirection = { 0.f, -1.f, 0.f, 0.f };
    _float4     vPosition{}; 
    _float      fRange = 10.f;
    _float      spotAngle = 30.f;

    _float4     vDiffuse{ 1.f, 1.f, 1.f , 1.f };
    _float4     vAmbient{ 1.f, 1.f, 1.f , 1.f };
    _float4     vSpecular{ 1.f, 1.f, 1.f , 1.f };

    uint8_t     bEnabled = 1;
    _bool       dirty = true;

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagLightSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());

        j["LightType"] = SCAST(_uint, type);
        j["Enabled"] = bEnabled;
        j["Range"] = fRange;
        j["SpotAngle"] = spotAngle;

        j["Color"] = { vColor.x, vColor.y, vColor.z, vColor.w };
        j["Direction"] = { vDirection.x, vDirection.y, vDirection.z, 0.f };
        j["Position"] = { vPosition.x, vPosition.y, vPosition.z, 1.f };

        j["Diffuse"] = { vDiffuse.x, vDiffuse.y, vDiffuse.z, vDiffuse.w };
        j["Ambient"] = { vAmbient.x, vAmbient.y, vAmbient.z, vAmbient.w };
        j["Specular"] = { vSpecular.x, vSpecular.y, vSpecular.z, vSpecular.w };
    }

    _bool FromJson(const json& j) override
    {
        try
        {
            if (j.contains("LightType")) type = SCAST(LIGHT_TYPE, (_uint)j["LightType"]);
            if (j.contains("Enabled"))   bEnabled = j["Enabled"];
            if (j.contains("Range"))     fRange = j["Range"];
            if (j.contains("SpotAngle")) spotAngle = j["SpotAngle"];

            auto read_vec4 = [&](const char* key, _float4& out)->_bool
                {
                    if (!j.contains(key) || !j[key].is_array() || j[key].size() < 4) return false;
                    out = { j[key][0], j[key][1], j[key][2], j[key][3] };
                    return true;
                };

            if (j.contains("Color") && j["Color"].is_array() && j["Color"].size() >= 4)
                vColor = { j["Color"][0], j["Color"][1], j["Color"][2], j["Color"][3] };

            if (j.contains("Direction") && j["Direction"].is_array() && j["Direction"].size() >= 3)
                vDirection = { j["Direction"][0], j["Direction"][1], j["Direction"][2], 0.f };

            if (j.contains("Position") && j["Position"].is_array() && j["Position"].size() >= 3)
                vPosition = { j["Position"][0], j["Position"][1], j["Position"][2], 1.f };

            if (!read_vec4("Diffuse", vDiffuse))  return false;
            if (!read_vec4("Ambient", vAmbient))  return false;
            if (!read_vec4("Specular", vSpecular)) return false;

            return true;
        }
        catch (...)
        {
            return false;
        }
    }
} LIGHT_SPEC;

typedef struct tagUIImageSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::UI_IMAGE)

    RECT_F      rcUV{};
    _float4     color{ 1,1,1,1 };

    ASSET_GUID  textureGuid{};

    _bool       bEnable = false;
    uint8_t     visualPriority = 0;
    uint8_t     pad1[2] = {};

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagUIImageSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["Enabled"] = bEnable;

        Guid_ToJson(j["TextureGuid"], textureGuid);
        RectF_ToJson(j["UV"], rcUV);
        Float4_ToJson(j["Color"], color);
        j["VisualPriority"] = visualPriority;
    }

    _bool FromJson(const json& j) override
    {
        try
        {
            if (j.contains("Enabled"))        bEnable = j["Enabled"];
            if (j.contains("TextureGuid"))    Guid_FromJson(j["TextureGuid"], textureGuid);
            if (j.contains("UV"))             RectF_FromJson(j["UV"], rcUV);
            if (j.contains("Color"))          Float4_FromJson(j["Color"], color);
            if (j.contains("VisualPriority")) visualPriority = j["VisualPriority"];
            return true;
        }
        catch (...) { return false; }
    }
} UI_IMAGE_SPEC;


typedef struct tagUIButtonSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::UI_BUTTON)

    _float4 normal{ 1,1,1,1 };
    _float4 hover{ 1,1,1,1 };
    _float4 pressed{ 1,1,1,1 };
    _float4 disabled{ 1,1,1,1 };

    ASSET_GUID normalTexGuid{};
    RECT_F     normalUV{};
    ASSET_GUID hoverTexGuid{};
    RECT_F     hoverUV{};
    ASSET_GUID pressedTexGuid{};
    RECT_F     pressedUV{};

    uint32_t   onClickEventId = 0;
    uint8_t    visualPriority = 10;
    _bool      bEnable = false;
    _bool      bInteractable = true;
    uint8_t    pad1[1] = {};

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagUIButtonSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["Enabled"] = bEnable;
        j["Interactable"] = bInteractable;

        Float4_ToJson(j["NormalColor"], normal);
        Float4_ToJson(j["HoverColor"], hover);
        Float4_ToJson(j["PressedColor"], pressed);
        Float4_ToJson(j["DisabledColor"], disabled);

        Guid_ToJson(j["NormalTexGuid"], normalTexGuid);  RectF_ToJson(j["NormalUV"], normalUV);
        Guid_ToJson(j["HoverTexGuid"], hoverTexGuid);    RectF_ToJson(j["HoverUV"], hoverUV);
        Guid_ToJson(j["PressedTexGuid"], pressedTexGuid); RectF_ToJson(j["PressedUV"], pressedUV);

        j["VisualPriority"] = visualPriority;
        j["OnClickEventId"] = onClickEventId;
    }

    _bool FromJson(const json& j) override
    {
        try
        {
            if (j.contains("Enabled"))        bEnable = j["Enabled"];
            if (j.contains("Interactable"))   bInteractable = j["Interactable"];

            if (j.contains("NormalColor"))    Float4_FromJson(j["NormalColor"], normal);
            if (j.contains("HoverColor"))     Float4_FromJson(j["HoverColor"], hover);
            if (j.contains("PressedColor"))   Float4_FromJson(j["PressedColor"], pressed);
            if (j.contains("DisabledColor"))  Float4_FromJson(j["DisabledColor"], disabled);

            if (j.contains("NormalTexGuid"))  Guid_FromJson(j["NormalTexGuid"], normalTexGuid);
            if (j.contains("NormalUV"))       RectF_FromJson(j["NormalUV"], normalUV);

            if (j.contains("HoverTexGuid"))   Guid_FromJson(j["HoverTexGuid"], hoverTexGuid);
            if (j.contains("HoverUV"))        RectF_FromJson(j["HoverUV"], hoverUV);

            if (j.contains("PressedTexGuid")) Guid_FromJson(j["PressedTexGuid"], pressedTexGuid);
            if (j.contains("PressedUV"))      RectF_FromJson(j["PressedUV"], pressedUV);

            if (j.contains("VisualPriority")) visualPriority = j["VisualPriority"];
            if (j.contains("OnClickEventId")) onClickEventId = j["OnClickEventId"];

            return true;
        }
        catch (...) { return false; }
    }
} UI_BUTTON_SPEC;











NS_END
