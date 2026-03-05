#pragma once
#include "Converter_Define.h"
#include "Converter_Util.h"

#include <filesystem>

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

            /* Fiona.model.meta -> Fiona 추출 */
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

        CONVERTED_MODEL_PART  part{};

        if (pAIMesh->mName.length > 0)
            part.strName = pAIMesh->mName.C_Str();
        else
            part.strName = "Part_" + std::to_string(i);

        if (!Convert_SingleMesh(pAIMesh, part.mesh))
            continue;

        out.parts.push_back(std::move(part));
    }

    return !out.parts.empty();
}

static const aiScene* LoadScene_Assimp(Assimp::Importer& importer, const std::filesystem::path& fbxPath, uint32_t iFlag = 0)
{
    const uint32_t flags =
        aiProcess_ConvertToLeftHanded       |   /* 왼손 좌표계 변경 */
        aiProcessPreset_TargetRealtime_Fast |   /* 실시간 렌더링 최적화를 위한 플래그 */
        aiProcess_Triangulate               |   /* 폴리곤 삼각형 */
        aiProcess_JoinIdenticalVertices     |   /* 중복 제거 */
        aiProcess_GenNormals;                   /* NORMAL 없으면 생성하기 */

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

static _bool Save_MetaJson(const std::filesystem::path& metaPath,
    const std::string& strGUID,
    const std::filesystem::path& sourceFbx,
    const std::filesystem::path& cookedMeshbin,
    const Engine::CONVERTED_MESH& mesh)
{
    std::ofstream ofs(metaPath);
    if (!ofs.is_open())
        return false;

    /* 최소 정보 :  GUID, source, cooked, type, counts */
    ofs << "{\n";
    ofs << "  \"GUID\": \"" << strGUID << "\",\n";
    ofs << "  \"Type\": \"Mesh\",\n";
    ofs << "  \"Source\": \"" << sourceFbx.generic_string() << "\",\n";
    ofs << "  \"Cooked\": \"" << cookedMeshbin.generic_string() << "\",\n";
    ofs << "  \"VertexCount\": " << mesh.vertices.size() << ",\n";
    ofs << "  \"IndexCount\": " << mesh.indices.size() << "\n";
    ofs << "}\n";
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

inline static _bool Convert(std::filesystem::path& inPath, std::filesystem::path& outPath, std::filesystem::path& outMeta)
{
    Assimp::Importer importer;
    const aiScene* pAIScene = LoadScene_Assimp(importer, inPath);
    if (pAIScene == nullptr)
    {
        std::cout << "Assimp load failed\n";
        return false;
    }

    CONVERTED_MODEL model{};
    if (!Convert_Model(pAIScene, model))
    {
        std::cout << "Convert model failed\n";
        return false;
    }

    std::filesystem::create_directories(outPath.parent_path());

    std::vector<SAVED_MODEL_PART_INFO> savedParts;
    savedParts.reserve(model.parts.size());

    const std::string strBaseName = outPath.stem().string();
    const std::filesystem::path parentDir = outPath.parent_path();

    for (size_t i = 0; i < model.parts.size(); ++i)
    {
        const auto& part = model.parts[i];

        const std::string strMeshGUID = Generate_GUID_String();

        std::string strPartFileBase = strBaseName + "_" + std::to_string(i);

        std::filesystem::path meshBinPath = parentDir / (strPartFileBase + ".mesh");
        std::filesystem::path meshMetaPath = parentDir / (strPartFileBase + ".mesh.meta");

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
        savedParts.push_back(std::move(info));
    }

    const std::string strModelGUID = Generate_GUID_String();

    /* payload는 .model */
    if (!Save_ModelFile(outPath, inPath, savedParts))
    {
        std::cout << "Save model file failed : " << outPath.string() << "\n";
        return false;
    }

    /* sidecar meta는 .model.meta */
    if (!Save_ModelMeta(outMeta, strModelGUID))
    {
        std::cout << "Save model meta failed : " << outMeta.string() << "\n";
        return false;
    }

    return true;
}

NS_END
