#pragma once

#include "Converter_Define.h"

NS_BEGIN(Converter)

static inline void CopyFloat3(_float3& dst, const aiVector3D& src)
{
    dst.x = src.x; dst.y = src.y; dst.z = src.z;
}

static inline void CopyFloat3_Scale(_float3& dst, const aiVector3D& src, const float fImportScale)
{
    dst.x = src.x * fImportScale; dst.y = src.y * fImportScale; dst.z = src.z * fImportScale;
}

static inline void CopyFloat2(_float2& dst, const aiVector3D& src)
{
    dst.x = src.x; dst.y = src.y;
}

static std::string To_String_Utf8(const GUID& value)
{
    wchar_t szBuff[64]{};

    /* Convert GUID to wide string(standard Windows format : {XXXXXXXX - XXXX - ...}) */
    _int n = ::StringFromGUID2(value, szBuff, 64);
    if (n <= 0)
        return {};
    /* Calculate required buffer size for UTF-8 conversion */
    _int iLen = ::WideCharToMultiByte(CP_UTF8, 0, szBuff, -1, nullptr, 0, nullptr, nullptr);
    std::string strOut;
    strOut.resize((iLen > 0) ? (iLen - 1) : 0);

    /* Convert Wide string to Multi-byte string (UTF-8) */
    if (!strOut.empty())
        ::WideCharToMultiByte(CP_UTF8, 0, szBuff, -1, strOut.data(), iLen, nullptr, nullptr);

    if (!strOut.empty() && strOut.front() == '{' && strOut.back() == '}')
        strOut = strOut.substr(1, strOut.size() - 2);

    return strOut;
}

static std::string Generate_GUID_String()
{
    return Engine::ASSET_GUID::New_GUID().To_String_Utf8();
}

static std::unordered_set<std::wstring> Build_ExistingModelStemSet(const std::filesystem::path& meshRoot)
{
    std::unordered_set<std::wstring> set;

    std::error_code ec;
    if (!std::filesystem::exists(meshRoot, ec))
        return set;

    for (auto& it : std::filesystem::directory_iterator(meshRoot, ec))
    {
        if (ec) break;
        if (!it.is_regular_file(ec)) continue;

        const auto& p = it.path();

        if (p.extension() == L".meta")
        {
            const std::wstring filename = p.filename().wstring();

            const std::wstring suffix = L".model.meta";
            if (filename.size() >= suffix.size())
            {
                if (filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0)
                {
                    const std::wstring stem = filename.substr(0, filename.size() - suffix.size());
                    set.insert(stem);
                }
            }
        }
    }

    return set;
}

static void Write_Float4x4_Bin(std::ofstream& ofs, const _float4x4& mat)
{
    const _float values[16] =
    {
        mat._11, mat._12, mat._13, mat._14,
        mat._21, mat._22, mat._23, mat._24,
        mat._31, mat._32, mat._33, mat._34,
        mat._41, mat._42, mat._43, mat._44
    };

    ofs.write(reinterpret_cast<const char*>(values), sizeof(values));
}

static _float4 Read_BaseColor(const aiMaterial* pAIMaterial)
{
    if (!pAIMaterial)
        return _float4{ 1.f, 1.f, 1.f, 1.f };

    aiColor4D vColor{};
    if (AI_SUCCESS == aiGetMaterialColor(pAIMaterial, AI_MATKEY_COLOR_DIFFUSE, &vColor))
    {
        return _float4{ vColor.r, vColor.g, vColor.b, vColor.a };
    }

    return _float4{ 1.f, 1.f, 1.f, 1.f };
}

static _float Read_Shininess(const aiMaterial* pAIMaterial)
{
    if (!pAIMaterial)
        return 32.f;

    _float fShininess = 32.f;
    if (AI_SUCCESS == aiGetMaterialFloat(pAIMaterial, AI_MATKEY_SHININESS, &fShininess))
        return fShininess;

    return 32.f;
}

static std::filesystem::path Make_MetaPath(const std::filesystem::path& assetPath)
{
    std::filesystem::path metaPath = assetPath;
    metaPath += L".meta";
    return metaPath;
}

static _bool Read_GUID_From_MetaFile(const std::filesystem::path& metaPath, std::string& outGUID)
{
    std::ifstream ifs(metaPath);
    if (!ifs.is_open())
        return false;

    std::string line;
    const std::string prefix = "guid=";

    while (std::getline(ifs, line))
    {
        if (line.rfind(prefix, 0) == 0)
        {
            outGUID = line.substr(prefix.size());
            return !outGUID.empty();
        }
    }

    return false;
}

static std::string Extract_TextureFileName(const aiString& strTexturePath)
{
    const std::filesystem::path texPath = strTexturePath.C_Str();
    return texPath.filename().string();
}

/**
 * \brief  Assets/Textures 폴더에 위치한 텍스쳐의 GUID를 반환하여, 머테리얼이 참조할 수 있도록 한다.
 */
static std::string Resolve_Texture_GUID_From_AssimpPath(
    const std::filesystem::path& textureRoot,
    const aiString& strTexturePath,
    const std::string& strDefaultGUID)
{
    const std::string strFileName = Extract_TextureFileName(strTexturePath);
    if (strFileName.empty())
        return strDefaultGUID;

    std::error_code ec;
    if (!std::filesystem::exists(textureRoot, ec))
        return strDefaultGUID;

    for (auto it = std::filesystem::recursive_directory_iterator(textureRoot, ec);
        it != std::filesystem::recursive_directory_iterator();
        ++it)
    {
        if (ec)
            break;

        if (!it->is_regular_file(ec))
            continue;

        const std::filesystem::path filePath = it->path();

        if (filePath.extension() == L".meta")
            continue;

        if (filePath.filename().string() != strFileName)
            continue;

        const std::filesystem::path metaPath = Make_MetaPath(filePath);

        std::string strGUID;
        if (Read_GUID_From_MetaFile(metaPath, strGUID))
            return strGUID;
    }

    std::cout << "Texture GUID resolve failed: " << strFileName
        << " in root: " << textureRoot.string() << "\n";

    return strDefaultGUID;
}


static _bool Save_ModelMeta(const std::filesystem::path& metaPath,
    const std::string& strModelGUID)
{
    std::ofstream ofs(metaPath);
    if (!ofs.is_open())
        return false;

    ofs << "guid=" << strModelGUID << "\n";
    ofs << "type=MODEL\n";

    return true;
}

NS_END
