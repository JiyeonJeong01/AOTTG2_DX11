#pragma once
#include "Engine_Define.h"
#include "Identity.h"

NS_BEGIN(Engine)

typedef struct tagScriptObjectRef final
{
    INSTANCE_UUID   tUUID{};
    OBJECT_HANDLE   hObject{};

    void Clear()
    {
        tUUID = INSTANCE_UUID{};
        hObject = OBJECT_HANDLE{};
    }

    _bool Is_Valid() const
    {
        return hObject.Is_Valid();
    }
} SCRIPT_OBJECT_REF;

typedef struct tagScriptFieldDesc final
{
    std::string         strName;
    SCRIPT_FIELD_TYPE   eType = SCRIPT_FIELD_TYPE::INT;
    size_t              iOffset = 0;
} SCRIPT_FIELD_DESC;

typedef struct tagScriptReflectionInfo final
{
    std::string                     strClassName;
    std::vector<SCRIPT_FIELD_DESC>  vecFields;
} SCRIPT_REFLECTION_INFO;

#pragma region Utils

inline void ScriptField_ToJson_Int(json& j, const _int& value)
{
    j = value;
}

inline void ScriptField_ToJson_Float(json& j, const _float& value)
{
    j = value;
}

inline void ScriptField_ToJson_Float2(json& j, const _float2& value)
{
    j = json::array({ value.x, value.y });
}

inline void ScriptField_ToJson_Float3(json& j, const _float3& value)
{
    j = json::array({ value.x, value.y, value.z });
}

inline void ScriptField_ToJson_Float4(json& j, const _float4& value)
{
    j = json::array({ value.x, value.y, value.z, value.w });
}

inline void ScriptField_ToJson_ObjectRef(json& j, const SCRIPT_OBJECT_REF& value)
{
    j["UUID"] = value.tUUID.To_String_Utf8();
}

inline void ScriptField_ToJson_AssetGUID(json& j, const ASSET_GUID& value)
{
    j["GUID"] = value.To_String_Utf8();
}

inline void ScriptField_ToJson_Char32(json& j, const char* value)
{
    j["Value"] = value;
}

inline _bool ScriptField_FromJson_Int(const json& j, _int& value)
{
    if (false == j.is_number_integer())
        return false;

    value = j.get<_int>();
    return true;
}

inline _bool ScriptField_FromJson_Float(const json& j, _float& value)
{
    if (false == j.is_number())
        return false;

    value = j.get<_float>();
    return true;
}

inline _bool ScriptField_FromJson_Float2(const json& j, _float2& value)
{
    if (false == j.is_array() || j.size() != 2)
        return false;

    value.x = j[0].get<_float>();
    value.y = j[1].get<_float>();

    return true;
}

inline _bool ScriptField_FromJson_Float3(const json& j, _float3& value)
{
    if (false == j.is_array() || j.size() != 3)
        return false;

    value.x = j[0].get<_float>();
    value.y = j[1].get<_float>();
    value.z = j[2].get<_float>();
    return true;
}

inline _bool ScriptField_FromJson_Float4(const json& j, _float4& value)
{
    if (false == j.is_array() || j.size() != 4)
        return false;

    value.x = j[0].get<_float>();
    value.y = j[1].get<_float>();
    value.z = j[2].get<_float>();
    value.w = j[3].get<_float>();
    return true;
}

inline _bool ScriptField_FromJson_ObjectRef(const json& j, SCRIPT_OBJECT_REF& value)
{
    if (false == j.is_object())
        return false;

    if (false == j.contains("UUID"))
        return false;

    if (false == INSTANCE_UUID::Try_Utf8_To_UUID(j["UUID"], value.tUUID))
        return false;

    value.hObject = OBJECT_HANDLE{};

    return true;
}

inline _bool ScriptField_FromJson_AssetGUID(const json& j, ASSET_GUID& value)
{
    if (false == j.is_object())
        return false;

    if (false == j.contains("GUID"))
        return false;

    if (false == ASSET_GUID::Try_Utf8_To_GUID(j["GUID"], value))
        return false;

    return true;
}

inline bool ScriptField_FromJson_Char32(const json& j, char* value)
{
    const json* target = &j;

    if (j.is_object() && j.contains("Value")) {
        target = &j["Value"];
    }

    if (!target->is_string()) {
        return false;
    }

    std::string s = target->get<std::string>();
    size_t copyLen = s.length() < 31 ? s.length() : 31;

    memcpy(value, s.c_str(), copyLen);
    value[copyLen] = '\0';

    return true;
}

#pragma endregion

#define SCRIPT_FIELDS_BEGIN(ClassName)                                  \
public :                                                                \
    static const SCRIPT_REFLECTION_INFO& Get_Static_Reflection_Info()   \
    {                                                                   \
        static SCRIPT_REFLECTION_INFO s_Info;                           \
        static _bool s_bInitialized = false;                            \
        if (false == s_bInitialized)                                    \
        {                                                               \
            s_bInitialized = true;                                      \
            s_Info.strClassName = #ClassName;                           \
            using SelfType = ClassName;                                 



#define SCRIPT_FIELD_INT(Member)                                        \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::INT, offsetof(SelfType, Member) });

#define SCRIPT_FIELD_FLOAT(Member)                                      \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::FLOAT, offsetof(SelfType, Member) });

#define SCRIPT_FIELD_FLOAT2(Member)                                     \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::FLOAT2, offsetof(SelfType, Member) });

#define SCRIPT_FIELD_FLOAT3(Member)                                     \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::FLOAT3, offsetof(SelfType, Member) });

#define SCRIPT_FIELD_FLOAT4(Member)                                     \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::FLOAT4, offsetof(SelfType, Member) });

#define SCRIPT_FIELD_OBJECT_REF(Member)                                 \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::OBJECT_REF, offsetof(SelfType, Member) });

#define SCRIPT_FIELD_ASSET_GUID(Member)                                 \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::ASSET_GUID, offsetof(SelfType, Member) });

#define SCRIPT_FIELD_CHAR(Member)                                 \
            s_Info.vecFields.push_back({ #Member, SCRIPT_FIELD_TYPE::CHAR32, offsetof(SelfType, Member) });

#define SCRIPT_FIELDS_END(ClassName)                                    \
        }                                                               \
        return s_Info;                                                  \
    }                                                                   \
    const SCRIPT_REFLECTION_INFO* Get_Reflection_Info() const override  \
    {                                                                   \
        return &ClassName::Get_Static_Reflection_Info();                \
    }

NS_END
