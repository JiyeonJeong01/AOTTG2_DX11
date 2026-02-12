#pragma once

#include "Shader.h"

NS_BEGIN(Engine)

class ENGINE_DLL CShaderBuilder final
{
public :
    static HRESULT Load_FX(ID3D11Device* pDevice, const _tchar* pFxPath, VERTEX_DECL eDecl, SHADER_ENTRY& outEntry);
};

NS_END
