#pragma once
#include "Engine_Define.h"
#include "Identity.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagMaterialEntry final
{
    ASSET_GUID  tGUID{};

    uint32_t hShader = 0;
    uint16_t passIndex = 0;

    ID3DX11EffectMatrixVariable* pWorld = nullptr;
    ID3DX11EffectMatrixVariable* pView = nullptr;
    ID3DX11EffectMatrixVariable* pProj = nullptr;

    /* UI / Sprite params (optional but recommended) */
    ID3DX11EffectShaderResourceVariable* pMainTex = nullptr;   // diffuse/albedo/main tex
    ID3DX11EffectVectorVariable* pColor = nullptr;      // float4
    ID3DX11EffectVectorVariable* pUV = nullptr;         // float4 (u0,v0,u1,v1) or (offset,scale)
    ID3DX11EffectVectorVariable* pClip = nullptr;       // float4 (x0,y0,x1,y1) in screen/clip space

public :
    _bool Is_Valid() const {
        return (hShader != 0) && (pWorld != nullptr) && (pView != nullptr) && (pProj != nullptr);
    }
} MATERIAL_ENTRY;

NS_END
