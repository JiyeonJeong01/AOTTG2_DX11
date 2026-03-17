#pragma once
#include "Converter_Define.h"
#include "Converter_Util.h"
#include "BuiltIn_GUID.h"
#include "Material_Converter.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>

NS_BEGIN(Converter)



static _bool Convert_SingleNonAnimMesh(const aiMesh* pAIMesh, Engine::CONVERTED_MESH& out, const float fImportScale)
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
        CopyFloat3_Scale(out.vertices[v].vPosition, pAIMesh->mVertices[v], fImportScale);

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

static _bool Convert_NonAnimModel(const aiScene* scene, CONVERTED_MODEL& out, const float fImportScale)
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

        if (!Convert_SingleNonAnimMesh(pAIMesh, part.mesh, fImportScale))
            continue;

        out.parts.push_back(std::move(part));
    }

    return !out.parts.empty();
}

static const aiScene* LoadScene_Assimp_NonAnim(Assimp::Importer& importer, const std::filesystem::path& fbxPath, uint32_t iFlag = 0)
{
    const uint32_t flags =
        aiProcess_ConvertToLeftHanded |             /* 왼손 좌표계 기준으로 변경 */
        aiProcessPreset_TargetRealtime_Fast |       /* 빠른 실시간 렌더링 용도 프리셋 */
        aiProcess_Triangulate |                     /* 모든 폴리곤 Triangle */
        aiProcess_JoinIdenticalVertices |           /* 중복 정점 줄여 최적화 */
        aiProcess_GenNormals |                      /* 노멀이 없다면 자동 노멀 생성 */
        aiProcess_CalcTangentSpace;                 /* 탄젠트/비탄젠트 계산  */

    return importer.ReadFile(fbxPath.string(), flags | iFlag);
}


static _bool Save_NonAnim_MeshBin(const std::filesystem::path& outPath, const CONVERTED_MESH& mesh)
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

static _bool Save_NonAnim_MeshMeta(const std::filesystem::path& metaPath,
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

static _bool Save_NonAnim_ModelFile(const std::filesystem::path& modelPath,
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

inline static _bool Convert_NonAnim(
    std::filesystem::path& inPath,          /* Converter/FBXs/... */
    std::filesystem::path& textureRoot,     /* Client/Assets/Textures */
    std::filesystem::path& outMeshPath,     /* Client/Assets/Meshes  */
    std::filesystem::path& outMatPath,      /* Client/Assets/Materials  */
    std::filesystem::path& outMeshMeta,
    const _float fImportScale)
{
    Assimp::Importer importer;
    const aiScene* pAIScene = LoadScene_Assimp_NonAnim(importer, inPath);

    if (pAIScene == nullptr)
    {
        std::cout << "Assimp load failed : " << inPath.string() << "\n";
        return false;
    }

    CONVERTED_MODEL model{}; /* CONVERTED_MODEL_PART 컨테이너를 가진 구조체 */
    if (!Convert_NonAnimModel(pAIScene, model, fImportScale))
    {
        std::cout << "Convert_NonAnim model failed : " << inPath.string() << "\n";
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
        const std::string strDefaultShaderGUID = To_String_Utf8(DEFAULT_ASSET_GUID::SHADER_VTXMESH.value);
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

        if (!Save_NonAnim_MeshBin(meshBinPath, part.mesh))
        {
            std::cout << "Save mesh bin failed : " << meshBinPath.string() << "\n";
            return false;
        }

        if (!Save_NonAnim_MeshMeta(meshMetaPath, strMeshGUID, inPath, meshBinPath, part.mesh))
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
    if (!Save_NonAnim_ModelFile(outMeshPath, inPath, savedParts))
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
