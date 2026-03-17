#pragma once
#include "Converter_Util.h"

NS_BEGIN(Converter)

static _bool Save_MaterialFile(const std::filesystem::path& matPath,
    const std::string& strMaterialGUID,
    const std::string& strShaderGUID,
    const _float4& baseColor,
    _float fShininess,
    const std::string& strBaseMapGUID,
    const std::string& strNormalMapGUID)
{
    std::ofstream ofs(matPath);
    if (!ofs.is_open())
        return false;

    ofs << "{\n";
    ofs << "  \"GUID\": \"" << strMaterialGUID << "\",\n";
    ofs << "  \"ShaderGUID\": \"" << strShaderGUID << "\",\n";
    ofs << "  \"PassIndex\": 0,\n";
    ofs << "  \"BaseColor\": [" << baseColor.x << ", " << baseColor.y << ", " << baseColor.z << ", " << baseColor.w << "],\n";
    ofs << "  \"Shininess\": " << fShininess << ",\n";
    ofs << "  \"BaseMapGUID\": \"" << strBaseMapGUID << "\",\n";
    ofs << "  \"NormalMapGUID\": \"" << strNormalMapGUID << "\"\n";
    ofs << "}\n";

    return true;
}

static _bool Save_MaterialMeta(const std::filesystem::path& metaPath,
    const std::string& strMaterialGUID,
    const std::filesystem::path& sourceFbx,
    const std::filesystem::path& cookedMaterialPath)
{
    std::ofstream ofs(metaPath);
    if (!ofs.is_open())
        return false;

    ofs << "guid=" << strMaterialGUID << "\n";
    ofs << "type=MATERIAL\n";
    ofs << "source=" << sourceFbx.generic_string() << "\n";
    ofs << "cooked=" << cookedMaterialPath.generic_string() << "\n";

    return true;
}

NS_END
