#pragma once
#include "Engine_Define.h"
#include <wrl/client.h>

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagMaterialEntry final
{
    uint32_t hShader = 0;
    uint16_t passIndex = 0;

    ID3DX11EffectMatrixVariable* pWorld = nullptr;
    ID3DX11EffectMatrixVariable* pView = nullptr;
    ID3DX11EffectMatrixVariable* pProj = nullptr;
public :
    _bool Is_Valid() const {
        return (hShader != 0) && (pWorld != nullptr) && (pView != nullptr) && (pProj != nullptr);
    }
} MATERIAL_ENTRY;

NS_END
