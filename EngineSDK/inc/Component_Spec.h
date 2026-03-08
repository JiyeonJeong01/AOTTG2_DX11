#pragma once
#include "Physics_Struct.h"
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
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_Vec3(j, "Position", vPosition)) return false;
        if (!Read_Vec4(j, "Rotation", vRotationQuat)) return false;
        if (!Read_Vec3(j, "Scale", vScale)) return false;

        /* 안전한 읽기 */
        {
            /* scale 0 방지 */ 
            const _float eps = 1e-6f;
            if (fabs(vScale.x) < eps) vScale.x = 1.f;
            if (fabs(vScale.y) < eps) vScale.y = 1.f;
            if (fabs(vScale.z) < eps) vScale.z = 1.f;

            const _float len2 =
                vRotationQuat.x * vRotationQuat.x +
                vRotationQuat.y * vRotationQuat.y +
                vRotationQuat.z * vRotationQuat.z +
                vRotationQuat.w * vRotationQuat.w;

            /* vRotationQuat 정규화 */
            if (len2 < eps)
                vRotationQuat = { 0,0,0,1 };
            else
            {
                const _float invLen = 1.f / sqrtf(len2);
                vRotationQuat.x *= invLen;
                vRotationQuat.y *= invLen;
                vRotationQuat.z *= invLen;
                vRotationQuat.w *= invLen;
            }
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
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_Vec2(j, "PosPx", vPosPx)) return false;
        if (!Read_Vec2(j, "SizePx", vSizePx)) return false;

        /* 사이즈 0/음수 방지 */ 
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

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["MaterialGUID"] = materialGUID.To_String_Utf8();
        j["TextureGUID"] = textureGUID.To_String_Utf8();
        j["vColor"] = { vColor.x, vColor.y, vColor.z, vColor.w };
        j["rcUV"] = { rcUV.fLeft, rcUV.fTop, rcUV.fRight, rcUV.fBottom };
        j["rcClip"] = { rcClip.fLeft, rcClip.fTop, rcClip.fRight, rcClip.fBottom };
        j["flags"] = flags;
        j["layer"] = SCAST(_uint, layer);
        j["sortZ"] = sortZ;
        j["bEnabled"] = bEnabled;
    }

    _bool FromJson(const json& j) override
    {
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_GUID(j, "MaterialGUID", materialGUID)) return false;
        if (!Read_GUID(j, "TextureGUID", textureGUID)) return false;

        if (!Read_Vec4(j, "vColor", vColor)) return false;
        if (!Read_RECTF(j, "rcUV", rcUV)) return false;
        if (!Read_RECTF(j, "rcClip", rcClip)) return false;

        if (!j.contains("flags")) return false;
        {
            const auto& v = j.at("flags");
            if (!v.is_number_unsigned()) return false;
            flags = v.get<uint32_t>();
        }

        if (!j.contains("layer")) return false;
        {
            const auto& v = j.at("layer");
            if (!v.is_number_unsigned()) return false;
            layer = SCAST(RENDER_LAYER, v.get<uint32_t>());
        }

        if (!j.contains("sortZ")) return false;
        {
            const auto& v = j.at("sortZ");
            if (!v.is_number()) return false;
            sortZ = v.get<_float>();
        }

        if (!j.contains("bEnabled")) return false;
        {
            const auto& v = j.at("bEnabled");
            if (!v.is_boolean()) return false;
            bEnabled = v.get<_bool>();
        }

        auto clamp01 = [](_float x) -> _float
            {
                return x < 0.f ? 0.f : (x > 1.f ? 1.f : x);
            };

        vColor.x = clamp01(vColor.x);
        vColor.y = clamp01(vColor.y);
        vColor.z = clamp01(vColor.z);
        vColor.w = clamp01(vColor.w);

        rcUV.fLeft = clamp01(rcUV.fLeft);
        rcUV.fTop = clamp01(rcUV.fTop);
        rcUV.fRight = clamp01(rcUV.fRight);
        rcUV.fBottom = clamp01(rcUV.fBottom);

        if (rcUV.fLeft > rcUV.fRight) std::swap(rcUV.fLeft, rcUV.fRight);
        if (rcUV.fTop > rcUV.fBottom) std::swap(rcUV.fTop, rcUV.fBottom);

        sortZ = Clamp01(sortZ);

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
        j["MeshGUID"] = meshGUID.To_String_Utf8();
        j["MaterialGUID"] = materialGUID.To_String_Utf8();
        j["Flags"] = flags;
        j["Layer"] = SCAST(uint32_t, layer);
        j["SortZ"] = sortZ;
        j["bEnabled"] = bEnabled;
    }

    _bool FromJson(const json& j) override
    {
        if (!Read_SpecType(j, Get_Type()))
            return false;
        if (!Read_GUID(j, "MeshGUID", meshGUID)) return false;
        if (!Read_GUID(j, "MaterialGUID", materialGUID)) return false;
        if (!Read_UInt(j, "Flags", flags))
            return false;
        {
            uint32_t layerValue = 0;
            if (!Read_UInt(j, "Layer", layerValue))
                return false;
            layer = SCAST(RENDER_LAYER, layerValue);
        }
        if (!Read_Float(j, "SortZ", sortZ))
            return false;
        if (!Read_Bool(j, "bEnabled", bEnabled))
            return false;
        sortZ = Clamp01(sortZ);
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
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_GUID(j, "ScriptGuid", scriptGuid))
            return false;
        if (!Read_Bool(j, "Enabled", bEnable))
            return false;

        return true;
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
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_Bool(j, "Ortho", bOrthographic))
            return false;
        if (!Read_Float(j, "Fovy", fovy))
            return false;
        if (!Read_Float(j, "OrthoSize", orthoSize))
            return false;
        if (!Read_Float(j, "Aspect", aspect))
            return false;
        if (!Read_Float(j, "Near", zNear))
            return false;
        if (!Read_Float(j, "Far", zFar))
            return false;
        if (!Read_UInt(j, "LayerMask", layerMask))
            return false;

        {
            uint32_t iEnabled = 0;
            if (!Read_UInt(j, "Enabled", iEnabled))
                return false;

            bEnabled = (iEnabled != 0) ? 1 : 0;
        }

        if (!(fovy > 0.f))       fovy = 60.f;
        if (!(orthoSize > 0.f))  orthoSize = 5.f;
        if (!(aspect > 0.f))     aspect = 16.f / 9.f;
        if (!(zNear > 0.f))      zNear = 0.1f;
        if (!(zFar > zNear))     zFar = zNear + 1000.f;

        return true;
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
        j["Enabled"] = SCAST(_uint, bEnabled);
        j["Range"] = fRange;
        j["SpotAngle"] = spotAngle;

        j["Color"] = { vColor.x, vColor.y, vColor.z, vColor.w };
        j["Direction"] = { vDirection.x, vDirection.y, vDirection.z, vDirection.w };
        j["Position"] = { vPosition.x, vPosition.y, vPosition.z, vPosition.w };

        j["Diffuse"] = { vDiffuse.x, vDiffuse.y, vDiffuse.z, vDiffuse.w };
        j["Ambient"] = { vAmbient.x, vAmbient.y, vAmbient.z, vAmbient.w };
        j["Specular"] = { vSpecular.x, vSpecular.y, vSpecular.z, vSpecular.w };
    }

    _bool FromJson(const json& j) override
    {
        if (!Read_SpecType(j, Get_Type()))
            return false;

        {
            uint32_t iType = 0;
            if (!Read_UInt(j, "LightType", iType))
                return false;
            type = SCAST(LIGHT_TYPE, iType);
        }

        {
            uint32_t iEnabled = 0;
            if (!Read_UInt(j, "Enabled", iEnabled))
                return false;
            bEnabled = (iEnabled != 0) ? 1 : 0;
        }

        if (!Read_Float(j, "Range", fRange))
            return false;
        if (!Read_Float(j, "SpotAngle", spotAngle))
            return false;

        if (!Read_Vec4(j, "Color", vColor))
            return false;
        if (!Read_Vec4(j, "Direction", vDirection))
            return false;
        if (!Read_Vec4(j, "Position", vPosition))
            return false;

        if (!Read_Vec4(j, "Diffuse", vDiffuse))
            return false;
        if (!Read_Vec4(j, "Ambient", vAmbient))
            return false;
        if (!Read_Vec4(j, "Specular", vSpecular))
            return false;

        vColor.x = Clamp01(vColor.x);
        vColor.y = Clamp01(vColor.y);
        vColor.z = Clamp01(vColor.z);
        vColor.w = Clamp01(vColor.w);

        vDiffuse.x = Clamp01(vDiffuse.x);
        vDiffuse.y = Clamp01(vDiffuse.y);
        vDiffuse.z = Clamp01(vDiffuse.z);
        vDiffuse.w = Clamp01(vDiffuse.w);

        vAmbient.x = Clamp01(vAmbient.x);
        vAmbient.y = Clamp01(vAmbient.y);
        vAmbient.z = Clamp01(vAmbient.z);
        vAmbient.w = Clamp01(vAmbient.w);

        vSpecular.x = Clamp01(vSpecular.x);
        vSpecular.y = Clamp01(vSpecular.y);
        vSpecular.z = Clamp01(vSpecular.z);
        vSpecular.w = Clamp01(vSpecular.w);

        if (!(fRange > 0.f))
            fRange = 10.f;

        if (!(spotAngle > 0.f))
            spotAngle = 30.f;

        dirty = true;

        return true;
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
        j["TextureGuid"] = textureGuid.To_String_Utf8();
        j["UV"] = { rcUV.fLeft, rcUV.fTop, rcUV.fRight, rcUV.fBottom };
        j["Color"] = { color.x, color.y, color.z, color.w };
        j["VisualPriority"] = SCAST(_uint, visualPriority);
    }

    _bool FromJson(const json& j) override
    {
        if (!Read_SpecType(j, Get_Type()))
            return false;
        if (!Read_Bool(j, "Enabled", bEnable))
            return false;
        if (!Read_GUID(j, "TextureGuid", textureGuid))
            return false;
        if (!Read_RECTF(j, "UV", rcUV))
            return false;
        if (!Read_Vec4(j, "Color", color))
            return false;

        {
            uint32_t iVisualPriority = 0;
            if (!Read_UInt(j, "VisualPriority", iVisualPriority))
                return false;

            visualPriority = SCAST(uint8_t, iVisualPriority);
        }

        color.x = Clamp01(color.x);
        color.y = Clamp01(color.y);
        color.z = Clamp01(color.z);
        color.w = Clamp01(color.w);

        rcUV.fLeft = Clamp01(rcUV.fLeft);
        rcUV.fTop = Clamp01(rcUV.fTop);
        rcUV.fRight = Clamp01(rcUV.fRight);
        rcUV.fBottom = Clamp01(rcUV.fBottom);

        if (rcUV.fLeft > rcUV.fRight)
            std::swap(rcUV.fLeft, rcUV.fRight);
        if (rcUV.fTop > rcUV.fBottom)
            std::swap(rcUV.fTop, rcUV.fBottom);
        return true;
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

        j["NormalColor"] = { normal.x, normal.y, normal.z, normal.w };
        j["HoverColor"] = { hover.x, hover.y, hover.z, hover.w };
        j["PressedColor"] = { pressed.x, pressed.y, pressed.z, pressed.w };
        j["DisabledColor"] = { disabled.x, disabled.y, disabled.z, disabled.w };

        j["NormalTexGuid"] = normalTexGuid.To_String_Utf8();
        j["NormalUV"] = { normalUV.fLeft, normalUV.fTop, normalUV.fRight, normalUV.fBottom };

        j["HoverTexGuid"] = hoverTexGuid.To_String_Utf8();
        j["HoverUV"] = { hoverUV.fLeft, hoverUV.fTop, hoverUV.fRight, hoverUV.fBottom };

        j["PressedTexGuid"] = pressedTexGuid.To_String_Utf8();
        j["PressedUV"] = { pressedUV.fLeft, pressedUV.fTop, pressedUV.fRight, pressedUV.fBottom };

        j["VisualPriority"] = SCAST(_uint, visualPriority);
        j["OnClickEventId"] = onClickEventId;
    }

    _bool FromJson(const json& j) override
    {
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_Bool(j, "Enabled", bEnable))
            return false;
        if (!Read_Bool(j, "Interactable", bInteractable))
            return false;

        if (!Read_Vec4(j, "NormalColor", normal))
            return false;
        if (!Read_Vec4(j, "HoverColor", hover))
            return false;
        if (!Read_Vec4(j, "PressedColor", pressed))
            return false;
        if (!Read_Vec4(j, "DisabledColor", disabled))
            return false;

        if (!Read_GUID(j, "NormalTexGuid", normalTexGuid))
            return false;
        if (!Read_RECTF(j, "NormalUV", normalUV))
            return false;

        if (!Read_GUID(j, "HoverTexGuid", hoverTexGuid))
            return false;
        if (!Read_RECTF(j, "HoverUV", hoverUV))
            return false;

        if (!Read_GUID(j, "PressedTexGuid", pressedTexGuid))
            return false;
        if (!Read_RECTF(j, "PressedUV", pressedUV))
            return false;

        if (!Read_UInt(j, "OnClickEventId", onClickEventId))
            return false;

        {
            uint32_t iVisualPriority = 0;
            if (!Read_UInt(j, "VisualPriority", iVisualPriority))
                return false;

            if (iVisualPriority > 255u)
                return false;

            visualPriority = SCAST(uint8_t, iVisualPriority);
        }

        Sanitize_Color(normal);
        Sanitize_Color(hover);
        Sanitize_Color(pressed);
        Sanitize_Color(disabled);

        Sanitize_UVRect(normalUV);
        Sanitize_UVRect(hoverUV);
        Sanitize_UVRect(pressedUV);

        return true;
    }
} UI_BUTTON_SPEC;


