#include "MaterialBuilder.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

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
    j["RenderType"] = SCAST(uint32_t, mat.eRenderType);
    j["BaseColor"] = { mat.baseColor.x, mat.baseColor.y, mat.baseColor.z, mat.baseColor.w };
    j["Shininess"] = mat.fShininess;
    WriteGuid(j, "BaseMapGUID", mat.baseMapGUID);
    WriteGuid(j, "NormalMapGUID", mat.normalMapGUID);

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
    try
    {
        ifs >> j;
    }
    catch (...)
    {
        return E_FAIL;
    }

    outDesc = MATERIAL_ENTRY{};

    /* 필수값 */
    if (!ReadGuid(j, "GUID", outDesc.tGUID))
        return E_FAIL;

    if (!ReadGuid(j, "ShaderGUID", outDesc.shaderGUID))
        return E_FAIL;

    if (!j.contains("PassIndex") || !j.at("PassIndex").is_number_unsigned())
        return E_FAIL;
    outDesc.passIndex = static_cast<uint16_t>(j.at("PassIndex").get<uint32_t>());

    /* 선택값 : RenderType */
    uint32_t renderTypeValue = To<uint32_t>(MATERIAL_RENDER_TYPE::DEFAULT);
    if (j.contains("RenderType"))
    {
        if (!j.at("RenderType").is_number_unsigned())
            return E_FAIL;

        renderTypeValue = j.at("RenderType").get<uint32_t>();
    }
    outDesc.eRenderType = SCAST(MATERIAL_RENDER_TYPE, renderTypeValue);

    /* 선택값 : BaseColor */
    if (j.contains("BaseColor"))
    {
        const auto& a = j.at("BaseColor");
        if (!a.is_array() || a.size() != 4)
            return E_FAIL;

        if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number() || !a[3].is_number())
            return E_FAIL;

        outDesc.baseColor = _float4 { a[0].get<_float>(),a[1].get<_float>(), a[2].get<_float>(),a[3].get<_float>()};
    }
    else
    {
        outDesc.baseColor = _float4{ 1.f, 1.f, 1.f, 1.f };
    }

    /* 선택값 : Shininess */
    if (j.contains("Shininess"))
    {
        if (!j.at("Shininess").is_number())
            return E_FAIL;

        outDesc.fShininess = j.at("Shininess").get<_float>();
    }
    else
    {
        outDesc.fShininess = 32.f;
    }

    /* 선택값 : BaseMapGUID */
    if (j.contains("BaseMapGUID"))
    {
        if (!ReadGuid(j, "BaseMapGUID", outDesc.baseMapGUID))
            return E_FAIL;
    }
    else
    {
        outDesc.baseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;
    }

    /* 선택값 : NormalMapGUID */
    if (j.contains("NormalMapGUID"))
        if (!ReadGuid(j, "NormalMapGUID", outDesc.normalMapGUID))
            return E_FAIL;
    else
        outDesc.normalMapGUID = DEFAULT_ASSET_GUID::TEXTURE_NORMALMAP_DEFAULT;

    /* 런타임 바인딩 값 초기화 */
    outDesc.hShader = 0;
    outDesc.hBaseMap = INVALID_HANDLE_UINT;
    outDesc.hNormalMap = INVALID_HANDLE_UINT;

    outDesc.pWorld = nullptr;
    outDesc.pView = nullptr;
    outDesc.pProj = nullptr;

    outDesc.pBaseMap = nullptr;
    outDesc.pNormalMap = nullptr;

    outDesc.pBaseColor = nullptr;
    outDesc.pShininess = nullptr;

    outDesc.materialParams.Clear();

    return S_OK;
}

HRESULT CMaterialBuilder::Load_Default_UI(MATERIAL_ENTRY& outDesc)
{
    outDesc = MATERIAL_ENTRY{};

    outDesc.tGUID = DEFAULT_ASSET_GUID::MATERIAL_UI_DEFAULT;
    outDesc.shaderGUID = DEFAULT_ASSET_GUID::SHADER_VTXTEX;
    outDesc.passIndex = 0;
    outDesc.baseColor = {1.f, 1.f, 1.f, 1.f};
    outDesc.baseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_UI_DEFAULT;

    outDesc.hShader = 0;
    outDesc.hBaseMap = INVALID_HANDLE_UINT;

    outDesc.pWorld = nullptr;
    outDesc.pView = nullptr;
    outDesc.pProj = nullptr;

    outDesc.pBaseMap = nullptr;
    outDesc.pBaseColor = nullptr;

    outDesc.materialParams.params.clear();

    return S_OK;

}

