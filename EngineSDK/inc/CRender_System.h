// Render_System.h

#pragma once
#include "Engine_Define.h"
#include "Render_Struct.h"
#include "Shader.h"

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
    _bool       Submit_Camera(_fmatrix matView, _fmatrix matProj);
    void        Submit_LineMesh(const DRAW_CMD& cmd);

    const UI_GLOBAL&    Get_UI_Global();
    void                Set_UI_Global(const UI_GLOBAL& tUI);

    static void         Apply_Block_To_Shader(SHADER_ENTRY* pShader, const NAME_VALUE_PARAM_BLOCK& blk);

public:
    const std::vector<DRAW_CMD>& Get_AllDrawCmds() const
    {
        return m_AllDrawCmds;
    }

private:
    /* COM 객체*/
    ID3D11Device*                                   m_pDevice{};
    ID3D11DeviceContext*                            m_pContext{};

    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rsScissor;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState>   m_rsNoScissor;
    std::unique_ptr<DirectX::SpriteBatch>           m_pSpriteBatch;

    ID3D11BlendState*                               m_pBlendState_None = nullptr;
    ID3D11BlendState*                               m_pBlendState_Alpha = nullptr;

    ID3D11DepthStencilState*                        m_pDepthState_Default = nullptr;
    ID3D11DepthStencilState*                        m_pDepthState_ReadOnly = nullptr;
    ID3D11DepthStencilState*                        m_pDepthState_Disabled = nullptr;

    ID3D11RasterizerState* m_pRasterizerState_Default = nullptr;
    ID3D11RasterizerState* m_pRasterizerState_CullCw = nullptr;

    /* Render Context */
    std::unique_ptr<CRender_Context>        m_upRenderContext{};
    /* Render Context 캐싱 */
    _float4x4               m_matView{};
    _float4x4               m_matProj{};
    UI_GLOBAL               m_gUI{};

    uint32_t                m_hUIRectMesh{};
    uint32_t                m_hDefaultBaseMap{};
    uint32_t                m_hDefaultNormalMap{};
    uint32_t                m_hVtxColShader{};
    uint32_t                m_hVtxParticlePoint{};

    /* Draw Calls */
    vector<DRAW_CMD>        m_AllDrawCmds;
    vector<DRAW_CMD>        m_PendingDrawCmds;
    std::array<std::vector<DRAW_CMD*>, SCAST(size_t, RENDER_LAYER::END)> m_LayerCmds;

    class CTransform_Processor*     m_pTransform_Processor{};
    class CRectTransform_Processor* m_pRectTransform_Processor{};
    class CAnimator_Processor*      m_pAnimator_Processor{};
    class CMeshRenderer_Processor*  m_pMeshRenderer_Processor{};

    _bool   bSubmittedThisFrame{}; /* 프레임당 하나의 카메라의 submit만 받는다. */
    RENDER_LAYER    m_eCurLayer = RENDER_LAYER::END;

private:
    HRESULT    Create_RenderState();

    void     Build_RenderQueue();

    void     Execute_RenderQueue();
    void     Execute_Pass(RENDER_LAYER layer);
    void     Execute_Draw(const DRAW_CMD& cmd);
    void     Execute_Draw_Mesh(const DRAW_CMD& cmd);
    void     Execute_Draw_Canvas(const DRAW_CMD& tCmd);
    void     Execute_Draw_Line(const DRAW_CMD& tCmd);
    void     Execute_Draw_Text(const DRAW_CMD& tCmd);
    void     Execute_Draw_Particle(const DRAW_CMD& tCmd);

    void     Execute_Draw_Mesh_Inner(uint32_t hMesh, uint32_t hMaterial, COMPONENT_HANDLE hComponent, COMPONENT_HANDLE hAnimator, uint32_t hPerObjectParams,
        uint32_t iFirstIdx, uint32_t iNumIdx, const std::vector<_float4x4>* pSkinningMatrices, const _float4x4& matAttach, MESH_MODE eMode);


    void    Apply_Pass_State_Skybox();
    void    Apply_Pass_State_Priority();
    void    Apply_Pass_State_NonBlend();
    void    Apply_Pass_State_Blend();
    void    Apply_Pass_State_UI();

    void    Bind_BlendState_None();
    void    Bind_BlendState_Alpha();

    void    Bind_DepthState_Default();
    void    Bind_DepthState_ReadOnly();
    void    Bind_DepthState_Disabled();

    void    Bind_RasterizerState_Default();
    void    Bind_RasterizerState_CullCw();

};


NS_END
