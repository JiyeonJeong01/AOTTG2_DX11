#pragma once

#include "Engine_Define.h"
#include "Component_Struct.h"

NS_BEGIN(Engine)

typedef struct tagMeshRendererData MESH_RENDERER_DATA;
typedef struct tagSpriteEffectData SPRITE_EFFECT_DATA;

enum class EXTRA_RENDER_PASS : uint32_t
{
    NONE = 0,
    OUTLINE = 1 << 0,
};

enum class MATERIAL_RENDER_TYPE : uint8_t
{
    DEFAULT,
    OUTLINE
};

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

typedef struct tagVertexAnimMesh
{
    XMFLOAT3    vPosition;
    XMFLOAT3    vNormal;
    XMFLOAT3    vTangent;
    XMFLOAT2    vTexcoord;

    XMUINT4     vBlendIndex;
    XMFLOAT4    vBlendWeight;

    static const unsigned int iNumElements = 6;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA,0 },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA,0 },
    };
} VTXANIMMESH;

typedef struct tagVertexCube
{
    XMFLOAT3    vPosition;
    XMFLOAT3    vTexcoord;

    static const unsigned int iNumElements = 2;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
} VTXCUBE;

typedef struct tagVertexPosition
{
    XMFLOAT3			vPosition;

    static const unsigned int iNumElements = 1;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
} VTXPOS;

typedef struct tagVertexTrail
{
    XMFLOAT3 vPosition;
    XMFLOAT2 vTexcoord;
    XMFLOAT4 vColor;

    static const unsigned int iNumElements = 3;
    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
} VTXTRAIL;

typedef struct tagVertexParticlePointInstanceDesc
{
    static const unsigned int iNumElements = 6;

    static constexpr D3D11_INPUT_ELEMENT_DESC Elements[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },

        { "WORLD",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD",    1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD",    2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "WORLD",    3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
} VTXPARTICLE_POINTINSTANCE_DESC;

typedef struct ENGINE_DLL tagILDesc
{
    const D3D11_INPUT_ELEMENT_DESC* pDesc;
    uint32_t                        iCount;
}IL_DESC;

/* Blueprints for gpu to interpret memory chuncks  */
static constexpr IL_DESC g_IL_TABLE[] = {
    { VTXCOL::Elements, VTXCOL::iNumElements },                                             // 0
    { VTXTEX::Elements, VTXTEX::iNumElements },                                             // 1
    { VTXNORTEX::Elements, VTXNORTEX::iNumElements },                                       // 2 
    { VTXMESH::Elements, VTXMESH::iNumElements },                                           // 3
    { VTXANIMMESH::Elements, VTXANIMMESH::iNumElements },                                   // 4
    { VTXCUBE::Elements, VTXCUBE::iNumElements },                                           // 5 
    { VTXPOS::Elements, VTXPOS::iNumElements },                                             // 6
    { VTXPARTICLE_POINTINSTANCE_DESC::Elements, VTXPARTICLE_POINTINSTANCE_DESC::iNumElements },     // 7
    { VTXTRAIL::Elements, VTXTRAIL::iNumElements },     // 7
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
            uint32_t            hMesh = INVALID_HANDLE_UINT;
            uint32_t            hMaterial = INVALID_HANDLE_UINT;
            COMPONENT_HANDLE    hTransform{};
            COMPONENT_HANDLE    hAnimator = INVALID_HANDLE;

            uint32_t        flags = RF_NONE;
            uint32_t        firstIndex = INVALID_HANDLE_UINT;
            uint32_t        indexCount = INVALID_HANDLE_UINT;
            uint32_t        hPerObjectParams = INVALID_HANDLE_UINT;

            uint32_t        iParticleRuntime = INVALID_HANDLE_UINT;

            const std::vector<_float4x4>*   pSkinningMatrices = nullptr;

            MESH_MODE                       eMode = MESH_MODE::NONE;
            _float4x4                       matAttach{};

            const MESH_RENDERER_DATA* pMeshRendererData = nullptr;
        } mesh;

        struct
        {
            uint32_t        hMesh = INVALID_HANDLE_UINT;
            uint32_t        hShader = INVALID_HANDLE_UINT;
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

        struct
        {
            uint32_t         hFont = INVALID_HANDLE_UINT;
            COMPONENT_HANDLE hRectTransform{};

            uint32_t         flags = 0;
            _float           sortZ = 0.f;
            _float4          vColor = { 1.f, 1.f, 1.f, 1.f };
            uint8_t          visualPriority = 0;
            uint8_t          pad1[3] = {};
            _float          fScale = 1.f;
            const std::basic_string<_tchar>* pText = nullptr;
            RECT_F           rcClip = { 0.f, 0.f, 0.f, 0.f };
            _float2         vOffset = {};
            _bool           bCenter = true;
        } text;
        struct
        {
            uint32_t hMaterial;
            uint32_t hTexture;
            COMPONENT_HANDLE hTransform;

            uint32_t hPerObjectParams;

            _uint iFrame;
            _uint iRow;
            _uint iCol;

            _float2 vSize;
            _float4 vColor;

            _bool bBillboard;

            const SPRITE_EFFECT_DATA* pData;
        } sprite;
    };

public:
    static tagDrawCmd Create_Mesh(uint32_t hMesh, uint32_t hMat, COMPONENT_HANDLE hTr, COMPONENT_HANDLE hAt, uint32_t flags, uint32_t first, uint32_t count)
    {
        tagDrawCmd c{};
        c.kind = DRAW_TYPE::MESH;
        c.mesh.hMesh = hMesh;
        c.mesh.hMaterial = hMat;
        c.mesh.hTransform = hTr;
        c.mesh.hAnimator = hAt;
        c.mesh.flags = flags;
        c.mesh.firstIndex = first;
        c.mesh.indexCount = count;
        return c;
    }

    static tagDrawCmd Create_Line(uint32_t hMesh, DRAW_TYPE eType, RENDER_LAYER eLayer, uint32_t hShader)
    {
        tagDrawCmd c{};
        c.kind = DRAW_TYPE::LINE;
        c.line.hMesh = hMesh;
        c.line.hShader = hShader;
        c.eLayer = eLayer;
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
    static tagDrawCmd Create_Text(uint32_t hFont, COMPONENT_HANDLE hRectTr, uint32_t flags,
        float sortZ, const _float4& color, float fScale, uint8_t visualPriority,
        const std::basic_string<_tchar>* pText, const RECT_F& clip, _float2 vOffset, _bool bCenter)
    {
        tagDrawCmd c{};
        c.kind = DRAW_TYPE::TEXT;
        c.text.hFont = hFont;
        c.text.hRectTransform = hRectTr;
        c.text.flags = flags;
        c.text.sortZ = sortZ;
        c.text.vColor = color;
        c.text.fScale = fScale;
        c.text.visualPriority = visualPriority;
        c.text.pText = pText;
        c.text.rcClip = clip;
        c.text.vOffset = vOffset;
        c.text.bCenter = bCenter;
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

typedef struct tagParticleInstanceVertex
{
    _float4 vRight;
    _float4 vUp;
    _float4 vLook;
    _float4 vTranslation;
    _float2 vLifeTime;
} VTXPARTICLE_INSTANCE;

NS_END
