#pragma once
#include "Render_Struct.h"
#include "SpriteEffect.h"
#include "Component_Processor_Impl.h"

NS_BEGIN(Engine)

class CTransform_Processor;

class ENGINE_DLL CSpriteEffect_Processor final
    : public CComponent_Processor_Impl<CSpriteEffect, COMPONENT_TYPE::SPRITE_EFFECT>
{
    DEF_PROCESSOR_ID(PROCESSOR_ID::SPRITE_EFFECT)

public:
    CSpriteEffect_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    ~CSpriteEffect_Processor() override;

public:
    HRESULT Initialize() override;
    void    Update(_float fDT) override;
    void    LateUpdate(_float fDT) override;
    void    Build_RenderQueue(vector<DRAW_CMD>& outCmds);

    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

private:
    void     Initialize_Component_Data(COMPONENT_HANDLE hComponent) override;
    uint64_t Make_SortKey(const SPRITE_EFFECT_DATA& tData) const;

private:
    ID3D11Device* m_pDevice{};
    ID3D11DeviceContext* m_pContext{};
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsScissor;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsNoScissor;

    CTransform_Processor* m_pTransform_Processor{};

    uint32_t  m_hUIRectMesh = INVALID_HANDLE_UINT;
    _float4x4 m_matView{};
    _float4x4 m_matProj{};

public:
    static std::unique_ptr<CSpriteEffect_Processor> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
};

NS_END
