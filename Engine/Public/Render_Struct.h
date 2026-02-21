#pragma once
#include "Engine_Define.h"
#include "Component_Struct.h"

NS_BEGIN(Engine)

typedef struct tagVertexPositionColor
{
    XMFLOAT3			vPosition;
    XMFLOAT4			vColor;
}VTXCOL;

typedef struct tagVertexPositionTexcoord
{
    XMFLOAT3			vPosition;
    XMFLOAT2			vTexcoord;
}VTXTEX;

static constexpr D3D11_INPUT_ELEMENT_DESC VTXCOL_LAYOUT[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

static constexpr D3D11_INPUT_ELEMENT_DESC VTXTEX_LAYOUT[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

typedef struct ENGINE_DLL tagILDesc
{
    const D3D11_INPUT_ELEMENT_DESC* pDesc;
    uint32_t                        iCount;
}IL_DESC;

/* Blueprints for gpu to interpret memory chuncks  */
static constexpr IL_DESC g_IL_TABLE[] = {
    { VTXCOL_LAYOUT, _countof(VTXCOL_LAYOUT) },
    { VTXTEX_LAYOUT, _countof(VTXTEX_LAYOUT) }
};

typedef struct ENGINE_DLL tagDrawCmd final
{
    uint64_t sortKey = 0;

    DRAW_TYPE kind = DRAW_TYPE::MESH;
    uint8_t   pad0[7] = {}; // 8바이트 정렬(선택)

    union
    {
        struct
        {
            uint32_t        hMesh = 0;
            uint32_t        hMaterial = 0;
            uint32_t        hMainTexture = 0;
            COMPONENT_HANDLE hTransform{};

            uint32_t        flags = RF_NONE;
            uint32_t        firstIndex = 0;
            uint32_t        indexCount = 0;
        } mesh;

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
    static tagDrawCmd Create_Mesh(uint32_t hMesh, uint32_t hMat, uint32_t hTex, COMPONENT_HANDLE hTr, uint32_t flags, uint32_t first, uint32_t count)
    {
        tagDrawCmd c{};
        c.kind = DRAW_TYPE::MESH;
        c.mesh.hMesh = hMesh;
        c.mesh.hMaterial = hMat;
        c.mesh.hMainTexture = hTex;
        c.mesh.hTransform = hTr;
        c.mesh.flags = flags;
        c.mesh.firstIndex = first;
        c.mesh.indexCount = count;
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

NS_END
