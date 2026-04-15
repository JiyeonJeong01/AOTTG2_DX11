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

    MATERIAL_RENDER_TYPE eRenderType = MATERIAL_RENDER_TYPE::DEFAULT;

    ID3DX11EffectShaderResourceVariable* pBaseMap = nullptr;        // base/diffuse/albedo
    ID3DX11EffectShaderResourceVariable* pNormalMap = nullptr;      // normal

    ID3DX11EffectVectorVariable* pBaseColor = nullptr;              // float4
    ID3DX11EffectScalarVariable* pShininess = nullptr;              // float4

    ID3DX11EffectMatrixVariable* pBoneMatrices = nullptr;

    /* ------ Standard material properties ------ */
    _float4     baseColor = { 1.f, 1.f, 1.f, 1.f };     // Albedo tint
    _float      fShininess = 32.f;

    uint32_t    hBaseMap = INVALID_HANDLE_UINT;
    ASSET_GUID  baseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;

    uint32_t    hNormalMap = INVALID_HANDLE_UINT;
    ASSET_GUID  normalMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;

    /* Generic param block */
    NAME_VALUE_PARAM_BLOCK materialParams;

public :
    _bool Is_Valid() const {
        return (hShader != 0) && (pWorld != nullptr) && (pView != nullptr) && (pProj != nullptr);
    }
} MATERIAL_ENTRY;

NS_END
