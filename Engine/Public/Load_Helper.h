#pragma once

#include "Converter_Struct.h"
#include "Mesh.h"

NS_BEGIN(Engine)

inline _bool Load_Mesh_Header(ID3D11Device* pDevice, const std::filesystem::path& meshPath, MESH_ENTRY& outEntry)
{
    std::ifstream ifs(meshPath, std::ios_base::binary);

    if (!ifs.is_open())
        return false;

    MESH_HEADER hdr{};
    ifs.read((char*)&hdr, sizeof(hdr));

    if (hdr.vertexCount == 0 || hdr.indexCount == 0)
        return false;

    std::vector<VTXMESH> vertices(hdr.vertexCount);
    std::vector<uint32_t> indices(hdr.indexCount);

    ifs.read((char*)vertices.data(), sizeof(VTXMESH) * hdr.vertexCount);
    ifs.read((char*)indices.data(), sizeof(uint32_t) * hdr.indexCount);

    _float3 vMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    _float3 vMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (const auto& vtx : vertices)
    {
        vMin.x = min(vMin.x, vtx.vPosition.x);
        vMin.y = min(vMin.y, vtx.vPosition.y);
        vMin.z = min(vMin.z, vtx.vPosition.z);

        vMax.x = max(vMax.x, vtx.vPosition.x);
        vMax.y = max(vMax.y, vtx.vPosition.y);
        vMax.z = max(vMax.z, vtx.vPosition.z);
    }

    /* 버텍스 버퍼 생성 */
    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.ByteWidth = sizeof(VTXMESH) * hdr.vertexCount;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.StructureByteStride = sizeof(VTXMESH);
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA vbData;
    vbData.pSysMem = vertices.data();

    ID3D11Buffer* pVB = nullptr;
    HRESULT hr = pDevice->CreateBuffer(&vbDesc, &vbData, &pVB);
    if (FAILED(hr))
        return false;

    /* 인덱스 버퍼 생성 */
    D3D11_BUFFER_DESC ibDesc{};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = (UINT)(sizeof(uint32_t) * indices.size());
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData{};
    ibData.pSysMem = indices.data();

    ID3D11Buffer* pIB = nullptr;
    hr = pDevice->CreateBuffer(&ibDesc, &ibData, &pIB);
    if (FAILED(hr))
    {
        pVB->Release();
        return false;
    }

    /* OutEntry 채우기 */
    outEntry.pVB = pVB;
    outEntry.pIB = pIB;
    outEntry.iVertexCount = (uint32_t)vertices.size();
    outEntry.iIndexCount = (uint32_t)indices.size();
    outEntry.iVertexStride = sizeof(VTXMESH);
    outEntry.eIndexFormat = DXGI_FORMAT_R32_UINT; /* 32비트 기준 */
    outEntry.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    outEntry.minAABB = vMin;
    outEntry.maxAABB = vMax;

    return true;
}

NS_END
