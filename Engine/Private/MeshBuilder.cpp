#include "MeshBuilder.h"

#include "BuiltIn_GUID.h"
#include "Render_Struct.h"
#include "Skeleton.h"
#include "Animation_Clip.h"

static _bool Read_Float4x4_Bin(std::ifstream& ifs, _float4x4& outMat)
{
    _float values[16]{};
    ifs.read(reinterpret_cast<char*>(values), sizeof(values));

    if (!ifs.good())
        return false;

    outMat._11 = values[0];  outMat._12 = values[1];  outMat._13 = values[2];  outMat._14 = values[3];
    outMat._21 = values[4];  outMat._22 = values[5];  outMat._23 = values[6];  outMat._24 = values[7];
    outMat._31 = values[8];  outMat._32 = values[9];  outMat._33 = values[10]; outMat._34 = values[11];
    outMat._41 = values[12]; outMat._42 = values[13]; outMat._43 = values[14]; outMat._44 = values[15];

    return true;
}

_bool CMeshBuilder::Split_KeyValue(const std::string& line, std::string& outKey, std::string& outValue)
{
    const size_t pos = line.find('=');
    if (pos == std::string::npos)
        return false;

    outKey = line.substr(0, pos);
    outValue = line.substr(pos + 1);
    return true;
}

static _bool Read_String_Bin(std::ifstream& ifs, std::string& outStr, uint32_t iLength)
{
    outStr.clear();

    if (iLength == 0)
        return true;

    outStr.resize(iLength);
    ifs.read(outStr.data(), static_cast<std::streamsize>(iLength));

    return ifs.good();
}

HRESULT CMeshBuilder::Create_Mesh(ID3D11Device* pDevice, const MESH_DESC& tDesc, MESH_ENTRY& outEntry)
{
    if (!pDevice || !tDesc.pVertices || !tDesc.iVertexStride || !tDesc.iVertexCnt || !tDesc.pIndices || !tDesc.iIndexCnt)
        return E_FAIL;

    /* Vertex buffer */
    D3D11_BUFFER_DESC vbDesc{};
    vbDesc.ByteWidth = (UINT)(tDesc.iVertexStride * tDesc.iVertexCnt);
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = (UINT)tDesc.iVertexStride;

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
    outEntry.iVertexStride = tDesc.iVertexStride;
    outEntry.iVertexCount = tDesc.iVertexCnt;
    outEntry.eIndexFormat = tDesc.eIndexFormat;
    outEntry.iIndexCount = tDesc.iIndexCnt;
    outEntry.eTopology = tDesc.eTopology;
    outEntry.iVBOffset = 0;

    /* AABB 구하기 */
    {
        const uint8_t* base = reinterpret_cast<const uint8_t*>(tDesc.pVertices);

        float fMinX = std::numeric_limits<float>::infinity();
        float fMinY = std::numeric_limits<float>::infinity();
        float fMinZ = std::numeric_limits<float>::infinity();
        float fMaxX = -std::numeric_limits<float>::infinity();
        float fMaxY = -std::numeric_limits<float>::infinity();
        float fMaxZ = -std::numeric_limits<float>::infinity();

        for (UINT i = 0; i < (UINT)tDesc.iVertexCnt; ++i)
        {
            const uint8_t* vptr = base + (size_t)i * (size_t)tDesc.iVertexStride;
            const float* pos = reinterpret_cast<const float*>(vptr);

            // pos[0]=x, pos[1]=y, pos[2]=z 라고 가정
            const float x = pos[0];
            const float y = pos[1];
            const float z = pos[2];

            fMinX = (x < fMinX) ? x : fMinX;
            fMaxX = (x > fMaxX) ? x : fMaxX;

            fMinY = (y < fMinY) ? y : fMinY;
            fMaxY = (y > fMaxY) ? y : fMaxY;

            fMinZ = (z < fMinZ) ? z : fMinZ;
            fMaxZ = (z > fMaxZ) ? z : fMaxZ;
        }

        outEntry.minAABB = { fMinX, fMinY, fMinZ };
        outEntry.maxAABB = { fMaxX, fMaxY, fMaxZ };
    }
    return S_OK;
}

