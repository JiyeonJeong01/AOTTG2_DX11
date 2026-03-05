#include "MaterialBuilder.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

HRESULT CMaterialBuilder::Create(const SHADER_ENTRY& shader, uint16_t passIndex, MATERIAL_ENTRY& outMaterial)
{
    if (!shader.Is_Valid())
        return E_FAIL;

    if (passIndex >= shader.pPasses.size())
        return E_FAIL;

    outMaterial = MATERIAL_ENTRY{};
    outMaterial.tGUID = ASSET_GUID::New_GUID();
    outMaterial.shaderGUID = shader.tGUID;
    outMaterial.passIndex = passIndex;

    outMaterial.baseColor = _float4{ 1.f, 1.f, 1.f, 1.f };
    outMaterial.baseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;

    return S_OK;
}

static void WriteGuid(json& j, const char* key, const ASSET_GUID& g)
{
    j[key] = g.To_String_Utf8();
}

static bool ReadGuid(const json& j, const char* key, ASSET_GUID& out)
{
    if (!j.contains(key))
        return false;
    const auto& v = j.at(key);
    if (!v.is_string())
        return false;
    ASSET_GUID::Try_Utf8_To_GUID(v.get<std::string>(), out);
    return true;
}

HRESULT CMaterialBuilder::Save_Material(const MATERIAL_ENTRY& mat, const std::filesystem::path& filePath)
{
    json j;

    WriteGuid(j, "GUID", mat.tGUID);
    WriteGuid(j, "ShaderGUID", mat.shaderGUID);
    j["PassIndex"] = mat.passIndex;
    j["BaseColor"] = { mat.baseColor.x, mat.baseColor.y, mat.baseColor.z, mat.baseColor.w };
    WriteGuid(j, "BaseMapGUID", mat.baseMapGUID);
    std::ofstream ofs(filePath, std::ios_base::trunc);
    if (!ofs.is_open())
        return E_FAIL;

    ofs << j.dump(4);
    return S_OK;
}

HRESULT CMaterialBuilder::Load_MaterialDesc(const std::filesystem::path& filePath, MATERIAL_ENTRY& outDesc)
{
    std::ifstream ifs(filePath);
    if (!ifs.is_open())
        return E_FAIL;

    json j;
    try { ifs >> j; }
    catch (...) { return E_FAIL; }

    outDesc = MATERIAL_ENTRY{};

    if (!ReadGuid(j, "GUID", outDesc.tGUID))
        return E_FAIL;
    if (!ReadGuid(j, "ShaderGUID", outDesc.shaderGUID))
        return E_FAIL;
    if (!j.contains("PassIndex") || !j.at("PassIndex").is_number_unsigned())
        return E_FAIL;
    outDesc.passIndex = (uint16_t)j.at("PassIndex").get<uint32_t>();

    if (j.contains("BaseColor"))
    {
        const auto& a = j.at("BaseColor");
        if (!a.is_array() || a.size() != 4) return E_FAIL;
        if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number() || !a[3].is_number()) return E_FAIL;

        outDesc.baseColor = _float4{
            a[0].get<_float>(),
            a[1].get<_float>(),
            a[2].get<_float>(),
            a[3].get<_float>()
        };
    }
    else
    {
        outDesc.baseColor = _float4{ 1.f,1.f,1.f,1.f };
    }

    if (j.contains("BaseMapGUID"))
    {
        if (!ReadGuid(j, "BaseMapGUID", outDesc.baseMapGUID))
            return E_FAIL;
    }
    else
    {
        outDesc.baseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;
    }

    outDesc.hShader = 0;
    outDesc.hBaseMap = INVALID_HANDLE_UINT;

    outDesc.pWorld = nullptr;
    outDesc.pView = nullptr;
    outDesc.pProj = nullptr;

    outDesc.pMainTex = nullptr;
    outDesc.pColor = nullptr;
    outDesc.pUV = nullptr;
    outDesc.pClip = nullptr;

    outDesc.materialParams.params.clear();

    return S_OK;
}

NS_END
