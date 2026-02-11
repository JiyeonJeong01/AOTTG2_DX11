#pragma once
#include "Engine_Define.h"
#include "Spec_Struct.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagTestASpec : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TEST_A)

    _float3 vData[4]{};

    [[nodiscard]]
    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagTestASpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        for (int i = 0; i < 4; ++i)
            j["vData"].push_back({ vData[i].x, vData[i].y, vData[i].z });
    };

    _bool FromJson(const json& j) override
    {
        if (!j.contains("vData") || !j["vData"].is_array())
            return false;
        for (int i = 0; i < 4 && i < j["vData"].size(); ++i) {
            vData[i].x = j["vData"][i][0];
            vData[i].y = j["vData"][i][1];
            vData[i].z = j["vData"][i][2];
        }
        return true;
    };


} TEST_A_SPEC;

typedef struct ENGINE_DLL tagTestBSpec : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TEST_B)

    _float3 vData[4]{};

    [[nodiscard]]
    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagTestBSpec>(*this);
    }

    void ToJson(json& j) const override
    {
        j["Type"] = SCAST(_uint, Get_Type());
        for (int i = 0; i < 4; ++i)
            j["vData"].push_back({ vData[i].x, vData[i].y, vData[i].z });
    };

    _bool FromJson(const json& j) override
    {
        if (!j.contains("vData") || !j["vData"].is_array())
            return false;
        for (int i = 0; i < 4 && i < j["vData"].size(); ++i)
        {
            vData[i].x = j["vData"][i][0];
            vData[i].y = j["vData"][i][1];
            vData[i].z = j["vData"][i][2];
        }
        return true;
    };

} TEST_B_SPEC;

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

typedef struct ENGINE_DLL tagTextureSpec final : public COMPONENT_SPEC_BASE
{
    COMPONENT_SPEC_TYPE(COMPONENT_TYPE::TEXTURE)

    [[nodiscard]]
    std::unique_ptr<COMPONENT_SPEC_BASE> Clone() const override
    {
        return std::make_unique<tagTextureSpec>(*this);
    }

    void ToJson(json& j) const override {
        j["Type"] = SCAST(_uint, Get_Type());
        if (pFilePathPattern) {
            std::wstring ws(pFilePathPattern);
            std::string s;
            for (auto c : ws) s += static_cast<char>(c);
            j["FilePath"] = s;
        }
        j["NumSRVs"] = iNumSRVs;
    }

    _bool FromJson(const json& j) override
    {
        if (j.contains("FilePath"))
        {
            std::string s = j["FilePath"];
            // pFilePathPattern = AllocateString(s); 
        }
        iNumSRVs = j.value("NumSRVs", 1);
        return true;
    };

    const _tchar* pFilePathPattern = nullptr;
    _uint iNumSRVs = 1;
} TEXTURE_SPEC;

NS_END