HRESULT CMeshBuilder::Create_Mesh_By_Geometry(ID3D11Device* pDevice, Geometry eGeometry, MESH_ENTRY& outEntry)
{
    switch (eGeometry)
    {
    case Geometry::Circle :
    case Geometry::Rect :
        return Create_Rect_VtxTex(pDevice, outEntry);
    case Geometry::Sphere :
        return Create_Sphere_VtxCol(pDevice, outEntry);
    case Geometry::Cube :
        return Create_Cube_VtxCol(pDevice, outEntry);
    }

    return E_FAIL;
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
    d.iVertexStride = sizeof(VTXTEX);
    d.iVertexCnt = 4;

    d.pIndices = idx;
    d.iIndexCnt = 6;
    d.eIndexFormat = DXGI_FORMAT_R16_UINT;
    d.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return Create_Mesh(pDevice, d, outEntry);
}

HRESULT CMeshBuilder::Create_Rect_VtxNorTex(ID3D11Device* pDevice, MESH_ENTRY& outEntry)
{
    VTXNORTEX v[4]{};
    v[0].vPosition = _float3(-0.5f, 0.f, 0.5f); v[0].vTexcoord = _float2(0.f, 0.f);
    v[1].vPosition = _float3(0.5f, 0.f, 0.5f); v[1].vTexcoord = _float2(1.f, 0.f);
    v[2].vPosition = _float3(0.5f, -0.f, -0.5f); v[2].vTexcoord = _float2(1.f, 1.f);
    v[3].vPosition = _float3(-0.5f, -0.f, -0.5f); v[3].vTexcoord = _float2(0.f, 1.f);

    uint16_t idx[6] = { 0,1,2, 0,2,3 };

    MESH_DESC d{};
    d.pVertices = v;
    d.iVertexStride = sizeof(VTXNORTEX);
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
    d.iVertexStride = sizeof(VTXCOL);
    d.iVertexCnt = 8;
    d.pIndices = idx;
    d.iIndexCnt = 36;
    d.eIndexFormat = DXGI_FORMAT_R16_UINT;
    d.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return Create_Mesh(pDevice, d, outEntry);
}

HRESULT CMeshBuilder::Create_Cube_VtxTex(ID3D11Device* pDevice, MESH_ENTRY& outEntry)
{
    VTXCUBE v[8];
    v[0].vPosition = XMFLOAT3(-0.5f, 0.5f, -0.5f);
    v[1].vPosition = XMFLOAT3(0.5f, 0.5f, -0.5f);
    v[2].vPosition = XMFLOAT3(0.5f, -0.5f, -0.5f);
    v[3].vPosition = XMFLOAT3(-0.5f, -0.5f, -0.5f);
    v[4].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.5f);
    v[5].vPosition = XMFLOAT3(0.5f, 0.5f, 0.5f); 
    v[6].vPosition = XMFLOAT3(0.5f, -0.5f, 0.5f);
    v[7].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.5f);

    for (int i = 0; i < 8; ++i)
        v[i].vTexcoord = v[i].vPosition;

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
    d.iVertexStride = sizeof(VTXCUBE);
    d.iVertexCnt = 8;
    d.pIndices = idx;
    d.iIndexCnt = 36;
    d.eIndexFormat = DXGI_FORMAT_R16_UINT;
    d.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return Create_Mesh(pDevice, d, outEntry);
}

