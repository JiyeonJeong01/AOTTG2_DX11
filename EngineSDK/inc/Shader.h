#pragma once
#include "Engine_Define.h"
#include "Identity.h"
#include "Logger.h"

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
    std::unordered_map<std::string, ID3DX11EffectVariable*> varCache;

    VERTEX_DECL eDecl = VERTEX_DECL::VTXTEX;

public:
    _bool Is_Valid() const noexcept
    {
        return pEffect != nullptr && pTech != nullptr && !pPasses.empty();
    }

    ID3DX11EffectVariable* Get_VarCached(const char* name)
    {
        if (!pEffect || !name) return nullptr;

        auto it = varCache.find(name);
        if (it != varCache.end())
            return it->second;

        ID3DX11EffectVariable* v = pEffect->GetVariableByName(name);
        cout << "Get_VarCached try = [" << name << "]" << endl;

        if (!v)
        {
            cout << "GetVariableByName returned null = [" << name << "]" << endl;
        }
        else if (!v->IsValid())
        {
            cout << "GetVariableByName invalid = [" << name << "]" << endl;
        }

        if (!v || !v->IsValid())
            v = nullptr;

        varCache.emplace(name, v);
        return v;
    }

} SHADER_ENTRY;

NS_END
