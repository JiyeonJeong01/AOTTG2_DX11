#pragma once

#include "Render_Struct.h"
#include "Identity.h"

NS_BEGIN(Engine)


typedef struct tagConvertedMesh
{
    std::vector<Engine::VTXMESH> vertices;
    std::vector<uint32_t> indices;
} CONVERTED_MESH;

typedef struct tagConvertedModelPart
{
    std::string     strName;
    CONVERTED_MESH  mesh;
    uint32_t        iMaterialIndex = 0;

} CONVERTED_MODEL_PART;

typedef struct tagConvertedModel
{
    std::vector<CONVERTED_MODEL_PART> parts;
} CONVERTED_MODEL;

typedef struct tagSavedModelPartInfo
{
    std::string strName;
    std::string strMeshGUID;
    std::string strMaterialGUID;
}SAVED_MODEL_PART_INFO;

typedef struct tagModelPartDesc
{
    std::string strName;
    ASSET_GUID  tMeshGUID{};
    ASSET_GUID  tMaterialGUID{};
}MODEL_PART_DESC;

typedef struct tagModelDesc
{
    ASSET_GUID tGUID{};
    std::filesystem::path pathSource;
    std::vector<MODEL_PART_DESC> parts;
}MODEL_DESC;

typedef struct tagMeshHeader
{
    /* Magic Number : 0x4D534842 -> 'MSHB' */
    uint32_t iMagic = 0x4D534842;

    uint32_t iVersion = 1;

    uint32_t vertexCount = 0;
    uint32_t indexCount = 0;

    uint32_t vertexStride = sizeof(VTXMESH);
    uint32_t reserved0 = 0;
}MESH_HEADER;

NS_END