typedef struct ENGINE_DLL tagColliderSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::COLLIDER)

    _bool       bEnable = false;
    BODY_TYPE   eColType{ BODY_TYPE::STATIC };
    _bool       bOnCol{ false };

    SHAPE       eShape{ SHAPE::END };
    _float3     vOffset{ 0.f, 0.f, 0.f };

    _float3     vHalfExtentsLocal{ 0.5f, 0.5f, 0.5f };      /* BOX */
    _float      fRadiusLocal = 0.5f;                                /* SPHERE */

    _float3     vNormalLocal{ 0.f, 1.f, 0.f };              /* PLANE */
    _float      fDistance = 0.f;
    _bool       bInfinite = true;
    _float2     vDimension{ 1.f, 1.f };

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagColliderSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["Enabled"] = bEnable;
        j["BodyType"] = SCAST(_uint, eColType);
        j["OnCol"] = bOnCol;
        j["Shape"] = SCAST(_uint, eShape);
        j["Offset"] = { vOffset.x, vOffset.y, vOffset.z };

        switch (eShape)
        {
        case SHAPE::BOX:
            j["HalfExtentsLocal"] = { vHalfExtentsLocal.x, vHalfExtentsLocal.y, vHalfExtentsLocal.z };
            break;

        case SHAPE::SPHERE:
            j["RadiusLocal"] = fRadiusLocal;
            break;

        case SHAPE::PLANE:
            j["NormalLocal"] = { vNormalLocal.x, vNormalLocal.y, vNormalLocal.z };
            j["Distance"] = fDistance;
            j["Infinite"] = bInfinite;
            j["Dimension"] = { vDimension.x, vDimension.y };
            break;

        default:
            break;
        }
    }

    _bool FromJson(const json& j) override
    {
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_Bool(j, "Enabled", bEnable))
            return false;

        {
            uint32_t iBodyType = 0;
            if (!Read_UInt(j, "BodyType", iBodyType))
                return false;
            eColType = SCAST(BODY_TYPE, iBodyType);
        }

        if (!Read_Bool(j, "OnCol", bOnCol))
            return false;

        {
            uint32_t iShape = 0;
            if (!Read_UInt(j, "Shape", iShape))
                return false;
            eShape = SCAST(SHAPE, iShape);
        }

        if (!Read_Vec3(j, "Offset", vOffset))
            return false;

        switch (eShape)
        {
        case SHAPE::BOX:
            if (!Read_Vec3(j, "HalfExtentsLocal", vHalfExtentsLocal))
                return false;

            if (!(vHalfExtentsLocal.x > 0.f)) vHalfExtentsLocal.x = 0.5f;
            if (!(vHalfExtentsLocal.y > 0.f)) vHalfExtentsLocal.y = 0.5f;
            if (!(vHalfExtentsLocal.z > 0.f)) vHalfExtentsLocal.z = 0.5f;
            break;

        case SHAPE::SPHERE:
            if (!Read_Float(j, "RadiusLocal", fRadiusLocal))
                return false;

            if (!(fRadiusLocal > 0.f)) fRadiusLocal = 0.5f;
            break;

        case SHAPE::PLANE:
            if (!Read_Vec3(j, "NormalLocal", vNormalLocal))
                return false;
            if (!Read_Float(j, "Distance", fDistance))
                return false;
            if (!Read_Bool(j, "Infinite", bInfinite))
                return false;
            if (!Read_Vec2(j, "Dimension", vDimension))
                return false;

            if (!(vDimension.x > 0.f)) vDimension.x = 1.f;
            if (!(vDimension.y > 0.f)) vDimension.y = 1.f;
            break;

        default:
            return false;
        }

        return true;
    }
} COLLIDER_SPEC;

