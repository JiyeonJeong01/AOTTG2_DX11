#pragma once
#include "Engine_Macro.h"
#include "Material.h"
#include "Shader.h"

NS_BEGIN(Engine)

class ENGINE_DLL CMaterialBuilder final
{
public:
    static HRESULT Create(const SHADER_ENTRY& shader, uint16_t passIndex, MATERIAL_ENTRY& outMaterial);
    static HRESULT Save_Material(const MATERIAL_ENTRY& mat, const std::filesystem::path& filePath);
    static HRESULT Load_MaterialDesc(const std::filesystem::path& filePath, MATERIAL_ENTRY& outDesc);

};

NS_END
