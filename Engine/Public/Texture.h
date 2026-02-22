#pragma once
#include "Engine_Define.h"

typedef struct ENGINE_DLL tagTextureEntry final
{
    ASSET_GUID  tGUID{};

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;

    _bool Is_Valid() const noexcept
    {
        return pSRV != nullptr;
    }
    ID3D11ShaderResourceView* SRV() const noexcept
    {
        return pSRV.Get();
    }
} TEXTURE_ENTRY;
