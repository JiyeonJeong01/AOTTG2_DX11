#pragma once

#include "Render_Struct.h"
#include "CanvasRenderer.h"
#include "Component_Processor_Impl.h"

NS_BEGIN(Engine)

class CRectTransform_Processor;

class ENGINE_DLL CCanvasRenderer_Processor final : public CComponent_Processor_Impl<CCanvasRenderer, COMPONENT_TYPE::CANVAS_RENDERER>
{
public:
    CCanvasRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CRectTransform_Processor* pRTProcessor);
    ~CCanvasRenderer_Processor() override;

public:
    HRESULT Initialize(_uint iWidth, _uint iHeight);
    void    Update(_float fDT) override;
    void    LateUpdate(_float fDT) override;
    void    Build_RenderQueue(vector<DRAW_CMD>& outCmds);

    HRESULT Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec) override;
    std::unique_ptr<COMPONENT_SPEC_BASE> Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) override;

private:
    void    Initialize_Component_Data(COMPONENT_HANDLE hComponent) override;
    uint64_t Make_SortKey(const CANVAS_RENDERER_DATA& tData) const;

private:
    ID3D11Device* m_pDevice{};
    ID3D11DeviceContext* m_pContext{};
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsScissor;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsNoScissor;

    CRectTransform_Processor* m_pRectTransform_Processor{};

    uint32_t m_hUIRectMesh = INVALID_HANDLE_UINT;
    _float4x4 m_matView{};
    _float4x4 m_matProj{};

public:
    static std::unique_ptr<CCanvasRenderer_Processor> Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CRectTransform_Processor* pProcessor,_uint iWidth, _uint iHeight);
};

NS_END