HRESULT CMaterialBuilder::Load_Default_VTXTEX(MATERIAL_ENTRY& outDesc)
{
    outDesc = MATERIAL_ENTRY{};

    outDesc.tGUID = DEFAULT_ASSET_GUID::MATERIAL_VTXTEX;
    outDesc.shaderGUID = DEFAULT_ASSET_GUID::SHADER_VTXTEX;
    outDesc.passIndex = 0;
    outDesc.baseColor = { 1.f, 1.f, 1.f, 1.f };
    outDesc.baseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;

    outDesc.hShader = 0;
    outDesc.hBaseMap = INVALID_HANDLE_UINT;

    outDesc.pWorld = nullptr;
    outDesc.pView = nullptr;
    outDesc.pProj = nullptr;

    outDesc.pBaseMap = nullptr;
    outDesc.pBaseColor = nullptr;

    outDesc.materialParams.params.clear();

    return S_OK;
}

HRESULT CMaterialBuilder::Load_ParticleDesc(const std::filesystem::path& filePath, PARTICLE_ENTRY& outDesc)
{
    std::ifstream ifs(filePath);
    if (!ifs.is_open())
        return E_FAIL;

    json j;
    try
    {
        ifs >> j;
    }
    catch (...)
    {
        return E_FAIL;
    }

    outDesc = PARTICLE_ENTRY{};

    /* 필수값 : GUID */
    if (!ReadGuid(j, "GUID", outDesc.tGUID))
        return E_FAIL;

    /* 필수값 : TextureGUID */
    if (!ReadGuid(j, "TextureGUID", outDesc.tTextureGUID))
        return E_FAIL;

    /* 필수값 : MaxParticles */
    if (!j.contains("MaxParticles") || !j.at("MaxParticles").is_number_unsigned())
        return E_FAIL;
    outDesc.iMaxParticles = j.at("MaxParticles").get<_uint>();

    /* 필수값 : LifeTime [min, max] */
    if (!j.contains("LifeTime") || !j.at("LifeTime").is_array() || j.at("LifeTime").size() != 2)
        return E_FAIL;
    {
        const auto& a = j.at("LifeTime");
        if (!a[0].is_number() || !a[1].is_number())
            return E_FAIL;

        outDesc.vLifeTime = _float2(a[0].get<_float>(), a[1].get<_float>());
    }

    /* 필수값 : Speed [min, max] */
    if (!j.contains("Speed") || !j.at("Speed").is_array() || j.at("Speed").size() != 2)
        return E_FAIL;
    {
        const auto& a = j.at("Speed");
        if (!a[0].is_number() || !a[1].is_number())
            return E_FAIL;

        outDesc.vSpeed = _float2(a[0].get<_float>(), a[1].get<_float>());
    }

    /* 필수값 : Scale [min, max] */
    if (!j.contains("Scale") || !j.at("Scale").is_array() || j.at("Scale").size() != 2)
        return E_FAIL;
    {
        const auto& a = j.at("Scale");
        if (!a[0].is_number() || !a[1].is_number())
            return E_FAIL;

        outDesc.vScale = _float2(a[0].get<_float>(), a[1].get<_float>());
    }

    /* 선택값 : Center */
    if (j.contains("Center"))
    {
        const auto& a = j.at("Center");
        if (!a.is_array() || a.size() != 3)
            return E_FAIL;
        if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number())
            return E_FAIL;

        outDesc.vCenter = _float3(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>());
    }
    else
    {
        outDesc.vCenter = _float3(0.f, 0.f, 0.f);
    }

    /* 선택값 : Range */
    if (j.contains("Range"))
    {
        const auto& a = j.at("Range");
        if (!a.is_array() || a.size() != 3)
            return E_FAIL;
        if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number())
            return E_FAIL;

        outDesc.vRange = _float3(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>());
    }
    else
    {
        outDesc.vRange = _float3(0.f, 0.f, 0.f);
    }

    /* 선택값 : Pivot */
    if (j.contains("Pivot"))
    {
        const auto& a = j.at("Pivot");
        if (!a.is_array() || a.size() != 3)
            return E_FAIL;
        if (!a[0].is_number() || !a[1].is_number() || !a[2].is_number())
            return E_FAIL;

        outDesc.vPivot = _float3(a[0].get<_float>(), a[1].get<_float>(), a[2].get<_float>());
    }
    else
    {
        outDesc.vPivot = _float3(0.f, 0.f, 0.f);
    }

    /* 선택값 : IsLoop */
    if (j.contains("IsLoop"))
    {
        if (!j.at("IsLoop").is_boolean())
            return E_FAIL;

        outDesc.isLoop = j.at("IsLoop").get<_bool>();
    }
    else
    {
        outDesc.isLoop = true;
    }

    /* 선택값 : Simulation */
    if (j.contains("Simulation"))
    {
        if (!j.at("Simulation").is_number_unsigned())
            return E_FAIL;

        const auto eSim = static_cast<PARTICLE_SIMULATION>(j.at("Simulation").get<uint32_t>());
        outDesc.eSimulation = eSim;
    }
    else
    {
        outDesc.eSimulation = PARTICLE_SIMULATION::DROP;
    }

    /* 런타임 바인딩 값 초기화 */
    outDesc.hTexture = INVALID_HANDLE_UINT;

    return S_OK;
}


NS_END
