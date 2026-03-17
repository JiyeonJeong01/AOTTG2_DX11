#include "MeshBuilder.h"

#include "BuiltIn_GUID.h"
#include "Render_Struct.h"
#include "Skeleton.h"
#include "Animation_Clip.h"

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

MODEL_TYPE CMeshBuilder::Peek_ModelType(const std::filesystem::path& modelPath)
{
    std::ifstream ifs(modelPath);
    if (!ifs.is_open())
        return Engine::MODEL_TYPE::NONANIM;

    std::string line;
    while (std::getline(ifs, line))
    {
        if (line.empty())
            continue;

        std::string key, value;
        if (!Split_KeyValue(line, key, value))
            continue;

        if (key == "boneCount" || key == "animCount")
            return Engine::MODEL_TYPE::ANIM;
    }

    return Engine::MODEL_TYPE::NONANIM;
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

    return hr;
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

HRESULT CMeshBuilder::Load_Anim_ModelDesc(const std::filesystem::path& modelPath, ANIM_MODEL_DESC& outDesc)
{
    std::ifstream ifs(modelPath);
    IF_TRUE_RETURN_MSG_BREAK(!ifs.is_open(), E_FAIL, "Load_AnimModelDesc failed: file open failed");

    outDesc = ANIM_MODEL_DESC{};

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
        else if (key == "boneCount")
        {
            const uint32_t count = (uint32_t)std::stoul(value);
            outDesc.tSkeleton.bones.resize(count);
        }
        else if (key == "animCount")
        {
            const uint32_t count = (uint32_t)std::stoul(value);
            outDesc.vecAnimClips.resize(count);
        }
        else if (key.rfind("part", 0) == 0)
        {
            const size_t namePos = key.find("Name");
            const size_t meshGuidPos = key.find("MeshGuid");
            const size_t materialGuidPos = key.find("MaterialGuid");
            const size_t boneCountPos = key.find("BoneCount");
            const size_t boneIndexPos = key.find("BoneIndex");
            const size_t offsetPos = key.find("Offset");

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
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.parts.size())
                    outDesc.parts.resize(idx + 1);

                ASSET_GUID::Try_Utf8_To_GUID(value, outDesc.parts[idx].tMeshGUID);
            }
            else if (materialGuidPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, materialGuidPos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.parts.size())
                    outDesc.parts.resize(idx + 1);

                ASSET_GUID::Try_Utf8_To_GUID(value, outDesc.parts[idx].tMaterialGUID);
            }
            else if (boneCountPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, boneCountPos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.parts.size())
                    outDesc.parts.resize(idx + 1);

                const uint32_t boneCount = (uint32_t)std::stoul(value);
                outDesc.parts[idx].vecBoneIndices.resize(boneCount);
                outDesc.parts[idx].vecOffsetMatrices.resize(boneCount);
            }
            else if (boneIndexPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, boneIndexPos - 4);
                const uint32_t partIdx = (uint32_t)std::stoul(numStr);

                const std::string suffix = key.substr(boneIndexPos + strlen("BoneIndex"));
                const uint32_t boneIdx = (uint32_t)std::stoul(suffix);

                if (partIdx >= outDesc.parts.size())
                    outDesc.parts.resize(partIdx + 1);

                if (boneIdx >= outDesc.parts[partIdx].vecBoneIndices.size())
                    outDesc.parts[partIdx].vecBoneIndices.resize(boneIdx + 1);

                outDesc.parts[partIdx].vecBoneIndices[boneIdx] = (uint32_t)std::stoul(value);
            }
            else if (offsetPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, offsetPos - 4);
                const uint32_t partIdx = (uint32_t)std::stoul(numStr);

                const std::string suffix = key.substr(offsetPos + strlen("Offset"));
                const uint32_t offsetIdx = (uint32_t)std::stoul(suffix);

                if (partIdx >= outDesc.parts.size())
                    outDesc.parts.resize(partIdx + 1);

                if (offsetIdx >= outDesc.parts[partIdx].vecOffsetMatrices.size())
                    outDesc.parts[partIdx].vecOffsetMatrices.resize(offsetIdx + 1);

                Parse_Float4x4(value, outDesc.parts[partIdx].vecOffsetMatrices[offsetIdx]);
            }
        }
        else if (key.rfind("bone", 0) == 0)
        {
            const size_t namePos = key.find("Name");
            const size_t parentPos = key.find("Parent");
            const size_t localBindPos = key.find("LocalBind");
            const size_t offsetPos = key.find("Offset");

            if (namePos != std::string::npos)
            {
                const std::string numStr = key.substr(4, namePos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.tSkeleton.bones.size())
                    outDesc.tSkeleton.bones.resize(idx + 1);

                outDesc.tSkeleton.bones[idx].strName = value;
                outDesc.tSkeleton.BoneNameToIndex[value] = idx;
            }
            else if (parentPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, parentPos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.tSkeleton.bones.size())
                    outDesc.tSkeleton.bones.resize(idx + 1);

                outDesc.tSkeleton.bones[idx].iParentBoneIndex = std::stoi(value);
            }
            else if (localBindPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, localBindPos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.tSkeleton.bones.size())
                    outDesc.tSkeleton.bones.resize(idx + 1);

                Parse_Float4x4(value, outDesc.tSkeleton.bones[idx].matLocalBind);
            }
            else if (offsetPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, offsetPos - 4);
                const uint32_t idx = (uint32_t)std::stoul(numStr);

                if (idx >= outDesc.tSkeleton.bones.size())
                    outDesc.tSkeleton.bones.resize(idx + 1);

                Parse_Float4x4(value, outDesc.tSkeleton.bones[idx].matOffset);
            }
        }
        else if (key.rfind("anim", 0) == 0)
        {
            const size_t namePos = key.find("Name");
            const size_t durationPos = key.find("Duration");
            const size_t tickPos = key.find("TickPerSecond");
            const size_t channelCountPos = key.find("ChannelCount");
            const size_t channelPos = key.find("Channel");

            if (namePos != std::string::npos && channelPos == std::string::npos)
            {
                const std::string numStr = key.substr(4, namePos - 4);
                const uint32_t animIdx = (uint32_t)std::stoul(numStr);

                if (animIdx >= outDesc.vecAnimClips.size())
                    outDesc.vecAnimClips.resize(animIdx + 1);

                outDesc.vecAnimClips[animIdx].strName = value;
            }
            else if (durationPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, durationPos - 4);
                const uint32_t animIdx = (uint32_t)std::stoul(numStr);

                if (animIdx >= outDesc.vecAnimClips.size())
                    outDesc.vecAnimClips.resize(animIdx + 1);

                outDesc.vecAnimClips[animIdx].fDuration = std::stof(value);
            }
            else if (tickPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, tickPos - 4);
                const uint32_t animIdx = (uint32_t)std::stoul(numStr);

                if (animIdx >= outDesc.vecAnimClips.size())
                    outDesc.vecAnimClips.resize(animIdx + 1);

                outDesc.vecAnimClips[animIdx].fTickPerSecond = std::stof(value);
            }
            else if (channelCountPos != std::string::npos)
            {
                const std::string numStr = key.substr(4, channelCountPos - 4);
                const uint32_t animIdx = (uint32_t)std::stoul(numStr);

                if (animIdx >= outDesc.vecAnimClips.size())
                    outDesc.vecAnimClips.resize(animIdx + 1);

                const uint32_t count = (uint32_t)std::stoul(value);
                outDesc.vecAnimClips[animIdx].channels.resize(count);
            }
            else if (channelPos != std::string::npos)
            {
                const size_t boneNamePos = key.find("BoneName");
                const size_t boneIndexPos = key.find("BoneIndex");
                const size_t keyCountPos = key.find("KeyCount");
                const size_t keyPos = key.find("Key");

                const std::string animNumStr = key.substr(4, channelPos - 4);
                const uint32_t animIdx = (uint32_t)std::stoul(animNumStr);

                const size_t channelNumStart = channelPos + strlen("Channel");
                size_t channelNumEnd = std::string::npos;

                if (boneNamePos != std::string::npos) channelNumEnd = boneNamePos;
                else if (boneIndexPos != std::string::npos) channelNumEnd = boneIndexPos;
                else if (keyCountPos != std::string::npos) channelNumEnd = keyCountPos;
                else if (keyPos != std::string::npos) channelNumEnd = keyPos;

                const std::string channelNumStr = key.substr(channelNumStart, channelNumEnd - channelNumStart);
                const uint32_t channelIdx = (uint32_t)std::stoul(channelNumStr);

                if (animIdx >= outDesc.vecAnimClips.size())
                    outDesc.vecAnimClips.resize(animIdx + 1);

                if (channelIdx >= outDesc.vecAnimClips[animIdx].channels.size())
                    outDesc.vecAnimClips[animIdx].channels.resize(channelIdx + 1);

                ANIMATION_CHANNEL_ENTRY& ch = outDesc.vecAnimClips[animIdx].channels[channelIdx];

                if (boneNamePos != std::string::npos)
                {
                    ch.strBoneName = value;
                }
                else if (boneIndexPos != std::string::npos)
                {
                    ch.iBoneIndex = std::stoi(value);
                }
                else if (keyCountPos != std::string::npos)
                {
                    const uint32_t keyCount = (uint32_t)std::stoul(value);
                    ch.vecKeyFrames.resize(keyCount);
                }
                else if (keyPos != std::string::npos)
                {
                    const std::string keyNumStr = key.substr(keyPos + strlen("Key"));
                    const uint32_t keyIdx = (uint32_t)std::stoul(keyNumStr);

                    if (keyIdx >= ch.vecKeyFrames.size())
                        ch.vecKeyFrames.resize(keyIdx + 1);

                    Parse_AnimKeyFrame(value, ch.vecKeyFrames[keyIdx]);
                }
            }
        }
    }

    /* skeleton child 정보 재구성 */
    for (uint32_t i = 0; i < outDesc.tSkeleton.bones.size(); ++i)
    {
        const int32_t iParent = outDesc.tSkeleton.bones[i].iParentBoneIndex;
        if (iParent >= 0)
        {
            outDesc.tSkeleton.bones[iParent].vecChildBoneIndices.push_back(i);
        }
        else if (outDesc.tSkeleton.iRootBoneIndex < 0)
        {
            outDesc.tSkeleton.iRootBoneIndex = (int32_t)i;
        }
    }

    return outDesc.parts.empty() ? E_FAIL : S_OK;
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
