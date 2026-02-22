#pragma once
#include "Engine_Define.h"
#include "Identity.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagShaderEntry final
{
    ASSET_GUID  tGUID{};

    Microsoft::WRL::ComPtr<ID3DX11Effect> pEffect{}; /* .fx file  */
    ID3DX11EffectTechnique* pTech = nullptr;

    struct PASS_CACHE
    {
        ID3DX11EffectPass* pPass = nullptr;
        Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout{ };
    };

    std::vector<PASS_CACHE> pPasses;

    VERTEX_DECL eDecl = VERTEX_DECL::VTXTEX;

public:
    _bool Is_Valid() const noexcept
    {
        return pEffect != nullptr && pTech != nullptr && !pPasses.empty();
    }

} SHADER_ENTRY;

NS_END