HRESULT CMeshBuilder::Create_Cube_VtxNorTex(ID3D11Device* pDevice, MESH_ENTRY& outEntry)
{
    VTXCUBENOR v[8];
    v[0].vPosition = XMFLOAT3(-0.5f, 0.5f, -0.5f);
    v[1].vPosition = XMFLOAT3(0.5f, 0.5f, -0.5f);
    v[2].vPosition = XMFLOAT3(0.5f, -0.5f, -0.5f);
    v[3].vPosition = XMFLOAT3(-0.5f, -0.5f, -0.5f);
    v[4].vPosition = XMFLOAT3(-0.5f, 0.5f, 0.5f);
    v[5].vPosition = XMFLOAT3(0.5f, 0.5f, 0.5f);
    v[6].vPosition = XMFLOAT3(0.5f, -0.5f, 0.5f);
    v[7].vPosition = XMFLOAT3(-0.5f, -0.5f, 0.5f);

    for (int i = 0; i < 8; ++i)
    {
        v[i].vTexcoord = v[i].vPosition;
        XMStoreFloat3(&v[i].vNormal, XMVector3Normalize(XMLoadFloat3(&v[i].vPosition)));
    }

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
    d.iVertexStride = sizeof(VTXCUBENOR);
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
    d.iVertexStride = sizeof(VTXCOL);
    d.iVertexCnt = (uint32_t)vertices.size();
    d.pIndices = indices.data();
    d.iIndexCnt = (uint32_t)indices.size();
    d.eIndexFormat = DXGI_FORMAT_R16_UINT;
    d.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    return Create_Mesh(pDevice, d, outEntry);
}

HRESULT CMeshBuilder::Create_RibbonLine_VtxCol(ID3D11Device* pDevice, MESH_ENTRY& outEntry, _uint iNumPoints)
{
    if (pDevice == nullptr)
        return E_FAIL;

    if (iNumPoints < 2)
        return E_FAIL;

    const _uint iVertexCount = iNumPoints * 2;
    const _uint iIndexCount = (iNumPoints - 1) * 6;

    outEntry.iVertexStride = sizeof(VTXCOL);
    outEntry.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    outEntry.iVertexCount = iVertexCount;
    outEntry.iIndexCount = iIndexCount;
    outEntry.eIndexFormat = DXGI_FORMAT_R16_UINT;
    outEntry.iVBOffset = 0;

    D3D11_BUFFER_DESC VertexBufferDesc{};
    VertexBufferDesc.ByteWidth = sizeof(VTXCOL) * iVertexCount;
    VertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    VertexBufferDesc.MiscFlags = 0;
    VertexBufferDesc.StructureByteStride = 0;

    D3D11_BUFFER_DESC IndexBufferDesc{};
    IndexBufferDesc.ByteWidth = sizeof(uint16_t) * iIndexCount;
    IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.CPUAccessFlags = 0;
    IndexBufferDesc.MiscFlags = 0;
    IndexBufferDesc.StructureByteStride = 0;

    std::unique_ptr<VTXCOL[]> pVertices = std::make_unique<VTXCOL[]>(iVertexCount);
    std::unique_ptr<uint16_t[]> pIndices = std::make_unique<uint16_t[]>(iIndexCount);

    for (_uint i = 0; i < iVertexCount; ++i)
    {
        pVertices[i].vPosition = { 0.f, 0.f, 0.f };
        pVertices[i].vColor = { 0.f, 0.f, 0.f, 1.f };
    }

    _uint iIndex = 0;
    for (_uint i = 0; i < iNumPoints - 1; ++i)
    {
        const uint16_t i0 = static_cast<uint16_t>(i * 2 + 0);         // LB
        const uint16_t i1 = static_cast<uint16_t>(i * 2 + 1);         // RB
        const uint16_t i2 = static_cast<uint16_t>((i + 1) * 2 + 0);   // LT
        const uint16_t i3 = static_cast<uint16_t>((i + 1) * 2 + 1);   // RT

        pIndices[iIndex++] = i0;
        pIndices[iIndex++] = i2;
        pIndices[iIndex++] = i1;

        pIndices[iIndex++] = i2;
        pIndices[iIndex++] = i3;
        pIndices[iIndex++] = i1;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> pVB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pIB;
    HRESULT hr{};

    /* 버텍스 버퍼 생성 */
    D3D11_SUBRESOURCE_DATA VertexInitialData{};
    VertexInitialData.pSysMem = pVertices.get();
    hr = pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, pVB.GetAddressOf());
    IF_FAIL_RETURN_MSG_BREAK(hr, E_FAIL, "Ribbon Mesh create failed.");

    /* 인덱스 버퍼 생성 */
    D3D11_SUBRESOURCE_DATA IndexInitialData{};
    IndexInitialData.pSysMem = pIndices.get();
    hr = pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, pIB.GetAddressOf());
    IF_FAIL_RETURN_MSG_BREAK(hr, E_FAIL, "Ribbon Mesh create failed.");

    outEntry.pVB = pVB;
    outEntry.pIB = pIB;

    return S_OK;
}

