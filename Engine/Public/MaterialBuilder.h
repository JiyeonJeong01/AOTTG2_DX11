#pragma once
#include "Engine_Macro.h"
#include "Material.h"
#include "Shader.h"

NS_BEGIN(Engine)

class ENGINE_DLL CMaterialBuilder final
{
public:
    static HRESULT Create(const SHADER_ENTRY& shader, uint16_t passIndex, MATERIAL_ENTRY& outMaterial);
};

NS_END
