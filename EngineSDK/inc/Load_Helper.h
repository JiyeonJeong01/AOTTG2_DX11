#pragma once

#include "Converter_Struct.h"
#include "Mesh.h"

NS_BEGIN(Engine)

inline static void Compute_AABB_From_VTXMESH(const std::vector<VTXMESH>& vertices, _float3& outMin, _float3& outMax)
{
    outMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    outMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (const auto& vtx : vertices)
    {
        outMin.x = min(outMin.x, vtx.vPosition.x);
        outMin.y = min(outMin.y, vtx.vPosition.y);
        outMin.z = min(outMin.z, vtx.vPosition.z);

        outMax.x = max(outMax.x, vtx.vPosition.x);
        outMax.y = max(outMax.y, vtx.vPosition.y);
        outMax.z = max(outMax.z, vtx.vPosition.z);
    }
}

inline static void Compute_AABB_From_VTXANIMMESH(const std::vector<VTXANIMMESH>& vertices, _float3& outMin, _float3& outMax)
{
    outMin = { FLT_MAX, FLT_MAX, FLT_MAX };
    outMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

    for (const auto& vtx : vertices)
    {
        outMin.x = min(outMin.x, vtx.vPosition.x);
        outMin.y = min(outMin.y, vtx.vPosition.y);
        outMin.z = min(outMin.z, vtx.vPosition.z);

        outMax.x = max(outMax.x, vtx.vPosition.x);
        outMax.y = max(outMax.y, vtx.vPosition.y);
        outMax.z = max(outMax.z, vtx.vPosition.z);
    }
}

template <typename TVertex>
inline static _bool Create_Mesh_Buffers(
    ID3D11Device* pDevice,
    const std::vector<TVertex>& vertices,
    const std::vector<uint32_t>& indices,
    _uint iVertexStride,
    const _float3& vMin,
    const _float3& vMax,
    MESH_ENTRY& outEntry)
{
    IF_NULL_RETURN_MSG_BREAK(pDevice, false, "Create_Mesh_Buffers failed : pDevice is nullptr");
    IF_TRUE_RETURN_MSG_BREAK(vertices.empty(), false, "Create_Mesh_Buffers failed : vertices empty");
    IF_TRUE_RETURN_MSG_BREAK(indices.empty(), false, "Create_Mesh_Buffers failed : indices empty");

    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.ByteWidth = static_cast<UINT>(iVertexStride * vertices.size());
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.StructureByteStride = iVertexStride;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.Usage = D3D11_USAGE_DEFAULT;

    D3D11_SUBRESOURCE_DATA vbData{};
    vbData.pSysMem = vertices.data();

    ID3D11Buffer* pVB = nullptr;
    HRESULT hr = pDevice->CreateBuffer(&vbDesc, &vbData, &pVB);
    if (FAILED(hr))
        return false;

    D3D11_BUFFER_DESC ibDesc{};
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * indices.size());
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

    outEntry.pVB = pVB;
    outEntry.pIB = pIB;
    outEntry.iVertexCount = static_cast<uint32_t>(vertices.size());
    outEntry.iIndexCount = static_cast<uint32_t>(indices.size());
    outEntry.iVertexStride = iVertexStride;
    outEntry.eIndexFormat = DXGI_FORMAT_R32_UINT;
    outEntry.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    outEntry.minAABB = vMin;
    outEntry.maxAABB = vMax;

    return true;
}

inline static _bool Load_NonAnim_Mesh_Body(
    ID3D11Device* pDevice,
    std::ifstream& ifs,
    const MESH_HEADER& hdr,
    MESH_ENTRY& outEntry)
{
    std::vector<VTXMESH> vertices(hdr.vertexCount);
    std::vector<uint32_t> indices(hdr.indexCount);

    ifs.read(reinterpret_cast<char*>(vertices.data()), sizeof(VTXMESH) * hdr.vertexCount);
    ifs.read(reinterpret_cast<char*>(indices.data()), sizeof(uint32_t) * hdr.indexCount);

    IF_TRUE_RETURN_MSG_BREAK(ifs.fail(), false, "Load_NonAnim_Mesh_Body failed : file read failed");

    _float3 vMin{}, vMax{};
    Compute_AABB_From_VTXMESH(vertices, vMin, vMax);

    return Create_Mesh_Buffers(
        pDevice,
        vertices,
        indices,
        sizeof(VTXMESH),
        vMin,
        vMax,
        outEntry);
}

inline static _bool Load_Anim_Mesh_Body(
    ID3D11Device* pDevice,
    std::ifstream& ifs,
    const MESH_HEADER& hdr,
    MESH_ENTRY& outEntry)
{
    std::vector<VTXANIMMESH> vertices(hdr.vertexCount);
    std::vector<uint32_t> indices(hdr.indexCount);

    ifs.read(reinterpret_cast<char*>(vertices.data()), sizeof(VTXANIMMESH) * hdr.vertexCount);
    ifs.read(reinterpret_cast<char*>(indices.data()), sizeof(uint32_t) * hdr.indexCount);

    IF_TRUE_RETURN_MSG_BREAK(ifs.fail(), false, "Load_Anim_Mesh_Body failed : file read failed");

    _float3 vMin{}, vMax{};
    Compute_AABB_From_VTXANIMMESH(vertices, vMin, vMax);

    return Create_Mesh_Buffers(
        pDevice,
        vertices,
        indices,
        sizeof(VTXANIMMESH),
        vMin,
        vMax,
        outEntry);
}

inline _bool Load_Mesh_Header(ID3D11Device* pDevice, const std::filesystem::path& meshPath, MESH_ENTRY& outEntry)
{
    std::ifstream ifs(meshPath, std::ios_base::binary);
    if (!ifs.is_open())
        return false;

    MESH_HEADER hdr{};
    ifs.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));

    IF_TRUE_RETURN_MSG_BREAK(ifs.fail(), false, "Load_Mesh_Header failed : header read failed");
    IF_TRUE_RETURN_MSG_BREAK(hdr.vertexCount == 0 || hdr.indexCount == 0, false, "Load_Mesh_Header failed : invalid counts");

    /* 필요시 magic/version 검사 추가 */
    IF_TRUE_RETURN_MSG_BREAK(hdr.iMagic != 0x4D534842, false, "Load_Mesh_Header failed : invalid magic");

    if (hdr.vertexStride == sizeof(VTXMESH))
    {
        return Load_NonAnim_Mesh_Body(pDevice, ifs, hdr, outEntry);
    }
    else if (hdr.vertexStride == sizeof(VTXANIMMESH))
    {
        return Load_Anim_Mesh_Body(pDevice, ifs, hdr, outEntry);
    }

    return false;
}

NS_END