HRESULT CMeshBuilder::Create_RibbonLine_VtxTrail(ID3D11Device* pDevice, MESH_ENTRY& outEntry, _uint iNumPoints)
{
    if (pDevice == nullptr) return E_FAIL;
    if (iNumPoints < 2) return E_FAIL;

    const _uint iVertexCount = iNumPoints * 2;
    const _uint iIndexCount = (iNumPoints - 1) * 6;

    outEntry.iVertexStride = sizeof(VTXTRAIL);
    outEntry.eTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    outEntry.iVertexCount = iVertexCount;
    outEntry.iIndexCount = iIndexCount;
    outEntry.eIndexFormat = DXGI_FORMAT_R16_UINT;
    outEntry.iVBOffset = 0;

    D3D11_BUFFER_DESC VertexBufferDesc{};
    VertexBufferDesc.ByteWidth = sizeof(VTXTRAIL) * iVertexCount;
    VertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    VertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    VertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    VertexBufferDesc.MiscFlags = 0;
    VertexBufferDesc.StructureByteStride = 0;

    D3D11_BUFFER_DESC IndexBufferDesc{};
    IndexBufferDesc.ByteWidth = sizeof(uint16_t) * iIndexCount;
    IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.CPUAccessFlags = 0;
    IndexBufferDesc.MiscFlags = 0;
    IndexBufferDesc.StructureByteStride = 0;

    std::unique_ptr<VTXTRAIL[]> pVertices = std::make_unique<VTXTRAIL[]>(iVertexCount);
    std::unique_ptr<uint16_t[]> pIndices = std::make_unique<uint16_t[]>(iIndexCount);

    for (_uint i = 0; i < iNumPoints; ++i)
    {
        const _float fV = (iNumPoints > 1) ? (To<_float>(i) / To<_float>(iNumPoints - 1)) : 0.f;
        pVertices[i * 2 + 0].vPosition = { 0.f, 0.f, 0.f };
        pVertices[i * 2 + 0].vTexcoord = { 0.f, fV };
        pVertices[i * 2 + 0].vColor = { 1.f, 1.f, 1.f, 0.f };

        pVertices[i * 2 + 1].vPosition = { 0.f, 0.f, 0.f };
        pVertices[i * 2 + 1].vTexcoord = { 1.f, fV };
        pVertices[i * 2 + 1].vColor = { 1.f, 1.f, 1.f, 0.f };
    }

    _uint iIndex = 0;
    for (_uint i = 0; i < iNumPoints - 1; ++i)
    {
        const uint16_t i0 = To<uint16_t>(i * 2 + 0);    // LB
        const uint16_t i1 = To<uint16_t>(i * 2 + 1);    // RB
        const uint16_t i2 = To<uint16_t>((i + 1) * 2 + 0);    // LT
        const uint16_t i3 = To<uint16_t>((i + 1) * 2 + 1);    // RT

        pIndices[iIndex++] = i0;
        pIndices[iIndex++] = i2;
        pIndices[iIndex++] = i1;

        pIndices[iIndex++] = i2;
        pIndices[iIndex++] = i3;
        pIndices[iIndex++] = i1;
    }

    Microsoft::WRL::ComPtr<ID3D11Buffer> pVB;
    Microsoft::WRL::ComPtr<ID3D11Buffer> pIB;
    HRESULT hr{};

    D3D11_SUBRESOURCE_DATA VertexInitialData{};
    VertexInitialData.pSysMem = pVertices.get();
    hr = pDevice->CreateBuffer(&VertexBufferDesc, &VertexInitialData, pVB.GetAddressOf());
    IF_FAIL_RETURN_MSG_BREAK(hr, E_FAIL, "Ribbon Trail Mesh create failed.");

    D3D11_SUBRESOURCE_DATA IndexInitialData{};
    IndexInitialData.pSysMem = pIndices.get();
    hr = pDevice->CreateBuffer(&IndexBufferDesc, &IndexInitialData, pIB.GetAddressOf());
    IF_FAIL_RETURN_MSG_BREAK(hr, E_FAIL, "Ribbon Trail Mesh create failed.");

    outEntry.pVB = pVB;
    outEntry.pIB = pIB;

    return S_OK;
}

