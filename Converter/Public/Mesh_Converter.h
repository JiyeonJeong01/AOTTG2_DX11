#pragma once
#include "Converter_Define.h"
#include "Converter_Util.h"
#include "BuiltIn_GUID.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>

NS_BEGIN(Converter)

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

static _bool Convert_SingleMesh(const aiMesh* pAIMesh, Engine::CONVERTED_MESH& out)
{
    if (!pAIMesh || pAIMesh->mNumVertices == 0)
        return false;

    out.vertices.clear();
    out.indices.clear();

    const _bool hasNormals = (pAIMesh->mNormals != nullptr);
    const _bool hasTangents = (pAIMesh->mTangents != nullptr);
    const _bool hasUV0 = (pAIMesh->mTextureCoords[0] != nullptr);

    out.vertices.resize(pAIMesh->mNumVertices);

    for (size_t v = 0; v < pAIMesh->mNumVertices; ++v)
    {
        CopyFloat3(out.vertices[v].vPosition, pAIMesh->mVertices[v]);

        if (hasNormals)  CopyFloat3(out.vertices[v].vNormal, pAIMesh->mNormals[v]);
        else             out.vertices[v].vNormal = _float3{ 0.f, 1.f, 0.f };

        if (hasTangents) CopyFloat3(out.vertices[v].vTangent, pAIMesh->mTangents[v]);
        else             out.vertices[v].vTangent = _float3{ 1.f, 0.f, 0.f };

        if (hasUV0)      CopyFloat2(out.vertices[v].vTexcoord, pAIMesh->mTextureCoords[0][v]);
        else             out.vertices[v].vTexcoord = _float2{ 0.f, 0.f };
    }

    out.indices.reserve(pAIMesh->mNumFaces * 3);

    for (uint32_t f = 0; f < pAIMesh->mNumFaces; ++f)
    {
        const aiFace& face = pAIMesh->mFaces[f];
        if (face.mNumIndices != 3)
            continue;

        out.indices.push_back((uint32_t)face.mIndices[0]);
        out.indices.push_back((uint32_t)face.mIndices[1]);
        out.indices.push_back((uint32_t)face.mIndices[2]);
    }

    return !out.vertices.empty() && !out.indices.empty();
}

static _bool Convert_Model(const aiScene* scene, CONVERTED_MODEL& out)
{
    if (!scene || scene->mNumMeshes == 0)
        return false;

    out.parts.clear();

    for (size_t i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* pAIMesh = scene->mMeshes[i];
        if (!pAIMesh)
            continue;

        CONVERTED_MODEL_PART part{};

        if (pAIMesh->mName.length > 0)
            part.strName = pAIMesh->mName.C_Str();
        else
            part.strName = "Part_" + std::to_string(i);

        part.iMaterialIndex = pAIMesh->mMaterialIndex;

        if (!Convert_SingleMesh(pAIMesh, part.mesh))
            continue;

        out.parts.push_back(std::move(part));
    }

    return !out.parts.empty();
}

static const aiScene* LoadScene_Assimp(Assimp::Importer& importer, const std::filesystem::path& fbxPath, uint32_t iFlag = 0)
{
    const uint32_t flags =
        aiProcess_ConvertToLeftHanded |
        aiProcessPreset_TargetRealtime_Fast |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_GenNormals |
        aiProcess_CalcTangentSpace;

    return importer.ReadFile(fbxPath.string(), flags);
}

static std::string Generate_GUID_String()
{
    return Engine::ASSET_GUID::New_GUID().To_String_Utf8();
}

