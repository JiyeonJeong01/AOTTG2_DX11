// Render_System.h

#pragma once
#include "Engine_Define.h"
#include "Render_Struct.h"

NS_BEGIN(Engine)

class CRender_Context;

class ENGINE_DLL CRender_System final
{
    DECLARE_SINGLETON(CRender_System)

public:
    HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWidth, _uint iHeight);
    void    Priority_Update();
    void    Render();

    CRender_Context* Contexts();
    _bool   Submit_Camera(_fmatrix matView, _fmatrix matProj);

    const UI_GLOBAL&    Get_UI_Global();
    void                Set_UI_Global(const UI_GLOBAL& tUI);

public:
    const std::vector<DRAW_CMD>& Get_AllDrawCmds() const { return m_AllDrawCmds; }

private:
    /* COM 객체*/
    ID3D11Device* m_pDevice{};
    ID3D11DeviceContext* m_pContext{};
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsScissor;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsNoScissor;

    /* Render Context */
    std::unique_ptr<CRender_Context>        m_upRenderContext{};
    /* Render Context 캐싱 */
    _float4x4               m_matView{};
    _float4x4               m_matProj{};
    UI_GLOBAL               m_gUI{};

    uint32_t                m_hUIRectMesh{};

    /* Draw Calls */
    vector<DRAW_CMD>        m_AllDrawCmds;
    std::array<std::vector<DRAW_CMD*>, SCAST(size_t, RENDER_LAYER::END)> m_LayerCmds;

    class CTransform_Processor* m_pTransform_Processor{};
    class CRectTransform_Processor* m_pRectTransform_Processor{};

    _bool   bSubmittedThisFrame{};

private:
    void     Build_RenderQueue();
    void     Execute_RenderQueue();
    void     Execute_Pass(RENDER_LAYER layer);
    void     Execute_Draw(const DRAW_CMD& cmd);
    void     Execute_Draw_Mesh(const DRAW_CMD& cmd);
    void     Execute_Draw_Canvas(const DRAW_CMD& tCmd);
};

NS_END
