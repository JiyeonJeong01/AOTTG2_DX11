#pragma once

#include "Render_Struct.h"

NS_BEGIN(Engine)

typedef struct tagConvertedMesh
{
    std::vector<Engine::VTXMESH> vertices;
    std::vector<uint32_t> indices;
}CONVERTED_MESH;

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
