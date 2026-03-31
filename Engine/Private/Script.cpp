#include "Script.h"
#include "GameObject_System.h"

void IScript::Save_Exposed_Fields(json& j) const
{
    const SCRIPT_REFLECTION_INFO* pInfo = Get_Reflection_Info();
    if (nullptr == pInfo)
        return;

    j["ClassName"] = pInfo->strClassName;
    j["Fields"] = json::array();

    for (const SCRIPT_FIELD_DESC& tDesc : pInfo->vecFields)
    {
        json jField;
        jField["Name"] = tDesc.strName;
        jField["Type"] = static_cast<uint32_t>(tDesc.eType);

        const void* pField = Get_Field_Ptr(tDesc);

        switch (tDesc.eType)
        {
        case SCRIPT_FIELD_TYPE::INT:
            ScriptField_ToJson_Int(jField["Value"], *reinterpret_cast<const int*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT:
            ScriptField_ToJson_Float(jField["Value"], *reinterpret_cast<const float*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT2:
            ScriptField_ToJson_Float2(jField["Value"], *reinterpret_cast<const _float2*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT3:
            ScriptField_ToJson_Float3(jField["Value"], *reinterpret_cast<const _float3*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT4:
            ScriptField_ToJson_Float4(jField["Value"], *reinterpret_cast<const _float4*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::OBJECT_REF:
            ScriptField_ToJson_ObjectRef(jField["Value"], *reinterpret_cast<const SCRIPT_OBJECT_REF*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::ASSET_GUID:
            ScriptField_ToJson_AssetGUID(jField["Value"], *reinterpret_cast<const ASSET_GUID*>(pField));
            break;

        default:
            continue;
        }

        j["Fields"].push_back(jField);
    }

}

_bool IScript::Load_Exposed_Fields(const json& j)
{
    const SCRIPT_REFLECTION_INFO* pInfo = Get_Reflection_Info();
    if (nullptr == pInfo)
        return false;

    if (false == j.is_object())
        return false;

    if (false == j.contains("Fields"))
        return false;

    const json& jFields = j["Fields"];
    if (false == jFields.is_array())
        return false;

    for (const json& jField : jFields)
    {
        if (false == jField.contains("Name") || false == jField.contains("Value"))
            continue;

        const std::string strName = jField["Name"].get<std::string>();

        const SCRIPT_FIELD_DESC* pMatched = nullptr;
        for (const SCRIPT_FIELD_DESC& tDesc : pInfo->vecFields)
        {
            if (tDesc.strName == strName)
            {
                pMatched = &tDesc;
                break;
            }
        }

        if (nullptr == pMatched)
            continue;

        void* pField = Get_Field_Ptr(*pMatched);
        const json& jValue = jField["Value"];

        switch (pMatched->eType)
        {
        case SCRIPT_FIELD_TYPE::INT:
            ScriptField_FromJson_Int(jValue, *reinterpret_cast<int*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT:
            ScriptField_FromJson_Float(jValue, *reinterpret_cast<float*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT2:
            ScriptField_FromJson_Float2(jValue, *reinterpret_cast<_float2*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT3:
            ScriptField_FromJson_Float3(jValue, *reinterpret_cast<_float3*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::FLOAT4:
            ScriptField_FromJson_Float4(jValue, *reinterpret_cast<_float4*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::OBJECT_REF:
            ScriptField_FromJson_ObjectRef(jValue, *reinterpret_cast<SCRIPT_OBJECT_REF*>(pField));
            break;

        case SCRIPT_FIELD_TYPE::ASSET_GUID:
            ScriptField_FromJson_AssetGUID(jValue, *reinterpret_cast<ASSET_GUID*>(pField));
            break;

        default:
            break;
        }
    }

    return true;
}

void IScript::Resolve_Exposed_ObjectRefs()
{
    const SCRIPT_REFLECTION_INFO* pInfo = Get_Reflection_Info();
    if (nullptr == pInfo)
        return;

    for (const SCRIPT_FIELD_DESC& tDesc : pInfo->vecFields)
    {
        if (tDesc.eType != SCRIPT_FIELD_TYPE::OBJECT_REF)
            continue;

        SCRIPT_OBJECT_REF* pRef = reinterpret_cast<SCRIPT_OBJECT_REF*>(Get_Field_Ptr(tDesc));
        if (nullptr == pRef)
            continue;

        if (pRef->tUUID == INSTANCE_UUID{})
        {
            pRef->hObject = OBJECT_HANDLE{};
            continue;
        }

        pRef->hObject = SYS_GAMEOBJECT.Find_Handle_By_UUID(pRef->tUUID);
    }
}

void* IScript::Get_Field_Ptr(const SCRIPT_FIELD_DESC& tDesc)
{
    /* iOffset : offsetof를 통해 알아낸, 객체 시작점에서 해당 멤버 변수까지의 거리(byte) */
    return reinterpret_cast<void*>(reinterpret_cast<_char*>(this) + tDesc.iOffset);
}

const void* IScript::Get_Field_Ptr(const SCRIPT_FIELD_DESC& tDesc) const
{
    return reinterpret_cast<const void*>(reinterpret_cast<const _char*>(this) + tDesc.iOffset);
}