typedef struct ENGINE_DLL tagRigidbodySpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::RIGIDBODY)

    _bool       bEnable = true;
    _bool       bGravity = true;
    uint8_t     pad0[2] = {};

    SHAPE       eShape = SHAPE::END;
    BODY_TYPE   eBodyType = BODY_TYPE::STATIC;

    _float      fMass = 1.f;
    _float      fDrag = 0.f;
    _float      fAngularDrag = 0.f;

    _float      fRestitution = 0.f;
    _float      fFriction = 1.f;

    AXIS_MASK   tRotationLock{};
    AXIS_MASK   tPositionLock{};

    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagRigidbodySpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        j["Enabled"] = bEnable;
        j["Gravity"] = bGravity;
        j["Shape"] = SCAST(_uint, eShape);
        j["BodyType"] = SCAST(_uint, eBodyType);
        j["Mass"] = fMass;
        j["Drag"] = fDrag;
        j["AngularDrag"] = fAngularDrag;
        j["Restitution"] = fRestitution;
        j["Friction"] = fFriction;

        j["RotationLock"] = {
            tRotationLock.bX ? 1.f : 0.f,
            tRotationLock.bY ? 1.f : 0.f,
            tRotationLock.bZ ? 1.f : 0.f
        };

        j["PositionLock"] = {
            tPositionLock.bX ? 1.f : 0.f,
            tPositionLock.bY ? 1.f : 0.f,
            tPositionLock.bZ ? 1.f : 0.f
        };
    }

    _bool FromJson(const json& j) override
    {
        if (!Read_SpecType(j, Get_Type()))
            return false;

        if (!Read_Bool(j, "Enabled", bEnable))
            return false;

        if (!Read_Bool(j, "Gravity", bGravity))
            return false;

        {
            uint32_t iShape = 0;
            if (!Read_UInt(j, "Shape", iShape))
                return false;
            eShape = SCAST(SHAPE, iShape);
        }

        {
            uint32_t iBodyType = 0;
            if (!Read_UInt(j, "BodyType", iBodyType))
                return false;
            eBodyType = SCAST(BODY_TYPE, iBodyType);
        }

        if (!Read_Float(j, "Mass", fMass))
            return false;
        if (!Read_Float(j, "Drag", fDrag))
            return false;
        if (!Read_Float(j, "AngularDrag", fAngularDrag))
            return false;
        if (!Read_Float(j, "Restitution", fRestitution))
            return false;
        if (!Read_Float(j, "Friction", fFriction))
            return false;

        {
            _float3 vLock{};
            if (!Read_Vec3(j, "RotationLock", vLock))
                return false;

            tRotationLock.bX = (vLock.x != 0.f);
            tRotationLock.bY = (vLock.y != 0.f);
            tRotationLock.bZ = (vLock.z != 0.f);
        }

        {
            _float3 vLock{};
            if (!Read_Vec3(j, "PositionLock", vLock))
                return false;

            tPositionLock.bX = (vLock.x != 0.f);
            tPositionLock.bY = (vLock.y != 0.f);
            tPositionLock.bZ = (vLock.z != 0.f);
        }

        if (fMass < 0.f)          fMass = 0.f;
        if (fDrag < 0.f)          fDrag = 0.f;
        if (fAngularDrag < 0.f)   fAngularDrag = 0.f;

        fRestitution = Clamp01(fRestitution);
        if (fFriction < 0.f)      fFriction = 0.f;

        return true;
    }
} RIGIDBODY_SPEC;


NS_END
