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

typedef struct ENGINE_DLL  tagDrawCmd final
{
    uint64_t sortKey = 0;

    uint32_t     hMesh = 0;
    uint32_t hMaterial = 0;
    COMPONENT_HANDLE hTransform{};

    uint32_t flags = RF_NONE;

    uint32_t firstIndex = 0;
    uint32_t indexCount = 0;
}DRAW_CMD;

/* Blueprints for gpu to interpret memory chuncks  */
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

NS_END