HRESULT CMeshBuilder::Create_Builtin(ID3D11Device* pDevice, MESH_ENTRY& outEntry, const ASSET_GUID& tGUID)
{
    HRESULT hr = E_FAIL;
    if (tGUID == DEFAULT_ASSET_GUID::MESH_CUBE)
        hr = CMeshBuilder::Create_Cube_VtxCol(pDevice, outEntry);
    else if (tGUID == DEFAULT_ASSET_GUID::MESH_RECT)
        hr = CMeshBuilder::Create_Rect_VtxTex(pDevice, outEntry);
    else if (tGUID == DEFAULT_ASSET_GUID::MESH_SPHERE)
        hr = CMeshBuilder::Create_Sphere_VtxCol(pDevice, outEntry);
    else if (tGUID == DEFAULT_ASSET_GUID::MESH_RECT_NORTEX)
        hr = CMeshBuilder::Create_Rect_VtxNorTex(pDevice, outEntry);
    else if (tGUID == DEFAULT_ASSET_GUID::MESH_CUBE_TEX)
        hr = CMeshBuilder::Create_Cube_VtxTex(pDevice, outEntry);
    else if (tGUID == DEFAULT_ASSET_GUID::MESH_CUBE_NOR_TEX)
        hr = CMeshBuilder::Create_Cube_VtxNorTex(pDevice, outEntry);

    return hr;
}

MODEL_TYPE CMeshBuilder::Peek_ModelType(const std::filesystem::path& modelPath)
{
    std::ifstream ifs(modelPath, std::ios_base::binary);
    if (!ifs.is_open())
        return Engine::MODEL_TYPE::NONANIM;

    /* 1차: 바이너리 anim .model 판별 */
    {
        ANIM_MODEL_HEADER tHeader{};
        ifs.read(reinterpret_cast<char*>(&tHeader), sizeof(tHeader));

        if (ifs.good() &&
            tHeader.iMagic == 0x4D494E41 &&   /* 'ANIM' */
            tHeader.iVersion == 1)
        {
            return (tHeader.boneCount > 0 || tHeader.clipCount > 0)
                ? Engine::MODEL_TYPE::ANIM
                : Engine::MODEL_TYPE::NONANIM;
        }
    }
    /* 2차: 예전 key-value 텍스트 .model fallback */
    ifs.clear();
    ifs.seekg(0, std::ios_base::beg);

    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty())
            continue;

        std::string key, value;
        if (!Split_KeyValue(line, key, value))
            continue;

        if (key == "boneCount" || key == "animCount" || key == "clipCount")
            return Engine::MODEL_TYPE::ANIM;
    }

    return Engine::MODEL_TYPE::NONANIM;
}

