#include "ShaderBuilder.h"
#include "Engine_Log.h"
#include "Render_Struct.h"

HRESULT CShaderBuilder::Load_FX(ID3D11Device* pDevice, const _tchar* pFxPath, VERTEX_DECL eDecl, SHADER_ENTRY& outEntry)
{
    if (!pDevice || !pFxPath)
        return E_FAIL;

    outEntry = SHADER_ENTRY{};
    outEntry.eDecl = eDecl;

    _uint iFlags = 0;

#ifdef _DEBUG
    iFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    iFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
#endif

    Microsoft::WRL::ComPtr<ID3DX11Effect> pFX;
    HRESULT hr = D3DX11CompileEffectFromFile(pFxPath, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, iFlags, 0, pDevice, pFX.GetAddressOf(), nullptr);
    IF_FAIL_RETURN_MSG_BREAK(hr, E_FAIL, "D3DX11CompileEffectFromFile faild");

    ID3DX11EffectTechnique* pTech = pFX->GetTechniqueByIndex(0);
    IF_NULL_RETURN_MSG_BREAK(pTech, E_FAIL, "GetTechniqueByIndex(%d) failed : Technique is nullptr", 0);

    D3DX11_TECHNIQUE_DESC tTD{};
    pTech->GetDesc(&tTD);
    IF_TRUE_RETURN_MSG_BREAK(tTD.Passes == 0, E_FAIL, "Technique has none pass");

    outEntry.pEffect = pFX;
    outEntry.pTech = pTech;
    outEntry.pPasses.resize(tTD.Passes);

    const D3D11_INPUT_ELEMENT_DESC* pElems = nullptr;
    UINT iElemCnt = 0;

    pElems = g_IL_TABLE[SCAST(_uint, eDecl)].pDesc;
    iElemCnt = g_IL_TABLE[SCAST(_uint, eDecl)].iCount;

    for (UINT i = 0; i < tTD.Passes; ++i)
    {
        ID3DX11EffectPass* pPass = pTech->GetPassByIndex(i);
        IF_NULL_RETURN_MSG_BREAK(pPass, E_FAIL, "%d effect pass is nullptr", i);

        D3DX11_PASS_DESC tPD{};
        pPass->GetDesc(&tPD);

        Microsoft::WRL::ComPtr<ID3D11InputLayout> pIL;
        HRESULT hr = pDevice->CreateInputLayout(pElems, iElemCnt, tPD.pIAInputSignature, tPD.IAInputSignatureSize, pIL.GetAddressOf());
        IF_FAIL_RETURN_MSG_BREAK(hr, E_FAIL, "CreateInputLayeout failed. pass=%u decl=%u", i, (uint32_t)eDecl);

        outEntry.pPasses[i].pPass = pPass;
        outEntry.pPasses[i].pInputLayout = pIL;
    }
    return S_OK;
}
