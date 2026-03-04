#pragma once
#include "Engine_Define.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)

static inline void RectF_ToJson(json& j, const RECT_F& rc)
{
    j = json::array({ rc.fLeft, rc.fTop, rc.fRight, rc.fBottom });
}
static inline _bool RectF_FromJson(const json& j, RECT_F& out)
{
    try
    {
        if (!j.is_array() || j.size() != 4) return false;
        out.fLeft = j[0];
        out.fTop = j[1];
        out.fRight = j[2];
        out.fBottom = j[3];
        return true;
    }
    catch (...) { return false; }
}
static inline void Float4_ToJson(json& j, const _float4& v)
{
    j = json::array({ v.x, v.y, v.z, v.w });
}
static inline _bool Float4_FromJson(const json& j, _float4& out)
{
    try
    {
        if (!j.is_array() || j.size() != 4) return false;
        out.x = j[0];
        out.y = j[1];
        out.z = j[2];
        out.w = j[3];
        return true;
    }
    catch (...) { return false; }
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
    catch (...) { return false; }
}

NS_END
