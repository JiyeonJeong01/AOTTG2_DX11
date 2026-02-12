#include "MeshBuilder.h"

#include "Render_Struct.h"

HRESULT CMeshBuilder::Create_Mesh(ID3D11Device* pDevice, const MESH_DESC& tDesc, MESH_ENTRY& outEntry)
{
    if (!pDevice || !tDesc.pVertices || !tDesc.iVertextStride || !tDesc.iVertexCnt || !tDesc.pIndices || !tDesc.iIndexCnt)
        return E_FAIL;

    /* Vertex buffer */
    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.ByteWidth = (UINT)(tDesc.iVertextStride * tDesc.iVertexCnt);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = (UINT)tDesc.iVertextStride;

    D3D11_SUBRESOURCE_DATA vbInit{};
    vbInit.pSysMem = tDesc.pVertices;

    Microsoft::WRL::ComPtr<ID3D11Buffer> pVB;
    if (FAILED(pDevice->CreateBuffer(&vbDesc, &vbInit, pVB.GetAddressOf())))
        return E_FAIL;

    /* Index buffer */
    const UINT iIdxStride = (tDesc.eIndexFormat == DXGI_FORMAT_R16_UINT ? 2u : 4u);
    D3D11_BUFFER_DESC ibDesc{};
    ibDesc.ByteWidth = (UINT)(iIdxStride * tDesc.iIndexCnt);
    ibDesc.Usage = D3D11_USAGE_DEFAULT;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = 0;
    ibDesc.MiscFlags = 0;
    ibDesc.StructureByteStride = iIdxStride;

    D3D11_SUBRESOURCE_DATA ibInit{};
    ibInit.pSysMem = tDesc.pIndices;

    Microsoft::WRL::ComPtr<ID3D11Buffer> pIB;
    if (FAILED(pDevice->CreateBuffer(&ibDesc, &ibInit, pIB.GetAddressOf())))
        return E_FAIL;

    outEntry.pVB = pVB;
    outEntry.pIB = pIB;
    outEntry.iVertexStride = tDesc.iVertextStride;
    outEntry.iVertexCount = tDesc.iVertexCnt;
    outEntry.eIndexFormat = tDesc.eIndexFormat;
    outEntry.iIndexCount = tDesc.iIndexCnt;
    outEntry.eTopology = tDesc.eTopology;
    outEntry.iVBOffset = 0;
    return S_OK;
}

HRESULT CMeshBuilder::Create_Rect_VtxTex(ID3D11Device* pDevice, MESH_ENTRY& outEntry)
{
    VTXTEX v[4]{};
    v[0].vPosition = _float3(-0.5f, 0.5f, 0.f); v[0].vTexcoord = _float2(0.f, 0.f);
    v[1].vPosition = _float3(0.5f, 0.5f, 0.f); v[1].vTexcoord = _float2(1.f, 0.f);
    v[2].vPosition = _float3(0.5f, -0.5f, 0.f); v[2].vTexcoord = _float2(1.f, 1.f);
    v[3].vPosition = _float3(-0.5f, -0.5f, 0.f); v[3].vTexcoord = _float2(0.f, 1.f);

    uint16_t idx[6] = { 0,1,2, 0,2,3 };

    MESH_DESC d{};
    d.pVertices = v;
    d.iVertextStride = sizeof(VTXTEX);
    d.iVertexCnt = 4;

    d.pIndices = idx;
    d.iIndexCnt = 6;
    d.eIndexFormat = DXGI_FORMAT_R16_UINT;
    d.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return Create_Mesh(pDevice, d, outEntry);
}

HRESULT CMeshBuilder::Create_Cube_VtxCol(ID3D11Device* pDevice, MESH_ENTRY& outEntry)
{
    VTXCOL v[8];
    v[0].vPosition = XMFLOAT3(-0.5f, 0.5f, -0.5f); v[0].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    v[1].vPosition = XMFLOAT3(0.5f, 0.5f, -0.5f); v[1].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    v[2].vPosition = XMFLOAT3(0.5f, -0.5f, -0.5f); v[2].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    v[3].vPosition = XMFLOAT3(-0.5f, -0.5f, -0.5f); v[3].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    v[4].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.5f); v[4].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    v[5].vPosition = XMFLOAT3(0.5f, 0.5f, 0.5f); v[5].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    v[6].vPosition = XMFLOAT3(0.5f, -0.5f, 0.5f); v[6].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
    v[7].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.5f); v[7].vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);

    uint16_t idx[36] = {
        0, 1, 2, 0, 2, 3, // front
        1, 5, 6, 1, 6, 2, // right
        5, 4, 7, 5, 7, 6, // back
        4, 0, 3, 4, 3, 7, // left
        4, 5, 1, 4, 1, 0, // up
        3, 2, 6, 3, 6, 7  // down
    };

    MESH_DESC d{};
    d.pVertices = v;
    d.iVertextStride = sizeof(VTXCOL);
    d.iVertexCnt = 8;
    d.pIndices = idx;
    d.iIndexCnt = 36;
    d.eIndexFormat = DXGI_FORMAT_R16_UINT;
    d.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return Create_Mesh(pDevice, d, outEntry);
}

HRESULT CMeshBuilder::Create_Sphere_VtxCol(ID3D11Device* pDevice, MESH_ENTRY& outEntry, _uint iStack, _uint iSlice, _float fRadius)
{
    std::vector<VTXCOL> vertices;
    std::vector<uint16_t> indices;

    for (uint16_t i = 0; i <= iStack; ++i) {
        float phi = XM_PI * float(i) / iStack;
        for (uint16_t j = 0; j <= iSlice; ++j) {
            float theta = XM_2PI * float(j) / iSlice;

            VTXCOL v;
            v.vPosition.x = fRadius * sinf(phi) * cosf(theta);
            v.vPosition.y = fRadius * cosf(phi);
            v.vPosition.z = fRadius * sinf(phi) * sinf(theta);
            v.vColor = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
            vertices.push_back(v);
        }
    }

    for (uint16_t i = 0; i < iStack; ++i) {
        for (uint16_t j = 0; j < iSlice; ++j) {
            uint16_t first = i * (iSlice + 1) + j;
            uint16_t second = first + iSlice + 1;

            indices.push_back(first);
            indices.push_back(first + 1);
            indices.push_back(second);

            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second + 1);
        }
    }

    MESH_DESC d{};
    d.pVertices = vertices.data();
    d.iVertextStride = sizeof(VTXCOL);
    d.iVertexCnt = (uint32_t)vertices.size();
    d.pIndices = indices.data();
    d.iIndexCnt = (uint32_t)indices.size();
    d.eIndexFormat = DXGI_FORMAT_R16_UINT;
    d.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return Create_Mesh(pDevice, d, outEntry);
}
