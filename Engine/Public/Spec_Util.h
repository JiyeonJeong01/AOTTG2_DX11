#pragma once
#include "Engine_Define.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)

inline _bool Read_SpecType(const json& j, COMPONENT_TYPE eExpectedType)
{
    if (!j.contains("Type"))
        return false;

    const auto t = j["Type"];
    if (!t.is_number_unsigned())
        return false;

    const _uint type = t.get<_uint>();
    return type == SCAST(_uint, eExpectedType);
}

static inline void RectF_ToJson(json& j, const RECT_F& rc)
{
    j = json::array({ rc.fLeft, rc.fTop, rc.fRight, rc.fBottom });
}

inline _bool Read_RECTF(const json& j, const char* key, RECT_F& out)
{
    if (!j.contains(key))
        return false;

    const auto& a = j.at(key);
    if (!a.is_array() || a.size() != 4)
        return false;

    out.fLeft = a[0].get<_float>();
    out.fTop = a[1].get<_float>();
    out.fRight = a[2].get<_float>();
    out.fBottom = a[3].get<_float>();
    return true;
}
static inline void Guid_ToJson(json& j, const ASSET_GUID& g)
{
    j = g.To_String_Utf8();
}
static inline _bool Guid_FromJson(const json& j, ASSET_GUID& out)
{
    try
    {
        if (j.is_string())
            return ASSET_GUID::Try_Utf8_To_GUID(j, out);
        return false;
    }
    catch (...)
    {
        return false;
    }
}
inline _bool Read_GUID(const json& j, const char* key, ASSET_GUID& out)
{
    if (!j.contains(key))
        return false;

    return Guid_FromJson(j.at(key), out);
}

inline _bool Read_Vec2(const json& j, const char* key, _float2& out)
{
    if (!j.contains(key))
        return false;

    const auto& a = j.at(key);
    if (!a.is_array() || a.size() != 2)
        return false;

    if (!a[0].is_number() || !a[1].is_number())
        return false;

    out = { a[0].get<_float>(), a[1].get<_float>() };
    return true;
}

inline _bool Read_Vec3(const json& j, const char* key, _float3& out)
{
    if (!j.contains(key))
        return false;

    const auto& a = j.at(key);
    if (!a.is_array() || a.size() != 3)
        return false;

    if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number())
        return false;

    out = { a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>() };
    return true;
}

inline _bool Read_Vec4(const json& j, const char* key, _float4& out)
{
    if (!j.contains(key))
        return false;

    const auto& a = j.at(key);
    if (!a.is_array() || a.size() != 4)
        return false;

    if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number() || !a[3].is_number())
        return false;

    out = { a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>(), a[3].get<_float>() };
    return true;
}

inline _bool Read_UInt(const json& j, const char* key, uint32_t& out)
{
    if (!j.contains(key))
        return false;

    const auto& v = j.at(key);
    if (!v.is_number_unsigned())
        return false;

    out = v.get<uint32_t>();
    return true;
}

inline _bool Read_Float(const json& j, const char* key, _float& out)
{
    if (!j.contains(key))
        return false;

    const auto& v = j.at(key);
    if (!v.is_number())
        return false;

    out = v.get<_float>();
    return true;
}

inline _bool Read_Bool(const json& j, const char* key, _bool& out)
{
    if (!j.contains(key))
        return false;

    const auto& v = j.at(key);
    if (!v.is_boolean())
        return false;

    out = v.get<_bool>();
    return true;
}

inline _float Clamp01(_float x)
{
   return x < 0.f ? 0.f : (x > 1.f ? 1.f : x);
}

inline void Sanitize_Color(_float4& v)
{
    v.x = Clamp01(v.x);
    v.y = Clamp01(v.y);
    v.z = Clamp01(v.z);
    v.w = Clamp01(v.w);
}

inline void Sanitize_UVRect(RECT_F& rc)
{
    rc.fLeft = Clamp01(rc.fLeft);
    rc.fTop = Clamp01(rc.fTop);
    rc.fRight = Clamp01(rc.fRight);
    rc.fBottom = Clamp01(rc.fBottom);

    if (rc.fLeft > rc.fRight) std::swap(rc.fLeft, rc.fRight);
    if (rc.fTop > rc.fBottom) std::swap(rc.fTop, rc.fBottom);
}

NS_END
