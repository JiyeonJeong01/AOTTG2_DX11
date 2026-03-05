#pragma once
#include "BuiltIn_GUID.h"
#include "Engine_Define.h"
#include "Identity.h"
#include "Render_Struct.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagMaterialEntry final
{
    ASSET_GUID  tGUID{};

    ASSET_GUID  shaderGUID = DEFAULT_ASSET_GUID::SHADER_VTXTEX;
    uint32_t hShader = 0;
    uint16_t passIndex = 0;

    ID3DX11EffectMatrixVariable* pWorld = nullptr;
    ID3DX11EffectMatrixVariable* pView = nullptr;
    ID3DX11EffectMatrixVariable* pProj = nullptr;

    ID3DX11EffectShaderResourceVariable* pMainTex = nullptr;   // diffuse/albedo/main tex
    ID3DX11EffectVectorVariable* pColor = nullptr;      // float4
    ID3DX11EffectVectorVariable* pUV = nullptr;         // float4 (u0,v0,u1,v1) or (offset,scale)
    ID3DX11EffectVectorVariable* pClip = nullptr;       // float4 (x0,y0,x1,y1) in screen/clip space

    /* ------ Standard material properties ------ */
    _float4     baseColor = { 1.f, 1.f, 1.f, 1.f };          // Albedo tint
    uint32_t    hBaseMap = INVALID_HANDLE_UINT;
    ASSET_GUID  baseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;

    /* Generic param block */
    NAME_VALUE_PARAM_BLOCK materialParams;

public :
    _bool Is_Valid() const {
        return (hShader != 0) && (pWorld != nullptr) && (pView != nullptr) && (pProj != nullptr);
    }

    void Sync_StandardParams()
    {
        materialParams.Set_Float4("g_BaseColor", baseColor);
        materialParams.Set_Texture("g_BaseMap", hBaseMap);
    }
} MATERIAL_ENTRY;

NS_END