static _bool Save_MeshBin(const std::filesystem::path& outPath, const CONVERTED_MESH& mesh)
{
    std::ofstream ofs(outPath, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    Engine::MESH_HEADER hdr{};
    hdr.vertexCount = (uint32_t)mesh.vertices.size();
    hdr.indexCount = (uint32_t)mesh.indices.size();

    ofs.write((const char*)&hdr, sizeof(hdr));
    ofs.write((const char*)mesh.vertices.data(), sizeof(Engine::VTXMESH) * mesh.vertices.size());
    ofs.write((const char*)mesh.indices.data(), sizeof(uint32_t) * mesh.indices.size());

    return true;
}

static _bool Save_MeshMeta(const std::filesystem::path& metaPath,
    const std::string& strGUID,
    const std::filesystem::path& sourceFbx,
    const std::filesystem::path& cookedMeshbin,
    const CONVERTED_MESH& mesh)
{
    std::ofstream ofs(metaPath);
    if (!ofs.is_open())
        return false;

    ofs << "guid=" << strGUID << "\n";
    ofs << "type=MESH\n";
    ofs << "source=" << sourceFbx.generic_string() << "\n";
    ofs << "cooked=" << cookedMeshbin.generic_string() << "\n";
    ofs << "vertexCount=" << mesh.vertices.size() << "\n";
    ofs << "indexCount=" << mesh.indices.size() << "\n";

    return true;
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

static _bool Save_ModelFile(const std::filesystem::path& modelPath,
    const std::filesystem::path& sourceFbx,
    const std::vector<SAVED_MODEL_PART_INFO>& parts)
{
    std::ofstream ofs(modelPath);
    if (!ofs.is_open())
        return false;

    ofs << "source=" << sourceFbx.generic_string() << "\n";
    ofs << "meshCount=" << parts.size() << "\n";

    for (size_t i = 0; i < parts.size(); ++i)
    {
        ofs << "part" << i << "Name=" << parts[i].strName << "\n";
        ofs << "part" << i << "MeshGuid=" << parts[i].strMeshGUID << "\n";
        ofs << "part" << i << "MaterialGuid=" << parts[i].strMaterialGUID << "\n";
    }

    return true;
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

inline static _bool Convert(
    std::filesystem::path& inPath,          /* Converter/FBXs/... */
    std::filesystem::path& textureRoot,     /* Client/Assets/Textures */
    std::filesystem::path& outMeshPath,     /* Client/Assets/Meshes  */
    std::filesystem::path& outMatPath,      /* Client/Assets/Materials  */
    std::filesystem::path& outMeshMeta)
{
    Assimp::Importer importer;
    const aiScene* pAIScene = LoadScene_Assimp(importer, inPath);

    if (pAIScene == nullptr)
    {
        std::cout << "Assimp load failed : " << inPath.string() << "\n";
        return false;
    }

    CONVERTED_MODEL model{}; /* CONVERTED_MODEL_PART 컨테이너를 가진 구조체 */
    if (!Convert_Model(pAIScene, model))
    {
        std::cout << "Convert model failed : " << inPath.string() << "\n";
        return false;
    }

    std::filesystem::create_directories(outMeshPath.parent_path());
    std::filesystem::create_directories(outMatPath.parent_path());

    std::vector<SAVED_MODEL_PART_INFO> savedParts; /* 최종 model 파일에 기록할 part별로 mesh/material GUID 정보 */
    savedParts.reserve(model.parts.size());

    const std::string strMeshBaseName = outMeshPath.stem().string();
    const std::filesystem::path parentMeshDir = outMeshPath.parent_path();

    const std::filesystem::path parentMatDir = outMatPath.parent_path();

    /* Mesh 정보 생성 전에 Mesh가 참조할 Material 정보를 먼저 생성해둔다. */
    std::unordered_map<uint32_t, std::string> materialIndexToGUID;
    {
        /* 머테리얼 정보 없을 시 채워넣을 기본 텍스쳐, 셰이더 GUID */
        const std::string strDefaultShaderGUID = To_String_Utf8(DEFAULT_ASSET_GUID::SHADER_VTXTEX.value);
        const std::string strDefaultBaseMapGUID = To_String_Utf8(DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT.value);
        const std::string strDefaultNormalMapGUID = To_String_Utf8(DEFAULT_ASSET_GUID::TEXTURE_NORMALMAP_DEFAULT.value);

        for (uint32_t iMat = 0; iMat < pAIScene->mNumMaterials; ++iMat)
        {
            const aiMaterial* pAIMaterial = pAIScene->mMaterials[iMat];

            if (!pAIMaterial)
                continue;

            /* 저장하기 위한 정보 생성 */
            std::string strMaterialGUID = Generate_GUID_String();
            materialIndexToGUID[iMat] = strMaterialGUID;

            std::string strMaterialName = strMeshBaseName + "_Mat_" + std::to_string(iMat);

            const std::filesystem::path matPath = parentMatDir / (strMaterialName + ".mat");            /* e.g., Character_Mat_0.mat */
            const std::filesystem::path matMetaPath = parentMatDir / (strMaterialName + ".mat.meta");   /* e.g., Character_Mat_0.mat.meta*/

            const _float4 baseColor = Read_BaseColor(pAIMaterial);
            const _float fShininess = Read_Shininess(pAIMaterial);

            std::string strBaseMapGUID = strDefaultBaseMapGUID;
            std::string strNormalMapGUID = strDefaultNormalMapGUID;

            /* 텍스쳐 경로 구하기 */
            aiString strTexPath{};
            if (AI_SUCCESS == pAIMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &strTexPath))
            {
                strBaseMapGUID = Resolve_Texture_GUID_From_AssimpPath(
                    textureRoot, strTexPath, strDefaultBaseMapGUID);
            }

            if (AI_SUCCESS == pAIMaterial->GetTexture(aiTextureType_NORMALS, 0, &strTexPath))
            {
                strNormalMapGUID = Resolve_Texture_GUID_From_AssimpPath(
                    textureRoot, strTexPath, strDefaultNormalMapGUID);
            }

            if (!Save_MaterialFile(
                matPath,
                strMaterialGUID,
                strDefaultShaderGUID,
                baseColor,
                fShininess,
                strBaseMapGUID,
                strNormalMapGUID))
            {
                std::cout << "Save material file failed : " << matPath.string() << "\n";
                return false;
            }

            if (!Save_MaterialMeta(matMetaPath, strMaterialGUID, inPath, matPath))
            {
                std::cout << "Save material meta failed : " << matMetaPath.string() << "\n";
                return false;
            }
        }
    }

    for (size_t i = 0; i < model.parts.size(); ++i)
    {
        const auto& part = model.parts[i];

        /* 모델+메쉬 저장할 경로 정보 생성 */
        const std::string strMeshGUID = Generate_GUID_String();
        const std::string strPartFileBase = strMeshBaseName + "_" + std::to_string(i);

        std::filesystem::path meshBinPath = parentMeshDir / (strPartFileBase + ".mesh");
        std::filesystem::path meshMetaPath = parentMeshDir / (strPartFileBase + ".mesh.meta");

        if (!Save_MeshBin(meshBinPath, part.mesh))
        {
            std::cout << "Save mesh bin failed : " << meshBinPath.string() << "\n";
            return false;
        }

        if (!Save_MeshMeta(meshMetaPath, strMeshGUID, inPath, meshBinPath, part.mesh))
        {
            std::cout << "Save mesh meta failed : " << meshMetaPath.string() << "\n";
            return false;
        }

        SAVED_MODEL_PART_INFO info{};
        info.strName = part.strName;
        info.strMeshGUID = strMeshGUID;

        /* 미리 생성해둔 Material 정보와 Mesh 매핑하기 */
        auto itFound = materialIndexToGUID.find(part.iMaterialIndex);
        if (itFound != materialIndexToGUID.end())
            info.strMaterialGUID = itFound->second;
        else
            info.strMaterialGUID.clear();

        savedParts.push_back(std::move(info));
    }

    /* 최종적으로 모델 정보 저장하기 */
    const std::string strModelGUID = Generate_GUID_String();
    if (!Save_ModelFile(outMeshPath, inPath, savedParts))
    {
        std::cout << "Save model file failed : " << outMeshPath.string() << "\n";
        return false;
    }

    if (!Save_ModelMeta(outMeshMeta, strModelGUID))
    {
        std::cout << "Save model meta failed : " << outMeshMeta.string() << "\n";
        return false;
    }

    return true;
}




NS_END
