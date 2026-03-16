#pragma once

#include "Engine_Define.h"
#include "Component_Struct.h"

NS_BEGIN(Engine)

typedef struct tagVertexPositionColor
{
    XMFLOAT3			vPosition;
    XMFLOAT4			vColor;

    static const unsigned int iNumElements = 2;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
}VTXCOL;

typedef struct tagVertexPositionTexcoord
{
    XMFLOAT3			vPosition;
    XMFLOAT2			vTexcoord;

    static const unsigned int iNumElements = 2;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
}VTXTEX;

typedef struct tagVertexPositionNormalTexcoord
{
    XMFLOAT3 vPosition;
    XMFLOAT3 vNormal;
    XMFLOAT2 vTexcoord;

    static const unsigned int iNumElements = 3;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[iNumElements] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
} VTXNORTEX;

typedef struct tagVertexMesh
{
    XMFLOAT3 vPosition;
    XMFLOAT3 vNormal;
    XMFLOAT3 vTangent;
    XMFLOAT2 vTexcoord;

    static const unsigned int iNumElements = 4;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
} VTXMESH;

typedef struct ENGINE_DLL tagILDesc
{
    const D3D11_INPUT_ELEMENT_DESC* pDesc;
    uint32_t                        iCount;
}IL_DESC;

/* Blueprints for gpu to interpret memory chuncks  */
static constexpr IL_DESC g_IL_TABLE[] = {
    { VTXCOL::Elements, VTXCOL::iNumElements },
    { VTXTEX::Elements, VTXTEX::iNumElements },
    { VTXNORTEX::Elements, VTXNORTEX::iNumElements },
    { VTXMESH::Elements, VTXMESH::iNumElements }
};

typedef struct ENGINE_DLL tagDrawCmd final
{
    uint64_t sortKey = 0;

    RENDER_LAYER eLayer = RENDER_LAYER::NONBLEND;
    DRAW_TYPE kind = DRAW_TYPE::MESH;
    uint8_t   pad0[7] = {}; // 8바이트 정렬(선택)

    union
    {
        struct
        {
            uint32_t        hMesh = INVALID_HANDLE_UINT;
            uint32_t        hMaterial = INVALID_HANDLE_UINT;
            COMPONENT_HANDLE hTransform{};

            uint32_t        flags = RF_NONE;
            uint32_t        firstIndex = INVALID_HANDLE_UINT;
            uint32_t        indexCount = INVALID_HANDLE_UINT;
            uint32_t        hPerObjectParams = INVALID_HANDLE_UINT;
        } mesh;

        struct
        {
            uint32_t        hMesh = INVALID_HANDLE_UINT;
        } line;

        struct
        {
            // UI는 보통 rect mesh는 Processor가 고정으로 들고감
            uint32_t        hMaterial = 0;
            uint32_t        hTexture = 0;
            COMPONENT_HANDLE hRectTransform{};

            uint32_t        flags = 0;    // CF_* (clip 등)
            _float          sortZ = 0.f;  // UI depth
            RECT_F          rcUV = { 0.f, 0.f, 1.f, 1.f };   // atlas 쓰면
            _float4         vColor = { 1.f, 1.f, 1.f, 1.f };// tint/alpha
            RECT_F          rcClip = { 0.f, 0.f, 0.f, 0.f };// clip on이면
        } canvas;
    };

public:
    static tagDrawCmd Create_Mesh(uint32_t hMesh, uint32_t hMat, COMPONENT_HANDLE hTr, uint32_t flags, uint32_t first, uint32_t count)
    {
        tagDrawCmd c{};
        c.kind = DRAW_TYPE::MESH;
        c.mesh.hMesh = hMesh;
        c.mesh.hMaterial = hMat;
        c.mesh.hTransform = hTr;
        c.mesh.flags = flags;
        c.mesh.firstIndex = first;
        c.mesh.indexCount = count;
        return c;
    }

    static tagDrawCmd Create_Line(uint32_t hMesh, DRAW_TYPE eType, RENDER_LAYER eLayer)
    {
        tagDrawCmd c{};
        c.kind = DRAW_TYPE::LINE;
        c.line.hMesh = hMesh;
       
        return c;
    }

    static tagDrawCmd Create_Canvas(uint32_t hMat, uint32_t hTex, COMPONENT_HANDLE hRectTr, uint32_t flags,
        float sortZ, const RECT_F& uv, const _float4& color, const RECT_F& clip)
    {
        tagDrawCmd c{};
        c.kind = DRAW_TYPE::CANVAS;
        c.canvas.hMaterial = hMat;
        c.canvas.hTexture = hTex;
        c.canvas.hRectTransform = hRectTr;
        c.canvas.flags = flags;
        c.canvas.sortZ = sortZ;
        c.canvas.rcUV = uv;
        c.canvas.vColor = color;
        c.canvas.rcClip = clip;
        return c;
    }

} DRAW_CMD;

