#pragma once

#include "CComponent_Proxy_Base.h"

NS_BEGIN(Engine)

typedef struct ENGINE_DLL tagShaderData final
{
    
}SHADER_DATA;

class CShader_Processor;

class ENGINE_DLL CShader final : public CComponent_Proxy_Base<SHADER_DATA, CShader>
{
public:
    using ProcessorType = CShader_Processor;
    using DataType = SHADER_DATA;

public:
    CShader() : CComponent_Proxy_Base() { m_eComType = COMPONENT_TYPE::SHADER; }
    CShader(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {
        m_eComType = COMPONENT_TYPE::SHADER;
    }
    ~CShader() override = default;
public:
    ID3DX11EffectTechnique* Get_Technique(uint32_t idx = 0) const;
    ID3DX11EffectPass*      Get_Pass(uint32_t passIdx) const;
    HRESULT                 Apply(uint32_t passIdx);

private:
    ID3DX11Effect*          m_pEffect = { nullptr };
    _uint		            m_iNumPasses = {};
};

NS_END
