#include "MaterialBuilder.h"
#include "Engine_Log.h"

NS_BEGIN(Engine)

HRESULT CMaterialBuilder::Create(const SHADER_ENTRY& shader, uint16_t passIndex, MATERIAL_ENTRY& outMaterial)
{
    if (!shader.Is_Valid())
        return E_FAIL;

    if (passIndex >= shader.pPasses.size())
        return E_FAIL;

    outMaterial = MATERIAL_ENTRY{};
    outMaterial.hShader = 0; /* Storage should set hShader */
    outMaterial.passIndex = passIndex;

    ID3DX11Effect* fx = shader.pEffect.Get();
    if (!fx)
        return E_FAIL;

    /* Caching variables */
    outMaterial.pWorld = fx->GetVariableByName("g_WorldMatrix")->AsMatrix();
    outMaterial.pView = fx->GetVariableByName("g_ViewMatrix")->AsMatrix();
    outMaterial.pProj = fx->GetVariableByName("g_ProjMatrix")->AsMatrix();

    IF_TRUE_RETURN_MSG_BREAK(!outMaterial.pWorld, E_FAIL, "g_WorldMatrix not found");
    IF_TRUE_RETURN_MSG_BREAK(!outMaterial.pView, E_FAIL, "g_ViewMatrix not found");
    IF_TRUE_RETURN_MSG_BREAK(!outMaterial.pProj, E_FAIL, "g_ProjMatrix not found");

    return S_OK;
}

NS_END