HRESULT CMeshBuilder::Load_NonAnim_ModelDesc(const std::filesystem::path& modelPath, MODEL_DESC& outDesc)
{
    std::ifstream ifs(modelPath);
    IF_TRUE_RETURN_MSG_BREAK(!ifs.is_open(), E_FAIL, "Load_NonAnim_ModelDesc failed: file open failed");

    outDesc = MODEL_DESC{};

    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty())
            continue;

        std::string key, value;
        if (!Split_KeyValue(line, key, value))
            continue;

        if (key == "guid")
        {
            /* .model에 guid가 없을 수도 있으니, 있는 경우만 반영한다. */
            ASSET_GUID::Try_Utf8_To_GUID(value, outDesc.tGUID);
        }
        else if (key == "source")
        {
            outDesc.pathSource = value;
        }
        else if (key == "meshCount")
        {
            const uint32_t count = (uint32_t)std::stoul(value);
            outDesc.parts.resize(count);
        }
        else if (key.rfind("part", 0) == 0)
        {
            const size_t namePos = key.find("Name");
            const size_t meshGuidPos = key.find("MeshGuid");
            const size_t materialGuidPos = key.find("MaterialGuid");

            if (namePos != std::string::npos)
            {
                const std::string numStr = key.substr(4, namePos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.parts.size())
                    outDesc.parts.resize(idx + 1);

                outDesc.parts[idx].strName = value;
            }
            else if (meshGuidPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, meshGuidPos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr); /* part0 에서 0 같은 인덱스를 뽑아온다. */

                if (idx >= outDesc.parts.size())
                    outDesc.parts.resize(idx + 1);

                ASSET_GUID::Try_Utf8_To_GUID(value, outDesc.parts[idx].tMeshGUID);
            }
            else if (materialGuidPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, materialGuidPos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr); /* part0 에서 0 같은 인덱스를 뽑아온다. */

                if (idx >= outDesc.parts.size())
                    outDesc.parts.resize(idx + 1);

                ASSET_GUID::Try_Utf8_To_GUID(value, outDesc.parts[idx].tMaterialGUID);
            }
        }
    }

    /* guid가 없어도 괜찮고, parts가 비었으면 실패 */
    return outDesc.parts.empty() ? E_FAIL : S_OK;
}

static _bool Parse_Float4x4(const std::string& value, _float4x4& outMat)
{
    std::stringstream ss(value);
    std::string token;

    float vals[16]{};
    int i = 0;

    while (std::getline(ss, token, ','))
    {
        if (i >= 16)
            return false;

        vals[i++] = std::stof(token);
    }

    if (i != 16)
        return false;

    outMat._11 = vals[0];  outMat._12 = vals[1];  outMat._13 = vals[2];  outMat._14 = vals[3];
    outMat._21 = vals[4];  outMat._22 = vals[5];  outMat._23 = vals[6];  outMat._24 = vals[7];
    outMat._31 = vals[8];  outMat._32 = vals[9];  outMat._33 = vals[10]; outMat._34 = vals[11];
    outMat._41 = vals[12]; outMat._42 = vals[13]; outMat._43 = vals[14]; outMat._44 = vals[15];

    return true;
}

static _bool Parse_AnimKeyFrame(const std::string& value, ANIM_KEYFRAME& outKey)
{
    std::stringstream ss(value);
    std::string part;

    std::vector<std::string> chunks;
    while (std::getline(ss, part, '|'))
        chunks.push_back(part);

    if (chunks.size() != 4)
        return false;

    outKey.fTrackPosition = std::stof(chunks[0]);

    {
        std::stringstream s0(chunks[1]);
        std::string token;
        float vals[3]{};
        int i = 0;

        while (std::getline(s0, token, ','))
        {
            if (i >= 3)
                return false;
            vals[i++] = std::stof(token);
        }

        if (i != 3)
            return false;

        outKey.vScale = _float3(vals[0], vals[1], vals[2]);
    }

    {
        std::stringstream s1(chunks[2]);
        std::string token;
        float vals[4]{};
        int i = 0;

        while (std::getline(s1, token, ','))
        {
            if (i >= 4)
                return false;
            vals[i++] = std::stof(token);
        }

        if (i != 4)
            return false;

        outKey.vRotation = _float4(vals[0], vals[1], vals[2], vals[3]);
    }

    {
        std::stringstream s2(chunks[3]);
        std::string token;
        float vals[3]{};
        int i = 0;

        while (std::getline(s2, token, ','))
        {
            if (i >= 3)
                return false;
            vals[i++] = std::stof(token);
        }

        if (i != 3)
            return false;

        outKey.vTranslation = _float3(vals[0], vals[1], vals[2]);
    }

    return true;
}

