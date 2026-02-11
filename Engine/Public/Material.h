#pragma once
#include "Engine_Define.h"
#include <wrl/client.h>

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagMaterialEntry final
{
    Microsoft::WRL::ComPtr<ID3D11VertexShader>       pVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>        pPS;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>        pIL;

    Microsoft::WRL::ComPtr<ID3D11Buffer>             cbPerObject; // optional

    Microsoft::WRL::ComPtr<ID3D11RasterizerState>    pRS; // optional
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState>  pDS; // optional
    Microsoft::WRL::ComPtr<ID3D11BlendState>         pBS; // optional

public:
    _bool Is_Valid() const noexcept
    {
        return (pVS != nullptr) && (pPS != nullptr) && (pIL != nullptr);
    }

    void Bind(ID3D11DeviceContext* pContext) const
    {
        if (!pContext)
            return;

        if (!pVS || !pPS || !pIL)
            return;

        pContext->IASetInputLayout(pIL.Get());
        pContext->VSSetShader(pVS.Get(), nullptr, 0);
        pContext->PSSetShader(pPS.Get(), nullptr, 0);

        if (cbPerObject)
        {
            ID3D11Buffer* cbs[] = { cbPerObject.Get() };
            pContext->VSSetConstantBuffers(0, 1, cbs);
            pContext->PSSetConstantBuffers(0, 1, cbs);
        }

        if (pRS)
            pContext->RSSetState(pRS.Get());

        if (pDS)
            pContext->OMSetDepthStencilState(pDS.Get(), 0);

        if (pBS)
        {
            const _float f[4] = { 0.f,0.f,0.f,0.f };
            pContext->OMSetBlendState(pBS.Get(), f, 0xFFFFFFFF);
        }
    }

} MATERIAL_ENTRY;

NS_END