using ParamType = std::variant<_float, _float4, _float4x4, uint32_t>;

typedef struct tagParamValue
{
    PARAM_TYPE  eType{};
    ParamType   data{};
}PARAM_VALUE;

typedef struct tagNameValueParam
{
    std::string strName;
    PARAM_VALUE value{};
}NAME_VALUE_PARAM;

typedef struct tagNameValueParamBlock
{
    std::vector<NAME_VALUE_PARAM>   params;

    void Clear()
    {
        params.clear();
    }

    NAME_VALUE_PARAM* Find(const std::string& strName)
    {
        for (auto& it : params)
            if (it.strName == strName)
                return &it;

        return nullptr;
    }

    void Set_Float(const std::string& strName, _float fValue)
    {
        NAME_VALUE_PARAM* pParam = Find(strName);
        if (!pParam)
        {
            params.push_back(NAME_VALUE_PARAM{});
            pParam = &params.back();
            pParam->strName = strName;
        }

        pParam->value.eType = PARAM_TYPE::FLOAT;
        pParam->value.data = fValue;
    }
    void Set_Float4(const std::string& strName, const _float4& vValue)
    {
        NAME_VALUE_PARAM* pParam = Find(strName);
        if (!pParam)
        {
            params.push_back({ strName });
            pParam = &params.back();
        }

        pParam->value.eType = PARAM_TYPE::FLOAT4;
        pParam->value.data = vValue;
    }

    void Set_Matrix(const std::string& strName, const _float4x4& mValue)
    {
        NAME_VALUE_PARAM* pParam = Find(strName);
        if (!pParam)
        {
            params.push_back({ strName });
            pParam = &params.back();
        }

        pParam->value.eType = PARAM_TYPE::FLOAT4X4;
        pParam->value.data = mValue;
    }
    void Set_Texture(const std::string& strName, uint32_t hTex)
    {
        NAME_VALUE_PARAM* pParam = Find(strName);
        if (!pParam)
        {
            params.push_back({ strName });
            pParam = &params.back();
        }

        pParam->value.eType = PARAM_TYPE::TEXTURE_HANDLE;
        pParam->value.data = hTex;
    }
}NAME_VALUE_PARAM_BLOCK;

typedef struct tagPerObjectParamBlock
{
    NAME_VALUE_PARAM_BLOCK block;
}PER_OBJECT_PARAM_BLOCK;

class CPerObjectParamPool final
{
public:
    uint32_t Alloc()
    {
        if (!m_Free.empty())
        {
            const uint32_t h = m_Free.back();
            m_Free.pop_back();
            m_Items[h - 1].block.Clear();
            return h;
        }

        m_Items.emplace_back();
        const uint32_t hNew = (uint32_t)m_Items.size(); // 0 - invalid / 1 - base
        m_Items[hNew - 1].block.Clear();
        return hNew;
    }

    void Free(uint32_t h)
    {
        if (h == INVALID_HANDLE_UINT)
            return;
        if (h < 1 || h >(uint32_t)m_Items.size())
            return;

        m_Items[h - 1].block.Clear();
        m_Free.push_back(h);
    }

    PER_OBJECT_PARAM_BLOCK* Get(uint32_t h)
    {
        if (h == INVALID_HANDLE_UINT)
            return nullptr;
        if (h < 1 || h >(uint32_t)m_Items.size())
            return nullptr;
        return &m_Items[h - 1];
    }

private:
    std::vector<PER_OBJECT_PARAM_BLOCK> m_Items;
    std::vector<uint32_t> m_Free;
};


NS_END
