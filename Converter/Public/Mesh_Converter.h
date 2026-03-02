#pragma once
#include "Converter_Define.h"
#include "Converter_Util.h"

#include <filesystem>

NS_BEGIN(Converter)

static std::unordered_set<std::wstring> Build_ExistingMeshStemSet(const std::filesystem::path& meshRoot)
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
        if (p.extension() == L".mesh")
        {
            set.insert(p.stem().wstring());
        }
    }
    return set;
}

/* TODO : 확장 필요!!  일단은 첫 번째 메쉬만 뽑는다. */
static _bool Convert_Mesh(const aiScene* scene, Engine::CONVERTED_MESH& out)
{
    if (!scene || scene->mNumMeshes == 0)
        return false;

    const aiMesh* pAIMesh = scene->mMeshes[0];
    if (!pAIMesh || pAIMesh->mNumVertices == 0)
        return false;

    out.vertices.resize(pAIMesh->mNumVertices);

    const bool hasNormals = (pAIMesh->mNormals != nullptr);
    const bool hasTangents = (pAIMesh->mTangents != nullptr);
    const bool hasUV0 = (pAIMesh->mTextureCoords[0] != nullptr);

    /* 버텍스 추출 */
    for (size_t i = 0; i < pAIMesh->mNumVertices; i++)
    {
        CopyFloat3(out.vertices[i].vPosition, pAIMesh->mVertices[i]);

        if (hasNormals)  CopyFloat3(out.vertices[i].vNormal, pAIMesh->mNormals[i]);
        else             out.vertices[i].vNormal = _float3{ 0.f, 1.f, 0.f };

        if (hasTangents) CopyFloat3(out.vertices[i].vTangent, pAIMesh->mTangents[i]);
        else             out.vertices[i].vTangent = _float3{ 1.f, 0.f, 0.f };

        if (hasUV0)      CopyFloat2(out.vertices[i].vTexcoord, pAIMesh->mTextureCoords[0][i]);
        else             out.vertices[i].vTexcoord = _float2{ 0.f, 0.f };
    }

    /* 인덱스 추출 */
    out.indices.clear();
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

static _bool Save_MeshBin(const std::filesystem::path& outPath, const Engine::CONVERTED_MESH& mesh)
{
    std::ofstream ofs(outPath, std::ios_base::binary);
    if (!ofs.is_open())
        return false;

    Engine::MESH_HEADER hdr{};
    hdr.vertexCount = (uint32_t)mesh.vertices.size();
    hdr.indexCount = (uint32_t)mesh.indices.size();

    ofs.write((const char*)&hdr, sizeof(hdr));
    ofs.write((const char*)mesh.vertices.data(), sizeof(VTXMESH) * mesh.vertices.size());
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

inline static _bool Convert(std::filesystem::path& inPath, std::filesystem::path& outPath, std::filesystem::path& outMeta)
{
    Assimp::Importer importer;
    const aiScene* pAIScene = LoadScene_Assimp(importer, inPath);
    if (pAIScene == nullptr)
    {
        std::cout << "Assimp load failed" << "\n";
        return false;
    }

    Engine::CONVERTED_MESH mesh{};
    if (!Convert_Mesh(pAIScene, mesh))
    {
        std::cout << "Convert mesh failed" << "\n";
        return false;
    }

    std::filesystem::create_directories(outPath.parent_path());
    if (!Save_MeshBin(outPath, mesh))
    {
        std::cout << "Convert mesh failed" << "\n";
        return false;
    }

    const std::string strGUID = Generate_GUID_String();
    if (!Save_MetaJson(outMeta, strGUID, inPath, outPath, mesh))
    {
        std::cout << "Save meta json failed" << "\n";
        return false;
    }

    return true;
}

NS_END