static void Debug_Print_Float4x4(const char* pPrefix, const _float4x4& mat)
{
    std::cout
        << pPrefix
        << mat._11 << "," << mat._12 << "," << mat._13 << "," << mat._14 << ","
        << mat._21 << "," << mat._22 << "," << mat._23 << "," << mat._24 << ","
        << mat._31 << "," << mat._32 << "," << mat._33 << "," << mat._34 << ","
        << mat._41 << "," << mat._42 << "," << mat._43 << "," << mat._44 << "\n";
}
static _bool Read_String_Bin(std::ifstream& ifs, std::string& outStr)
{
    outStr.clear();

    uint32_t iLength = 0;
    ifs.read(reinterpret_cast<char*>(&iLength), sizeof(iLength));
    if (!ifs.good())
        return false;

    if (iLength == 0)
        return true;

    outStr.resize(iLength);
    ifs.read(outStr.data(), iLength);
    return ifs.good();
}

template <typename T>
static _bool Read_Value_Bin(std::ifstream& ifs, T& outValue)
{
    ifs.read(reinterpret_cast<char*>(&outValue), sizeof(T));
    return ifs.good();
}

template <typename T>
static _bool Read_Vector_Bin(std::ifstream& ifs, std::vector<T>& outVec)
{
    outVec.clear();

    uint32_t iCount = 0;
    if (!Read_Value_Bin(ifs, iCount))
        return false;

    if (iCount == 0)
        return true;

    outVec.resize(iCount);
    ifs.read(reinterpret_cast<char*>(outVec.data()), sizeof(T) * iCount);
    return ifs.good();
}

static _bool Read_Anim_ModelPartInfo_Bin(std::ifstream& ifs, ANIM_MODEL_PART_DESC& outPart)
{
    std::string strName;
    std::string strMeshGUID;
    std::string strMaterialGUID;

    if (!Read_String_Bin(ifs, strName))
        return false;
    if (!Read_String_Bin(ifs, strMeshGUID))
        return false;
    if (!Read_String_Bin(ifs, strMaterialGUID))
        return false;

    outPart = ANIM_MODEL_PART_DESC{};
    outPart.strName = strName;

    ASSET_GUID::Try_Utf8_To_GUID(strMeshGUID, outPart.tMeshGUID);
    ASSET_GUID::Try_Utf8_To_GUID(strMaterialGUID, outPart.tMaterialGUID);

    return true;
}

static _bool Read_Bone_Bin(std::ifstream& ifs, BONE_ENTRY& outBone)
{
    outBone = BONE_ENTRY{};

    if (!Read_String_Bin(ifs, outBone.strName))
        return false;

    BONE_HEADER_BIN tBoneHeader{};
    if (!Read_Value_Bin(ifs, tBoneHeader))
        return false;

    outBone.iParentBoneIndex = tBoneHeader.iParentBoneIndex;

    if (tBoneHeader.childCount > 0)
    {
        outBone.vecChildBoneIndices.resize(tBoneHeader.childCount);
        ifs.read(reinterpret_cast<char*>(outBone.vecChildBoneIndices.data()),
            sizeof(uint32_t) * tBoneHeader.childCount);
        if (!ifs.good())
            return false;
    }

    if (!Read_Value_Bin(ifs, outBone.matLocalBind))
        return false;
    if (!Read_Value_Bin(ifs, outBone.matCombinedBind))
        return false;
    if (!Read_Value_Bin(ifs, outBone.matOffset))
        return false;

    return true;
}

static _bool Read_Skeleton_Bin(std::ifstream& ifs, SKELETON_ENTRY& outSkeleton)
{
    outSkeleton = SKELETON_ENTRY{};

    SKELETON_HEADER_BIN tHeader{};
    if (!Read_Value_Bin(ifs, tHeader))
        return false;

    outSkeleton.iRootBoneIndex = tHeader.iRootBoneIndex;
    outSkeleton.bones.resize(tHeader.boneCount);

    for (uint32_t i = 0; i < tHeader.boneCount; ++i)
    {
        if (!Read_Bone_Bin(ifs, outSkeleton.bones[i]))
            return false;

        outSkeleton.BoneNameToIndex[outSkeleton.bones[i].strName] = i;
    }

    return true;
}

static _bool Read_Animation_Channel_Bin(std::ifstream& ifs, ANIMATION_CHANNEL_ENTRY& outChannel)
{
    outChannel = ANIMATION_CHANNEL_ENTRY{};

    if (!Read_String_Bin(ifs, outChannel.strBoneName))
        return false;

    ANIMATION_CHANNEL_HEADER_BIN tHeader{};
    if (!Read_Value_Bin(ifs, tHeader))
        return false;

    outChannel.iBoneIndex = tHeader.iBoneIndex;

    if (tHeader.keyFrameCount > 0)
    {
        outChannel.vecKeyFrames.resize(tHeader.keyFrameCount);
        ifs.read(reinterpret_cast<char*>(outChannel.vecKeyFrames.data()),
            sizeof(ANIM_KEYFRAME) * tHeader.keyFrameCount);
        if (!ifs.good())
            return false;
    }

    return true;
}

static _bool Read_Animation_Clip_Bin(std::ifstream& ifs, ANIMATION_CLIP_ENTRY& outClip)
{
    outClip = ANIMATION_CLIP_ENTRY{};

    if (!Read_String_Bin(ifs, outClip.strName))
        return false;

    ANIMATION_CLIP_HEADER_BIN tHeader{};
    if (!Read_Value_Bin(ifs, tHeader))
        return false;

    outClip.fDuration = tHeader.fDuration;
    outClip.fTickPerSecond = tHeader.fTickPerSecond;
    outClip.channels.resize(tHeader.channelCount);

    for (uint32_t i = 0; i < tHeader.channelCount; ++i)
    {
        if (!Read_Animation_Channel_Bin(ifs, outClip.channels[i]))
            return false;
    }

    return true;
}

HRESULT CMeshBuilder::Load_Anim_ModelDesc(const std::filesystem::path& modelPath, ANIM_MODEL_DESC& outDesc)
{
    std::ifstream ifs(modelPath, std::ios_base::binary);
    IF_TRUE_RETURN_MSG_BREAK(!ifs.is_open(), E_FAIL, "Load_Anim_ModelDesc failed: file open failed");

    outDesc = ANIM_MODEL_DESC{};

    ANIM_MODEL_HEADER tHeader{};
    if (!Read_Value_Bin(ifs, tHeader))
        return E_FAIL;

    if (tHeader.iMagic != 0x4D494E41 || tHeader.iVersion != 1)
        return E_FAIL;

    outDesc.parts.resize(tHeader.partCount);

    for (uint32_t i = 0; i < tHeader.partCount; ++i)
    {
        if (!Read_Anim_ModelPartInfo_Bin(ifs, outDesc.parts[i]))
            return E_FAIL;
    }

    if (!Read_Skeleton_Bin(ifs, outDesc.tSkeleton))
        return E_FAIL;

    outDesc.vecAnimClips.resize(tHeader.clipCount);
    for (uint32_t i = 0; i < tHeader.clipCount; ++i)
    {
        if (!Read_Animation_Clip_Bin(ifs, outDesc.vecAnimClips[i]))
            return E_FAIL;
    }

    return S_OK;
}
