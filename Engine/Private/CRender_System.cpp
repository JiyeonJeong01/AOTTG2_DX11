#pragma region HEADER
// Render_System.cpp
#include "CRender_System.h"

#include "Core_System.h"
#include "Component_System.h"
#include "Engine_Math.h"
#include "Resource_System.h"
#include "Logger.h"

// proxies
#include "Transform_Processor.h"
#include "RectTransform_Processor.h"
#include "Animator_Processor.h"
#include "MeshRenderer_Processor.h"

// resource types
#include "BuiltIn_GUID.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "Render_Context.h"
#include "Texture.h"
#include "Animator.h"
#include "Font.h"
#include "UIText.h"
#pragma endregion

IMPLEMENT_SINGLETON(CRender_System)

CRender_System::CRender_System() = default;
CRender_System::~CRender_System() = default;

static inline int LAYER_TO_IDX(RENDER_LAYER e)
{
    return SCAST(int, e);
}


HRESULT CRender_System::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWidth, _uint iHeight)
{
    m_pDevice = pDevice;
    m_pContext = pContext;

    IF_NULL_RETURN_MSG_BREAK(m_pDevice, E_FAIL, "RenderSystem device is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_pContext, E_FAIL, "RenderSystem context is nullptr");

    Create_RenderState();

    /* 머테리얼 기본 텍스쳐 핸들 준비 */
    {
        m_hDefaultBaseMap = SYS_RESOURCE.Load_Texture(DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT);
        IF_TRUE_RETURN_MSG_BREAK(m_hDefaultBaseMap == INVALID_HANDLE_UINT, E_FAIL, "DefaultBaseMap load failed");
        m_hUIRectMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT);
        IF_TRUE_RETURN_MSG_BREAK(m_hUIRectMesh == INVALID_HANDLE_UINT, E_FAIL, "UI rect mesh load failed");
        m_hDefaultNormalMap = SYS_RESOURCE.Load_Texture(DEFAULT_ASSET_GUID::TEXTURE_NORMALMAP_DEFAULT);
        IF_TRUE_RETURN_MSG_BREAK(m_hDefaultNormalMap == INVALID_HANDLE_UINT, E_FAIL, "DefaultNormalMap load failed");
        m_hDeferredShader = SYS_RESOURCE.Load_Shader(DEFAULT_ASSET_GUID::SHADER_DEFERRED);
        IF_TRUE_RETURN_MSG_BREAK(m_hDeferredShader == INVALID_HANDLE_UINT, E_FAIL, "DeferredShader load failed");
        m_hSpeedLineShader = SYS_RESOURCE.Load_Shader(DEFAULT_ASSET_GUID::SHADER_SPEEDLINE);
        IF_TRUE_RETURN_MSG_BREAK(m_hSpeedLineShader == INVALID_HANDLE_UINT, E_FAIL, "SpeedLine load failed");
        m_hFogShader = SYS_RESOURCE.Load_Shader(DEFAULT_ASSET_GUID::SHADER_FOG);
        IF_TRUE_RETURN_MSG_BREAK(m_hSpeedLineShader == INVALID_HANDLE_UINT, E_FAIL, "SpeedLine load failed");
        m_hVtxParticlePoint = SYS_RESOURCE.Load_Shader(DEFAULT_ASSET_GUID::SHADER_VTXPARTICLEPOINT);
        IF_TRUE_RETURN_MSG_BREAK(m_hVtxParticlePoint == INVALID_HANDLE_UINT, E_FAIL, "ParticlePoint load failed");
    }

    /* 랜더 관련 장치 세팅  */
    {
        D3D11_RASTERIZER_DESC rs{};
        rs.FillMode = D3D11_FILL_SOLID;
        rs.CullMode = D3D11_CULL_NONE;
        rs.DepthClipEnable = TRUE;

        rs.ScissorEnable = TRUE;
        IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rs, m_rsScissor.GetAddressOf()),
            E_FAIL, "CreateRasterizerState(scissor) failed");

        rs.ScissorEnable = FALSE;
        IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rs, m_rsNoScissor.GetAddressOf()),
            E_FAIL, "CreateRasterizerState(no scissor) failed");

        m_pSpriteBatch = std::make_unique<DirectX::SpriteBatch>(m_pContext);
    }

    /* 렌더에 필요한 컴포넌트 프로세서 가져오기 */
    {
        m_pTransform_Processor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
        IF_NULL_RETURN_MSG_BREAK(m_pTransform_Processor, E_FAIL, "Transform processor bind failed");

        m_pRectTransform_Processor = SYS_COMPONENT.Bind_Processor<CRectTransform_Processor>();
        IF_NULL_RETURN_MSG_BREAK(m_pRectTransform_Processor, E_FAIL, "RectTransform processor bind failed");

        m_pAnimator_Processor = SYS_COMPONENT.Bind_Processor<CAnimator_Processor>();
        IF_NULL_RETURN_MSG_BREAK(m_pAnimator_Processor, E_FAIL, "Animator processor bind failed");

        m_pMeshRenderer_Processor = SYS_COMPONENT.Bind_Processor<CMeshRenderer_Processor>();
        IF_NULL_RETURN_MSG_BREAK(m_pMeshRenderer_Processor, E_FAIL, "Animator processor bind failed");
    }

    m_upRenderContext = CRender_Context::Create(iWidth, iHeight);
    return S_OK;
}

void CRender_System::Priority_Update()
{
    bSubmittedThisFrame = false;

    m_upRenderContext->Update();

    m_matView = m_upRenderContext->Get_View();
    m_matProj = m_upRenderContext->Get_Proj();
    m_gUI = m_upRenderContext->Get_UI_Global();
}

HRESULT CRender_System::Create_RenderState()
{
    if (!m_pDevice)
        return E_FAIL;

    HRESULT hr = S_OK;

    /* -------------------------------------------------
       BlendState : None
    ------------------------------------------------- */
    {
        D3D11_BLEND_DESC desc{};
        desc.AlphaToCoverageEnable = FALSE;
        desc.IndependentBlendEnable = FALSE;

        D3D11_RENDER_TARGET_BLEND_DESC& rt = desc.RenderTarget[0];
        rt.BlendEnable = FALSE;                                     // 블렌딩 : off
        rt.SrcBlend = D3D11_BLEND_ONE;                              // 새로 그려질 색상에 곱할 값 : 1
        rt.DestBlend = D3D11_BLEND_ZERO;                            // 이미 그려진 색상에 곱할 값 : 0. 덮는다
        rt.BlendOp = D3D11_BLEND_OP_ADD;                            // 두 색을 합치는 연산 : +
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;                         // 알파 채널에 대한 Src/Dst의 연산 방식
        rt.DestBlendAlpha = D3D11_BLEND_ZERO;                       //  - 알파 값에 곱하여 최종 알파 값이 된다
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;                       // 알파 채널을 합치는 연산 방식 
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;    // RGBA 중 어떤 채널을 기록할지 : 전부 

        hr = m_pDevice->CreateBlendState(&desc, &m_pBlendState_None);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       BlendState : Alpha ( 반투명 )
    ------------------------------------------------- */
    {
        D3D11_BLEND_DESC desc{};
        desc.AlphaToCoverageEnable = FALSE;
        desc.IndependentBlendEnable = FALSE;

        D3D11_RENDER_TARGET_BLEND_DESC& rt = desc.RenderTarget[0];
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D11_BLEND_SRC_ALPHA;                            // FINAL COLOR = (SRC_COL * SRC_A) + (DST_COL * (1 - SRC_A))
        rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = m_pDevice->CreateBlendState(&desc, &m_pBlendState_Alpha);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       BlendState : Add ( 반투명 )
    ------------------------------------------------- */
    {
        D3D11_BLEND_DESC desc{};
        desc.AlphaToCoverageEnable = FALSE;
        desc.IndependentBlendEnable = FALSE;

        D3D11_RENDER_TARGET_BLEND_DESC& rt = desc.RenderTarget[0];
        rt.BlendEnable = TRUE;
        rt.SrcBlend = D3D11_BLEND_ONE;
        rt.DestBlend = D3D11_BLEND_ONE;
        rt.BlendOp = D3D11_BLEND_OP_ADD;
        rt.SrcBlendAlpha = D3D11_BLEND_ONE;
        rt.DestBlendAlpha = D3D11_BLEND_ONE;
        rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
        rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        hr = m_pDevice->CreateBlendState(&desc, &m_pBlendState_Add);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       DepthStencilState : Default
    ------------------------------------------------- */
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = TRUE;                                        // 깊이 테스트 ON.
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;               // 현재 그린 픽셀의 깊이 값을 기록한다.
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;                   // 새 픽셀의 깊이 <= 기존 버퍼의 깊이 일 때만 그린다.
        desc.StencilEnable = FALSE;

        hr = m_pDevice->CreateDepthStencilState(&desc, &m_pDepthState_Default);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       DepthStencilState : ReadOnly
    ------------------------------------------------- */
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = TRUE;                                        // 깊이 테스트 ON
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;              // 깊이 버퍼 내 기록은 하지 않는다 -> 직접 연산 필요
        desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
        desc.StencilEnable = FALSE;

        hr = m_pDevice->CreateDepthStencilState(&desc, &m_pDepthState_ReadOnly);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       DepthStencilState : Disabled
    ------------------------------------------------- */
    {
        D3D11_DEPTH_STENCIL_DESC desc{};
        desc.DepthEnable = FALSE;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        desc.StencilEnable = FALSE;

        hr = m_pDevice->CreateDepthStencilState(&desc, &m_pDepthState_Disabled);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       RasterizerState : Default
    ------------------------------------------------- */
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_BACK;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = FALSE;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;

        hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerState_Default);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       RasterizerState : CullCw
    ------------------------------------------------- */
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_FRONT;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = FALSE;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;

        hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerState_CullCw);
        if (FAILED(hr))
            return hr;
    }

    /* -------------------------------------------------
       RasterizerState : CullNone
    ------------------------------------------------- */
    {
        D3D11_RASTERIZER_DESC desc{};
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = FALSE;
        desc.MultisampleEnable = FALSE;
        desc.AntialiasedLineEnable = FALSE;

        hr = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerState_CullNone);
        if (FAILED(hr))
            return hr;
    }

    return S_OK;
}

void CRender_System::Submit_LineMesh(const DRAW_CMD& cmd)
{
    m_PendingDrawCmds.push_back(cmd);
}

void CRender_System::Submit_SpeedLine(const SPEED_LINE_DESC& tDesc)
{
    m_tPendingSpeedLine = tDesc;
}

void CRender_System::Build_RenderQueue()
{
    for (int i = 0; i < LAYER_TO_IDX(RENDER_LAYER::END); ++i)
        m_LayerCmds[i].clear();

    /* 모든 DRAW_CMD 빌드하기 */
    m_AllDrawCmds.clear();

    m_AllDrawCmds.insert(m_AllDrawCmds.end(), m_PendingDrawCmds.begin(), m_PendingDrawCmds.end());

    SYS_COMPONENT.Build_RenderQueue(m_AllDrawCmds);

    /* 분배 */
    for (auto& cmd : m_AllDrawCmds)
    {
        const int idx = LAYER_TO_IDX(cmd.eLayer);
        if (idx < 0 || idx >= LAYER_TO_IDX(RENDER_LAYER::END))
            continue;

        m_LayerCmds[idx].push_back(&cmd);
    }

    /* 패스별 정렬 */
    {
        auto& q = m_LayerCmds[LAYER_TO_IDX(RENDER_LAYER::BLEND)];
        std::sort(q.begin(), q.end(), [](const DRAW_CMD* a, const DRAW_CMD* b)
            {
                return a->sortKey > b->sortKey;
            });
    }
    {
        auto& q = m_LayerCmds[LAYER_TO_IDX(RENDER_LAYER::UI)];
        std::sort(q.begin(), q.end(), [](const DRAW_CMD* a, const DRAW_CMD* b)
            {
                return a->sortKey < b->sortKey;
            });
    }
}


void CRender_System::Execute_RenderQueue()
{
    Apply_Pass_State_Skybox();
    Execute_Pass(RENDER_LAYER::SKY);

    Apply_Pass_State_Priority();
    Execute_Pass(RENDER_LAYER::PRIORITY);

    Apply_Pass_State_NonBlend();
    Render_GBuffer();

    Apply_Pass_State_Light();
    Render_LightPass();

    Apply_Pass_State_Combined();
    Render_CombinedPass();

    Apply_Pass_State_PostProcess();
    Render_PostProcess();
    Render_PostProcessComposite();

    Apply_Pass_State_Blend();
    Execute_Pass(RENDER_LAYER::BLEND);

    Render_SpeedLinePass();
    Apply_Pass_State_UI();
    Execute_Pass(RENDER_LAYER::UI);

    m_PendingDrawCmds.clear();
}

void CRender_System::Render_GBuffer()
{
    const _float4 vDiffuseClear = { 0.f, 0.f, 0.f, 0.f };
    const _float4 vNormalClear = { 0.5f, 0.5f, 1.f, 1.f };
    const _float4 vDepthClear = { 1.f, 1.f, 1.f, 1.f };

    SYS_CORE.Bind_GBufferRTV();
    SYS_CORE.Clear_Depth_RTV(&vDepthClear);
    SYS_CORE.Clear_GBuffer_Buffers(&vDiffuseClear, &vNormalClear);

    Execute_Pass(RENDER_LAYER::NONBLEND);

    Unbind_PS_SRVs();
}

void CRender_System::Render_LightPass()
{
    _float4 vLightClear = { 0.f, 0.f, 0.f, 1.f };
    _float4 vSpecularClear = { 0.f, 0.f, 0.f, 1.f };

    SYS_CORE.Bind_LightRTV();
    SYS_CORE.Clear_Light_Buffer(&vLightClear);
    SYS_CORE.Clear_Specular_RTV(&vSpecularClear);

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(m_hDeferredShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "Deferred shader is nullptr.");

    const uint16_t passIndex = 0; /* Directional */
    if (passIndex >= pShader->pPasses.size())
        return;

    ID3D11ShaderResourceView* pNormalSRV = nullptr;
    ID3D11ShaderResourceView* pDepthSRV = nullptr;

    SYS_CORE.Share_NormalSRV(&pNormalSRV);
    SYS_CORE.Share_DepthSRV(&pDepthSRV);

    if (!pNormalSRV || !pDepthSRV)
        return;

    auto* pVarWorld = pShader->Get_VarCached("g_WorldMatrix");
    auto* pVarView = pShader->Get_VarCached("g_ViewMatrix");
    auto* pVarProj = pShader->Get_VarCached("g_ProjMatrix");

    auto* pVarViewInv = pShader->Get_VarCached("g_ViewMatrixInverse");
    auto* pVarProjInv = pShader->Get_VarCached("g_ProjMatrixInverse");

    auto* pVarNormalTex = pShader->Get_VarCached("g_NormalTexture");
    auto* pVarDepthTex = pShader->Get_VarCached("g_DepthTexture");

    auto* pVarLightDir = pShader->Get_VarCached("g_vLightDir");
    auto* pVarDiffuseLight = pShader->Get_VarCached("g_vDiffuseLight");
    auto* pVarAmbientLight = pShader->Get_VarCached("g_vAmbientLight");
    auto* pVarSpecularLight = pShader->Get_VarCached("g_vSpecularLight");

    auto* pVarDiffuseMtrl = pShader->Get_VarCached("g_vDiffuseMtrl");
    auto* pVarAmbientMtrl = pShader->Get_VarCached("g_vAmbientMtrl");
    auto* pVarSpecularMtrl = pShader->Get_VarCached("g_vSpecularMtrl");

    auto* pVarCamPosition = pShader->Get_VarCached("g_vCamPosition");

    {
        IF_NULL_RETURN_MSG_BREAK(pVarWorld, , "g_WorldMatrix not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarView, , "g_ViewMatrix not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarProj, , "g_ProjMatrix not found.");

        IF_NULL_RETURN_MSG_BREAK(pVarViewInv, , "g_ViewMatrixInverse not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarProjInv, , "g_ProjMatrixInverse not found.");

        IF_NULL_RETURN_MSG_BREAK(pVarNormalTex, , "g_NormalTexture not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarDepthTex, , "g_DepthTexture not found.");

        IF_NULL_RETURN_MSG_BREAK(pVarLightDir, , "g_vLightDir not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarDiffuseLight, , "g_vDiffuseLight not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarAmbientLight, , "g_vAmbientLight not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarSpecularLight, , "g_vSpecularLight not found.");

        IF_NULL_RETURN_MSG_BREAK(pVarDiffuseMtrl, , "g_vDiffuseMtrl not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarAmbientMtrl, , "g_vAmbientMtrl not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarSpecularMtrl, , "g_vSpecularMtrl not found.");

        IF_NULL_RETURN_MSG_BREAK(pVarCamPosition, , "g_vCamPosition not found.");

    }

    const UI_VIEWPORT_RECT& vp = m_upRenderContext->Get_UI_Global().tSceneView;

    _float4x4 matWorld, matView, matProj;
    XMStoreFloat4x4(&matWorld, XMMatrixScaling((_float)vp.vSize.x, (_float)vp.vSize.y, 1.f));
    XMStoreFloat4x4(&matView, XMMatrixIdentity());
    XMStoreFloat4x4(&matProj, XMMatrixOrthographicLH((_float)vp.vSize.x, (_float)vp.vSize.y, 0.f, 1.f));

    const _float4x4& matViewInv = m_upRenderContext->Get_ViewInv();
    const _float4x4& matProjInv = m_upRenderContext->Get_ProjInv();

    const _float3& vCamPos3 = m_upRenderContext->Get_CamPosition();
    _float4 vCamPosition = { vCamPos3.x, vCamPos3.y, vCamPos3.z, 1.f };

    _float4 vLightDir = { -1.f, -1.f, 1.f, 0.f };
    _float4 vDiffuseLight = { 1.f, 1.f, 1.f, 1.f };
    _float4 vAmbientLight = { 0.5f, 0.5f, 0.5f, 1.f };
    _float4 vSpecularLight = { 0.2f, 0.2f, 0.2f, 1.f };

    _float4 vDiffuseMtrl = { 1.f, 1.f, 1.f, 1.f };
    _float4 vAmbientMtrl = { 1.f, 1.f, 1.f, 1.f };
    _float4 vSpecularMtrl = { 0.3f, 0.3f, 0.3f, 1.f };

    pVarWorld->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pVarView->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matView));
    pVarProj->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matProj));

    pVarViewInv->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matViewInv));
    pVarProjInv->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matProjInv));

    pVarNormalTex->AsShaderResource()->SetResource(pNormalSRV);
    pVarDepthTex->AsShaderResource()->SetResource(pDepthSRV);

    pVarLightDir->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vLightDir));
    pVarDiffuseLight->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vDiffuseLight));
    pVarAmbientLight->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vAmbientLight));
    pVarSpecularLight->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vSpecularLight));

    pVarDiffuseMtrl->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vDiffuseMtrl));
    pVarAmbientMtrl->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vAmbientMtrl));
    pVarSpecularMtrl->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vSpecularMtrl));

    pVarCamPosition->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vCamPosition));

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    IF_NULL_RETURN_MSG_BREAK(pPass, , "Deferred directional pass is nullptr.");

    pPass->Apply(0, m_pContext);

    const MESH_ENTRY* pRectMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pRectMesh, , "Screen rect mesh is nullptr.");

    pRectMesh->Bind_IA(m_pContext);
    pRectMesh->Draw(m_pContext);

    Unbind_PS_SRVs();
}

void CRender_System::Render_CombinedPass()
{
    SYS_CORE.Bind_SceneRTV_WithoutDSV();

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(m_hDeferredShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "Deferred shader is nullptr.");

    const uint16_t passIndex = 2; /* Combined */
    if (passIndex >= pShader->pPasses.size())
        return;

    ID3D11ShaderResourceView* pDiffuseSRV = nullptr;
    ID3D11ShaderResourceView* pLightSRV = nullptr;
    ID3D11ShaderResourceView* pSpecularSRV = nullptr;

    SYS_CORE.Share_DiffuseSRV(&pDiffuseSRV);
    SYS_CORE.Share_LightSRV(&pLightSRV);
    SYS_CORE.Share_SpecularSRV(&pSpecularSRV);

    if (!pDiffuseSRV) return;
    if (!pLightSRV) return;
    if (!pSpecularSRV) return;

    auto* pVarWorld = pShader->Get_VarCached("g_WorldMatrix");
    auto* pVarView = pShader->Get_VarCached("g_ViewMatrix");
    auto* pVarProj = pShader->Get_VarCached("g_ProjMatrix");
    auto* pVarDiffuseTex = pShader->Get_VarCached("g_DiffuseTexture");
    auto* pVarShadeTex = pShader->Get_VarCached("g_ShadeTexture");
    auto* pVarSpecularTex = pShader->Get_VarCached("g_SpecularTexture");

    IF_NULL_RETURN_MSG_BREAK(pVarWorld, , "g_WorldMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarView, , "g_ViewMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarProj, , "g_ProjMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarDiffuseTex, , "g_DiffuseTexture not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarShadeTex, , "g_ShadeTexture not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarSpecularTex, , "g_SpecularTexture not found.");

    const UI_VIEWPORT_RECT& vp = m_upRenderContext->Get_UI_Global().tSceneView;

    _float4x4 matWorld, matView, matProj;
    XMStoreFloat4x4(&matWorld, XMMatrixScaling((_float)vp.vSize.x, (_float)vp.vSize.y, 1.f));
    XMStoreFloat4x4(&matView, XMMatrixIdentity());
    XMStoreFloat4x4(&matProj, XMMatrixOrthographicLH((_float)vp.vSize.x, (_float)vp.vSize.y, 0.f, 1.f));

    pVarWorld->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pVarView->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matView));
    pVarProj->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matProj));
    pVarDiffuseTex->AsShaderResource()->SetResource(pDiffuseSRV);
    pVarShadeTex->AsShaderResource()->SetResource(pLightSRV);
    pVarSpecularTex->AsShaderResource()->SetResource(pSpecularSRV);

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    IF_NULL_RETURN_MSG_BREAK(pPass, , "Deferred combined pass is nullptr.");

    pPass->Apply(0, m_pContext);

    const MESH_ENTRY* pRectMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pRectMesh, , "Screen rect mesh is nullptr.");

    pRectMesh->Bind_IA(m_pContext);
    pRectMesh->Draw(m_pContext);

    Unbind_PS_SRVs();
}

void CRender_System::Render_SpeedLinePass()
{
    if (false == m_tPendingSpeedLine.bEnable)
        return;

    Bind_BlendState_Alpha();
    Bind_DepthState_Disabled();
    Bind_RasterizerState_Default();

    SYS_CORE.Bind_SceneRTV_WithoutDSV();

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(m_hSpeedLineShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "SpeedLine shader is nullptr.");

    const uint16_t passIndex = 0; /* Combined */
    if (passIndex >= pShader->pPasses.size())
        return;

    auto* pVarWorld = pShader->Get_VarCached("g_WorldMatrix");
    auto* pVarView = pShader->Get_VarCached("g_ViewMatrix");
    auto* pVarProj = pShader->Get_VarCached("g_ProjMatrix");

    auto* pVelocityDir = pShader->Get_VarCached("g_vVelocityDir");
    auto* pIntensity = pShader->Get_VarCached("g_fIntensity");

    auto* pTime = pShader->Get_VarCached("g_fTime");

    {
        IF_NULL_RETURN_MSG_BREAK(pVarWorld, , "g_WorldMatrix not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarView, , "g_ViewMatrix not found.");
        IF_NULL_RETURN_MSG_BREAK(pVarProj, , "g_ProjMatrix not found.");

        IF_NULL_RETURN_MSG_BREAK(pVelocityDir, , "g_vVelocityDir not found.");
        IF_NULL_RETURN_MSG_BREAK(pIntensity, , "g_fIntensity not found.");

        IF_NULL_RETURN_MSG_BREAK(pTime, , "g_fTime not found.");
    }

    const UI_VIEWPORT_RECT& vp = m_upRenderContext->Get_UI_Global().tSceneView;

    _float4x4 matWorld, matView, matProj;
    XMStoreFloat4x4(&matWorld, XMMatrixScaling((_float)vp.vSize.x, (_float)vp.vSize.y, 1.f));
    XMStoreFloat4x4(&matView, XMMatrixIdentity());
    XMStoreFloat4x4(&matProj, XMMatrixOrthographicLH((_float)vp.vSize.x, (_float)vp.vSize.y, 0.f, 1.f));

    pVarWorld->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pVarView->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matView));
    pVarProj->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matProj));

    pTime->AsScalar()->SetFloat(SYS_CORE.Get_FrameDT());
    pIntensity->AsScalar()->SetFloat(m_tPendingSpeedLine.fIntensity);

    _float4 vVelocityDir = { m_tPendingSpeedLine.vVelocityDir.x, m_tPendingSpeedLine.vVelocityDir.y, 0.f, 0.f };
    pVelocityDir->AsVector()->SetFloatVector(reinterpret_cast<const _float*>(&vVelocityDir));

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    IF_NULL_RETURN_MSG_BREAK(pPass, , "Deferred combined pass is nullptr.");

    pPass->Apply(0, m_pContext);

    const MESH_ENTRY* pRectMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pRectMesh, , "Screen rect mesh is nullptr.");

    pRectMesh->Bind_IA(m_pContext);
    pRectMesh->Draw(m_pContext);

    Unbind_PS_SRVs();
    m_tPendingSpeedLine = {};
}


void CRender_System::Render_PostProcess()
{
    SYS_CORE.Bind_PostProcessRTV();

    const _float4 vClear = { 0.f, 0.f, 0.f, 0.f };
    SYS_CORE.Clear_PostProcess_RTV(&vClear);

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(m_hFogShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "Fog shader is nullptr.");

    const uint16_t passIndex = 0; /* Fog */
    if (passIndex >= pShader->pPasses.size())
        return;

    ID3D11ShaderResourceView* pSceneSRV = nullptr;
    ID3D11ShaderResourceView* pDepthSRV = nullptr;

    SYS_CORE.Share_SceneSRV(&pSceneSRV);
    SYS_CORE.Share_DepthSRV(&pDepthSRV);

    if (!pSceneSRV) return;
    if (!pDepthSRV) return;

    auto* pVarWorld = pShader->Get_VarCached("g_WorldMatrix");
    auto* pVarView = pShader->Get_VarCached("g_ViewMatrix");
    auto* pVarProj = pShader->Get_VarCached("g_ProjMatrix");

    auto* pVarViewInv = pShader->Get_VarCached("g_ViewMatrixInverse");
    auto* pVarProjInv = pShader->Get_VarCached("g_ProjMatrixInverse");

    auto* pVarSceneTex = pShader->Get_VarCached("g_SceneTexture");
    auto* pVarDepthTex = pShader->Get_VarCached("g_DepthTexture");

    auto* pFogColor = pShader->Get_VarCached("g_vFogColor");
    auto* pFogParams = pShader->Get_VarCached("g_vFogParams");
    auto* pVarCamPosition = pShader->Get_VarCached("g_vCamPosition");

    IF_NULL_RETURN_MSG_BREAK(pVarWorld, , "g_WorldMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarView, , "g_ViewMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarProj, , "g_ProjMatrix not found.");

    IF_NULL_RETURN_MSG_BREAK(pVarViewInv, , "g_ViewMatrixInverse not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarProjInv, , "g_ProjMatrixInverse not found.");

    IF_NULL_RETURN_MSG_BREAK(pVarSceneTex, , "g_SceneTexture not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarDepthTex, , "g_DepthTexture not found.");

    IF_NULL_RETURN_MSG_BREAK(pFogColor, , "g_vFogColor not found.");
    IF_NULL_RETURN_MSG_BREAK(pFogParams, , "g_vFogParams not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarCamPosition, , "g_vCamPosition not found.");

    const UI_VIEWPORT_RECT& vp = m_upRenderContext->Get_UI_Global().tSceneView;

    _float4x4 matWorld, matView, matProj;
    XMStoreFloat4x4(&matWorld, XMMatrixScaling((_float)vp.vSize.x, (_float)vp.vSize.y, 1.f));
    XMStoreFloat4x4(&matView, XMMatrixIdentity());
    XMStoreFloat4x4(&matProj, XMMatrixOrthographicLH((_float)vp.vSize.x, (_float)vp.vSize.y, 0.f, 1.f));

    const _float4x4& matViewInv = m_upRenderContext->Get_ViewInv();
    const _float4x4& matProjInv = m_upRenderContext->Get_ProjInv();

    const _float3& vCamPos3 = m_upRenderContext->Get_CamPosition();
    _float4 vCamPosition = { vCamPos3.x, vCamPos3.y, vCamPos3.z, 1.f };

    POST_PROCESS_PARAM tParam{};
    tParam.vFogColor = {
        m_tPostProcessDesc.tFog.vColor.x,
        m_tPostProcessDesc.tFog.vColor.y,
        m_tPostProcessDesc.tFog.vColor.z,
        1.f
    };

    tParam.vFogParams = {
        m_tPostProcessDesc.tFog.fStart,
        m_tPostProcessDesc.tFog.fEnd,
        m_tPostProcessDesc.tFog.fDensity,
        m_tPostProcessDesc.tFog.bEnable ? 1.f : 0.f
    };

    pVarWorld->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pVarView->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matView));
    pVarProj->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matProj));

    pVarViewInv->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matViewInv));
    pVarProjInv->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matProjInv));

    pVarSceneTex->AsShaderResource()->SetResource(pSceneSRV);
    pVarDepthTex->AsShaderResource()->SetResource(pDepthSRV);

    pFogColor->SetRawValue(reinterpret_cast<const _float*>(&tParam.vFogColor), 0, sizeof(_float4));
    pFogParams->SetRawValue(reinterpret_cast<const _float*>(&tParam.vFogParams), 0, sizeof(_float4));
    pVarCamPosition->AsVector()->SetFloatVector(reinterpret_cast<const float*>(&vCamPosition));

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    IF_NULL_RETURN_MSG_BREAK(pPass, , "Fog pass is nullptr.");

    pPass->Apply(0, m_pContext);

    const MESH_ENTRY* pRectMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pRectMesh, , "Screen rect mesh is nullptr.");

    pRectMesh->Bind_IA(m_pContext);
    pRectMesh->Draw(m_pContext);

    Unbind_PS_SRVs();
}

void CRender_System::Render_PostProcessComposite()
{
    Bind_BlendState_None();
    Bind_DepthState_Disabled();
    Bind_RasterizerState_Default();

    SYS_CORE.Bind_SceneRTV_WithoutDSV();

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(m_hFogShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "Fog shader is nullptr.");

    const uint16_t passIndex = 1; /* Composite */
    if (passIndex >= pShader->pPasses.size())
        return;

    ID3D11ShaderResourceView* pPostProcessSRV = nullptr;
    SYS_CORE.Share_PostProcessSRV(&pPostProcessSRV);

    if (!pPostProcessSRV)
        return;

    auto* pVarWorld = pShader->Get_VarCached("g_WorldMatrix");
    auto* pVarView = pShader->Get_VarCached("g_ViewMatrix");
    auto* pVarProj = pShader->Get_VarCached("g_ProjMatrix");
    auto* pVarSceneTex = pShader->Get_VarCached("g_SceneTexture");

    IF_NULL_RETURN_MSG_BREAK(pVarWorld, , "g_WorldMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarView, , "g_ViewMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarProj, , "g_ProjMatrix not found.");
    IF_NULL_RETURN_MSG_BREAK(pVarSceneTex, , "g_SceneTexture not found.");

    const UI_VIEWPORT_RECT& vp = m_upRenderContext->Get_UI_Global().tSceneView;

    _float4x4 matWorld, matView, matProj;
    XMStoreFloat4x4(&matWorld, XMMatrixScaling((_float)vp.vSize.x, (_float)vp.vSize.y, 1.f));
    XMStoreFloat4x4(&matView, XMMatrixIdentity());
    XMStoreFloat4x4(&matProj, XMMatrixOrthographicLH((_float)vp.vSize.x, (_float)vp.vSize.y, 0.f, 1.f));

    pVarWorld->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pVarView->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matView));
    pVarProj->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(&matProj));
    pVarSceneTex->AsShaderResource()->SetResource(pPostProcessSRV);

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    IF_NULL_RETURN_MSG_BREAK(pPass, , "PostProcess composite pass is nullptr.");

    pPass->Apply(0, m_pContext);

    const MESH_ENTRY* pRectMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pRectMesh, , "Screen rect mesh is nullptr.");

    pRectMesh->Bind_IA(m_pContext);
    pRectMesh->Draw(m_pContext);

    Unbind_PS_SRVs();
}


void CRender_System::Execute_Pass(RENDER_LAYER layer)
{
    const int idx = LAYER_TO_IDX(layer);
    m_eCurLayer = layer;
    IF_TRUE_RETURN_MSG_BREAK(idx < 0 || idx >= LAYER_TO_IDX(RENDER_LAYER::END), , "Invalid render layer index");

    auto& passCmds = m_LayerCmds[idx];
    if (passCmds.empty())
        return;

    for (auto* pCmd : passCmds)
        Execute_Draw(*pCmd);
}

void CRender_System::Execute_Draw(const DRAW_CMD& cmd)
{
    IF_NULL_RETURN_MSG_BREAK(m_pContext, , "RenderSystem context is nullptr");

    switch (cmd.kind)
    {
    case DRAW_TYPE::MESH:
        Execute_Draw_Mesh(cmd);
        break;

    case DRAW_TYPE::CANVAS:
        Execute_Draw_Canvas(cmd);
        break;

    case DRAW_TYPE::LINE:
        Execute_Draw_Line(cmd);
        break;

    case DRAW_TYPE::TEXT:
        Execute_Draw_Text(cmd);
        break;

    case DRAW_TYPE::SPRITE_EFFECT:
        Execute_Draw_SpriteEffect(cmd);
        break;

    default:
        break;
    }
}

void CRender_System::Execute_Draw_Mesh(const DRAW_CMD& cmd)
{
    if (cmd.mesh.eMode == MESH_MODE::PARTICLE)
    {
        Execute_Draw_Particle(cmd);
        return;
    }

    if (!SYS_RESOURCE.Is_ModelHandle(cmd.mesh.hMesh))
    {
        Execute_Draw_Mesh_Inner(cmd.mesh.hMesh, cmd.mesh.hMaterial, cmd.mesh.hTransform, cmd.mesh.hAnimator, cmd.mesh.hPerObjectParams,
            cmd.mesh.firstIndex, cmd.mesh.indexCount, cmd.mesh.pSkinningMatrices, cmd.mesh.matAttach, cmd.mesh.eMode);
        return;
    }

    const MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(cmd.mesh.hMesh);
    IF_NULL_RETURN_MSG_BREAK(pModel, , "Model is nullptr.");

    _bool bForceCmdMaterial = false;
    if (cmd.mesh.hMaterial != INVALID_HANDLE_UINT)
    {
        MATERIAL_ENTRY* pCmdMat = SYS_RESOURCE.Get_Material(cmd.mesh.hMaterial);
        if (pCmdMat && pCmdMat->eRenderType == MATERIAL_RENDER_TYPE::OUTLINE)
            bForceCmdMaterial = true;
    }

    for (size_t i = 0; i < pModel->parts.size(); ++i)
    {
        const auto& part = pModel->parts[i];

        if (part.hMesh == INVALID_HANDLE_UINT)
            continue;

        uint32_t hMat = INVALID_HANDLE_UINT;

        if (bForceCmdMaterial)
        {
            hMat = cmd.mesh.hMaterial;
        }
        else
        {
            /* override */
            if (cmd.mesh.pMeshRendererData &&
                i < cmd.mesh.pMeshRendererData->vecOverrideMaterials.size())
            {
                uint32_t overrideMat =
                    cmd.mesh.pMeshRendererData->vecOverrideMaterials[i];

                if (overrideMat != INVALID_HANDLE_UINT)
                    hMat = overrideMat;
            }

            /* part material */
            if (hMat == INVALID_HANDLE_UINT && part.hMaterial != INVALID_HANDLE_UINT)
                hMat = part.hMaterial;

            /* fallback */
            if (hMat == INVALID_HANDLE_UINT)
                hMat = cmd.mesh.hMaterial;
        }

        if (hMat == INVALID_HANDLE_UINT)
            continue;

        Execute_Draw_Mesh_Inner(
            part.hMesh,
            hMat,
            cmd.mesh.hTransform,
            cmd.mesh.hAnimator,
            cmd.mesh.hPerObjectParams,
            part.iFirstIndex,
            part.iIndexCount,
            cmd.mesh.pSkinningMatrices,
            cmd.mesh.matAttach,
            cmd.mesh.eMode);
    }
}

void CRender_System::Execute_Draw_Canvas(const DRAW_CMD& tCmd)
{
    if (tCmd.kind != DRAW_TYPE::CANVAS)
        return;

    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pMesh, , "UI Rect Mesh is nullptr.");

    MATERIAL_ENTRY* pMat = SYS_RESOURCE.Get_Material(tCmd.canvas.hMaterial);
    IF_NULL_RETURN_MSG_BREAK(pMat, , "Material is nullptr.");

    const SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(pMat->hShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "Shader is nullptr.");

    const uint16_t passIndex = pMat->passIndex;
    if (passIndex >= pShader->pPasses.size())
        return;

    const auto rt = m_pRectTransform_Processor->Get_Proxy(COMPONENT_TYPE::RECT_TRANSFORM, tCmd.canvas.hRectTransform);
    IF_TRUE_RETURN_MSG_BREAK(!rt.Is_Valid(), , "RectTransform proxy invalid.");

    const _matrix matWorld = Engine::Math::Load(rt->matWorld);

    IF_NULL_RETURN_MSG_BREAK(pMat->pWorld, , "pWorld is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pView, , "pView is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pProj, , "pProj is nullptr.");

    D3D11_VIEWPORT vp{};
    UINT n = 1;
    m_pContext->RSGetViewports(&n, &vp);

    auto gUI = m_upRenderContext->Get_UI_Global();

    pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pMat->pView->SetMatrix(reinterpret_cast<const float*>(&gUI.matView));
    pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&gUI.matProj));

    if (pMat->pBaseColor && pMat->pBaseColor->IsValid())
        pMat->pBaseColor->SetFloatVector(reinterpret_cast<const float*>(&tCmd.canvas.vColor));

    if (pMat->pBaseMap && pMat->pBaseMap->IsValid())
    {
        ID3D11ShaderResourceView* srv = nullptr;
        if (tCmd.canvas.hTexture != INVALID_HANDLE_UINT)
        {
            const TEXTURE_ENTRY* tex = SYS_RESOURCE.Get_Texture(tCmd.canvas.hTexture);
            srv = (tex && tex->Is_Valid()) ? tex->SRV() : nullptr;
        }
        pMat->pBaseMap->SetResource(srv);
    }

    const _bool bClip = (tCmd.canvas.flags & CF_CLIP_RECT) != 0;
    if (bClip)
    {
        const RECT_F& c = tCmd.canvas.rcClip;

        D3D11_RECT rectClip{};
        rectClip.left = (LONG)c.fLeft;
        rectClip.top = (LONG)c.fTop;
        rectClip.right = (LONG)(c.fLeft + c.fRight);
        rectClip.bottom = (LONG)(c.fTop + c.fBottom);

        _float2 vPos = rt.Get_PositionPx();
        _float2 vSize = rt.Get_SizePx();

        D3D11_RECT rectImg;
        rectImg.left = To<LONG>(vPos.x - vSize.x * 0.5f);
        rectImg.top = To<LONG>(vPos.y - vSize.y * 0.5f);
        rectImg.right = To<LONG>(vPos.x + vSize.x * 0.5f);
        rectImg.bottom = To<LONG>(vPos.y + vSize.y * 0.5f);

        D3D11_RECT rectFinal;
        rectFinal.left = rectClip.left + rectImg.left;
        rectFinal.top = rectClip.top + rectImg.top;
        rectFinal.right = rectClip.right + rectImg.right;
        rectFinal.bottom = rectClip.bottom + rectImg.bottom;

        if (rectFinal.left > rectFinal.right)  std::swap(rectFinal.left, rectFinal.right);
        if (rectFinal.top > rectFinal.bottom) std::swap(rectFinal.top, rectFinal.bottom);

        m_pContext->RSSetState(m_rsScissor.Get());
        m_pContext->RSSetScissorRects(1, &rectFinal);
    }
    else
    {
        m_pContext->RSSetState(m_rsNoScissor.Get());
    }

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    if (!pPass)
        return;

    pPass->Apply(0, m_pContext);

    pMesh->Bind_IA(m_pContext);
    pMesh->Draw(m_pContext, 0, 0);
}

void CRender_System::Execute_Draw_Line(const DRAW_CMD& tCmd)
{
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> pPrevRS;
    m_pContext->RSGetState(pPrevRS.GetAddressOf());

    Bind_RasterizerState_CullNone();

    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(tCmd.line.hMesh);
    IF_NULL_RETURN_MSG_BREAK(pMesh, , "pMesh is nullptr.");

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(tCmd.line.hShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "pShader is nullptr");

    ID3D11InputLayout* pIL = pShader->pPasses[0].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[0].pPass;
    if (!pPass)
        return;

    ID3DX11Effect* pFx = pShader->pEffect.Get();
    ID3DX11EffectMatrixVariable* pWorld = nullptr;
    ID3DX11EffectMatrixVariable* pView = nullptr;
    ID3DX11EffectMatrixVariable* pProj = nullptr;
    pWorld = pFx->GetVariableByName("g_WorldMatrix")->AsMatrix();
    pView = pFx->GetVariableByName("g_ViewMatrix")->AsMatrix();
    pProj = pFx->GetVariableByName("g_ProjMatrix")->AsMatrix();

    _matrix matWorld = XMMatrixIdentity();

    if (pWorld)
        pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));

    if (pView)
        pView->SetMatrix(reinterpret_cast<const float*>(&SYS_RENDER.Contexts()->Get_View()));

    if (pProj)
        pProj->SetMatrix(reinterpret_cast<const float*>(&SYS_RENDER.Contexts()->Get_Proj()));

    pPass->Apply(0, m_pContext);

    pMesh->Bind_IA(m_pContext);
    pMesh->Draw(m_pContext);

    m_pContext->RSSetState(pPrevRS.Get());
}

void CRender_System::Execute_Draw_Text(const DRAW_CMD& tCmd)
{
    if (tCmd.kind != DRAW_TYPE::TEXT)
        return;

    FONT_ENTRY* pFontEntry = SYS_RESOURCE.Get_Font(tCmd.text.hFont);
    if (!pFontEntry || !pFontEntry->Is_Valid() || !pFontEntry->pFont)
        return;

    auto rt = m_pRectTransform_Processor->Get_Proxy(COMPONENT_TYPE::RECT_TRANSFORM, tCmd.text.hRectTransform);
    if (!rt.Is_Valid())
        return;

    //if (!m_bBeginThisFrame)
    //{
        m_pSpriteBatch->Begin(DirectX::SpriteSortMode_Deferred, m_pBlendState_Alpha);
    //    m_bBeginThisFrame = true;
    //}

    const _float2 vPos = rt->vPosPx;

    DirectX::XMVECTOR vColor = DirectX::XMVectorSet(
        tCmd.text.vColor.x,
        tCmd.text.vColor.y,
        tCmd.text.vColor.z,
        tCmd.text.vColor.w
    );

    if (tCmd.text.vColor.w < 0.f)
    {
        DEBUG_POINT;
    }

    DirectX::XMFLOAT2 vOrigin = { 0.f, 0.f };
    if (tCmd.text.bCenter)
    {
        const wchar_t* pText = tCmd.text.pText ? tCmd.text.pText->c_str() : TEXT("");
        DirectX::XMVECTOR vMeasure = pFontEntry->pFont->MeasureString(pText);
        DirectX::XMFLOAT2 vTextSize{};
        DirectX::XMStoreFloat2(&vTextSize, vMeasure);

        vOrigin = { vTextSize.x * 0.5f, vTextSize.y * 0.5f };
    }


    pFontEntry->pFont->DrawString(
        m_pSpriteBatch.get(),
        tCmd.text.pText ? tCmd.text.pText->c_str() : TEXT(""),
        DirectX::XMFLOAT2(vPos.x + tCmd.text.vOffset.x, vPos.y + tCmd.text.vOffset.y),
        vColor,
        0.f,
        vOrigin,
        tCmd.text.fScale
    );

    m_pSpriteBatch->End();
}

void CRender_System::Render()
{
    m_pContext->RSSetState(m_rsNoScissor.Get());
    Build_RenderQueue();

    m_bBeginThisFrame = false;

    Execute_RenderQueue();
}

void CRender_System::Execute_Draw_Particle(const DRAW_CMD& cmd)
{
    PARTICLE_RUNTIME* pRuntime = m_pMeshRenderer_Processor->Get_ParticleRuntime(cmd.mesh.iParticleRuntime);
    if (!pRuntime)
        return;

    auto* pTr = To<TRANSFORM_DATA*>(m_pTransform_Processor->Get_DataPtr(COMPONENT_TYPE::TRANSFORM, cmd.mesh.hTransform));
    if (!pTr)
        return;

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(m_hVtxParticlePoint);
    if (!pShader || !pShader->pEffect)
        return;

    if (pShader->pPasses.empty() || !pShader->pPasses[0].pInputLayout)
        return;

    TEXTURE_ENTRY* pTextureEntry = SYS_RESOURCE.Get_Texture(pRuntime->hTexture);
    if (!pTextureEntry || !pTextureEntry->pSRV)
        return;

    m_pContext->IASetInputLayout(pShader->pPasses[0].pInputLayout.Get());

    ID3D11Buffer* pBuffers[] =
    {
        pRuntime->pPointVB.Get(),
        pRuntime->pInstanceVB.Get()
    };

    UINT strides[] =
    {
        sizeof(VTXPOS),
        sizeof(VTXPARTICLE_INSTANCE)
    };

    UINT offsets[] = { 0, 0 };

    m_pContext->IASetVertexBuffers(0, 2, pBuffers, strides, offsets);
    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

    ID3DX11EffectMatrixVariable* pWorld =
        pShader->pEffect->GetVariableByName("g_WorldMatrix")->AsMatrix();
    ID3DX11EffectMatrixVariable* pView =
        pShader->pEffect->GetVariableByName("g_ViewMatrix")->AsMatrix();
    ID3DX11EffectMatrixVariable* pProj =
        pShader->pEffect->GetVariableByName("g_ProjMatrix")->AsMatrix();

    ID3DX11EffectShaderResourceVariable* pBaseMap =
        pShader->pEffect->GetVariableByName("g_BaseMap")->AsShaderResource();

    ID3DX11EffectVectorVariable* pCamPosition =
        pShader->pEffect->GetVariableByName("g_vCamPosition")->AsVector();

    if (!pWorld || !pView || !pProj || !pBaseMap || !pCamPosition)
        return;

    _float3 vCamPos3 = SYS_RENDER.Contexts()->Get_CamPosition();
    _float4 vCamPosition = _float4(vCamPos3.x, vCamPos3.y, vCamPos3.z, 1.f);

    pWorld->SetMatrix(reinterpret_cast<const float*>(&pTr->matWorld));
    pView->SetMatrix(reinterpret_cast<const float*>(&m_matView));
    pProj->SetMatrix(reinterpret_cast<const float*>(&m_matProj));
    pBaseMap->SetResource(pTextureEntry->pSRV.Get());
    pCamPosition->SetFloatVector(reinterpret_cast<const float*>(&vCamPosition));

    ID3DX11EffectPass* pPass = pShader->pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0);
    if (!pPass)
        return;

    pPass->Apply(0, m_pContext);
    m_pContext->DrawInstanced(1, pRuntime->iNumInstances, 0, 0);
    m_pContext->GSSetShader(nullptr, nullptr, 0);
}

void CRender_System::Execute_Draw_SpriteEffect(const DRAW_CMD& tCmd)
{
    /* sprite draw cmd 확인 */
    if (tCmd.kind != DRAW_TYPE::SPRITE_EFFECT)
        return;

    /* 공용 quad mesh + material + shader 가져오기 */
    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pMesh, , "SpriteEffect RectMesh is nullptr.");

    MATERIAL_ENTRY* pMat = SYS_RESOURCE.Get_Material(tCmd.sprite.hMaterial);
    IF_NULL_RETURN_MSG_BREAK(pMat, , "SpriteEffect Material is nullptr.");

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(pMat->hShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "SpriteEffect Shader is nullptr.");

    const uint16_t passIndex = 1;//pMat->passIndex;
    if (passIndex >= pShader->pPasses.size())
        return;

    /* transform */
    const auto tr = m_pTransform_Processor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, tCmd.sprite.hTransform);
    IF_TRUE_RETURN_MSG_BREAK(!tr.Is_Valid(), , "SpriteEffect Transform invalid.");

    _matrix matWorld = Math::Load(tr->matWorld);

    /* billboard면 여기서 월드 보정 */
    if (tCmd.sprite.bBillboard)
    {
        _float3 vPos = tr->vPosition;

        _matrix matScale = XMMatrixScaling(tCmd.sprite.vSize.x, tCmd.sprite.vSize.y, 1.f);
        _matrix matInvView = XMMatrixInverse(nullptr, Math::Load(m_matView));
        matInvView.r[3] = XMVectorSet(0.f, 0.f, 0.f, 1.f);

        _matrix matTrans = XMMatrixTranslation(vPos.x, vPos.y, vPos.z);

        matWorld = matScale * matInvView * matTrans;
    }

    IF_NULL_RETURN_MSG_BREAK(pMat->pWorld, , "pWorld is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pView, , "pView is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pProj, , "pProj is nullptr.");

    pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pMat->pView->SetMatrix(reinterpret_cast<const float*>(&m_matView));
    pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&m_matProj));

    /* sprite texture */
    if (pMat->pBaseMap)
    {
        const TEXTURE_ENTRY* pTex = SYS_RESOURCE.Get_Texture(tCmd.sprite.hTexture);
        ID3D11ShaderResourceView* pSRV = nullptr;

        if (!pTex || !pTex->Is_Valid())
            pSRV = SYS_RESOURCE.Get_Texture(m_hDefaultBaseMap)->pSRV.Get();
        else
            pSRV = pTex->SRV();

        pMat->pBaseMap->SetResource(pSRV);
    }

    /* tint color */
    if (pMat->pBaseColor)
        pMat->pBaseColor->SetFloatVector(reinterpret_cast<const float*>(&tCmd.sprite.vColor));

    /* sprite sheet 전용 변수
       이름은 사용자님 shader 변수명에 맞게 바꾸면 됩니다 */
    if (auto* pFrame = pShader->Get_VarCached("g_iFrame"))
        pFrame->AsScalar()->SetInt((_int)tCmd.sprite.iFrame);

    if (auto* pRow = pShader->Get_VarCached("g_iRow"))
        pRow->AsScalar()->SetInt((_int)tCmd.sprite.iRow);

    if (auto* pCol = pShader->Get_VarCached("g_iCol"))
        pCol->AsScalar()->SetInt((_int)tCmd.sprite.iCol);

    /* material block */
    Apply_Block_To_Shader(pShader, pMat->materialParams);

    /* per object block */
    if (tCmd.sprite.hPerObjectParams != INVALID_HANDLE_UINT)
    {
        PER_OBJECT_PARAM_BLOCK* pBlk = SYS_RESOURCE.Get_PerObjectParamBlock(tCmd.sprite.hPerObjectParams);
        if (pBlk)
            Apply_Block_To_Shader(pShader, pBlk->block);
    }

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    if (!pPass)
        return;

    pPass->Apply(0, m_pContext);

    pMesh->Bind_IA(m_pContext);
    pMesh->Draw(m_pContext, 0, 6);
}

void CRender_System::Execute_Draw_Mesh_Inner(uint32_t hMesh, uint32_t hMaterial, COMPONENT_HANDLE hComponent, COMPONENT_HANDLE hAnimator,
                                             uint32_t hPerObjectParams, uint32_t iFirstIdx, uint32_t iNumIdx, const std::vector<_float4x4>* pSkinningMatrices, const _float4x4& matAttach, MESH_MODE eMode)
{
    /* 메쉬 + 머테리얼 + 셰이더 리소스 가져오기 */
    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(hMesh);
    IF_NULL_RETURN_MSG_BREAK(pMesh, , "Mesh is nullptr.");

    MATERIAL_ENTRY* pMat = SYS_RESOURCE.Get_Material(hMaterial);
    if (!pMat) return;
    IF_NULL_RETURN_MSG_BREAK(pMat, , "Material is nullptr.");

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(pMat->hShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "Shader is nullptr.");

    const uint16_t passIndex = pMat->passIndex;
    if (passIndex >= pShader->pPasses.size())
        return;

    /* 행렬 설정하기 : 월드, 뷰, 투영 */
    const auto tr = m_pTransform_Processor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, hComponent);
    IF_TRUE_RETURN_MSG_BREAK(!tr.Is_Valid(), , "Transform proxy invalid.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pWorld, , "pWorld is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pView, , "pView is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pProj, , "pProj is nullptr.");


    _matrix matWorld;

    if (eMode == MESH_MODE::ATTACH)
        matWorld = Math::Load(matAttach);
    else
        matWorld = Engine::Math::Load(tr->matWorld);

    pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pMat->pView->SetMatrix(reinterpret_cast<const float*>(&m_matView));     /* TODO : 렌더링 최적화 !! 프레임 당 한 번으로 수정 */
    pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&m_matProj));     /* TODO : 렌더링 최적화 !! 프레임 당 한 번으로 수정 */

    if (pMat->pBaseColor)
        pMat->pBaseColor->SetFloatVector(reinterpret_cast<const float*>(&pMat->baseColor));

    if (pMat->pShininess)
        pMat->pShininess->SetFloat(pMat->fShininess);

    if (pMat->pBaseMap)
    {
        const TEXTURE_ENTRY* pTex = SYS_RESOURCE.Get_Texture(pMat->hBaseMap);
        ID3D11ShaderResourceView* pBaseSRV = nullptr;
        if (!pTex || !pTex->Is_Valid())
            pBaseSRV = SYS_RESOURCE.Get_Texture(m_hDefaultBaseMap)->pSRV.Get(); /* 흰 이미지로 설정 */
        else
            pBaseSRV = pTex->SRV(); /* 지정된 baseMap으로 설정 */
        pMat->pBaseMap->SetResource(pBaseSRV);
    }

    if (pMat->pNormalMap)
    {
        const TEXTURE_ENTRY* pTex = SYS_RESOURCE.Get_Texture(pMat->hNormalMap);
        ID3D11ShaderResourceView* pNormalSRV = nullptr;
        if (!pTex || !pTex->Is_Valid())
            pNormalSRV = SYS_RESOURCE.Get_Texture(m_hDefaultNormalMap)->pSRV.Get(); /* flat normal 설정 */
        else
            pNormalSRV = pTex->SRV(); /* 지정된 baseMap으로 설정 */
        pMat->pNormalMap->SetResource(pNormalSRV);
    }

    Apply_Block_To_Shader(pShader, pMat->materialParams);

    /* AnimMesh 인 경우 */
    if (pMat->pBoneMatrices != nullptr)
    {
        /* PARTS 메쉬인 경우 */
        if (eMode == MESH_MODE::PARTS)
        {
            if (pSkinningMatrices != nullptr && !pSkinningMatrices->empty())
            {
                pMat->pBoneMatrices->SetMatrixArray(
                    reinterpret_cast<const float*>(pSkinningMatrices->data()),
                    0,
                    static_cast<UINT>(pSkinningMatrices->size()));
            }
        }
        /* 부모 메쉬인 경우 */
        else if (eMode == MESH_MODE::NONE)
        {
            if (hAnimator.Is_Valid())
            {
                ANIMATOR_DATA* pAnim = To<ANIMATOR_DATA*>(m_pAnimator_Processor->Get_DataPtr(COMPONENT_TYPE::ANIMATOR, hAnimator));
                if (pAnim && !pAnim->finalBoneMatrices.empty())
                {
                    pMat->pBoneMatrices->SetMatrixArray(
                        reinterpret_cast<const float*>(pAnim->finalBoneMatrices.data()),
                        0,
                        static_cast<UINT>(pAnim->finalBoneMatrices.size()));
                }
            }
        }
        else if (eMode == MESH_MODE::ATTACH)
        {
            /* NOTE : attach는 bone matrix를 쓰지 않음 */
        }
    }

    /* 사용자가 정의한 셰이더 변수 적용 */
    if (hPerObjectParams != INVALID_HANDLE_UINT)
    {
        PER_OBJECT_PARAM_BLOCK* pBlk = SYS_RESOURCE.Get_PerObjectParamBlock(hPerObjectParams);
        if (pBlk)
            Apply_Block_To_Shader(pShader, pBlk->block);
    }

    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    if (!pPass)
        return;

    const _bool bOutline = (pMat->eRenderType == MATERIAL_RENDER_TYPE::OUTLINE);

    if (bOutline)
    {
        Bind_BlendState_None();
        Bind_DepthState_ReadOnly();
        Bind_RasterizerState_CullCw();
    }

    pPass->Apply(0, m_pContext);

    pMesh->Bind_IA(m_pContext);
    pMesh->Draw(m_pContext, iFirstIdx, iNumIdx);

    if (bOutline)
    {
        if (m_eCurLayer == RENDER_LAYER::NONBLEND)
            Apply_Pass_State_NonBlend();
        else if (m_eCurLayer == RENDER_LAYER::BLEND)
            Apply_Pass_State_Blend();
        else if (m_eCurLayer == RENDER_LAYER::PRIORITY)
            Apply_Pass_State_Priority();
        else if (m_eCurLayer == RENDER_LAYER::UI)
            Apply_Pass_State_UI();
        else if (m_eCurLayer == RENDER_LAYER::SKY)
            Apply_Pass_State_Skybox();
    }
}


CRender_Context* CRender_System::Contexts()
{
    return m_upRenderContext.get();
}

_bool CRender_System::Submit_Camera(_fmatrix matView, _fmatrix matProj)
{
    if (bSubmittedThisFrame)
        return false;

    m_upRenderContext->Set_View(matView);
    m_upRenderContext->Set_Proj(matProj);

    return bSubmittedThisFrame = true;
}

const UI_GLOBAL& CRender_System::Get_UI_Global()
{
    return m_upRenderContext->Get_UI_Global();
}

void CRender_System::Set_UI_Global(const UI_GLOBAL& tUI)
{
    m_upRenderContext->Set_UI_Global(tUI);
}

void CRender_System::Apply_Block_To_Shader(SHADER_ENTRY* pShader, const NAME_VALUE_PARAM_BLOCK& blk)
{
    if (!pShader || !pShader->pEffect)
        return;

    for (const auto& it : blk.params)
    {

        ID3DX11EffectVariable* pVar = pShader->Get_VarCached(it.strName.c_str());
        if (!pVar)
        {
            //_DEBUG_ERROR_BREAK("EffectVariable is nullptr");
            continue;
        }

        switch (it.value.eType)
        {
        case PARAM_TYPE::FLOAT:
        {
            const _float* p = std::get_if<_float>(&it.value.data);
            if (p) pVar->AsScalar()->SetFloat(*p);
            break;
        }
        case PARAM_TYPE::FLOAT4:
        {
            const _float4* p = std::get_if<_float4>(&it.value.data);
            if (p) pVar->AsVector()->SetFloatVector(reinterpret_cast<const float*>(p));
            break;
        }
        case PARAM_TYPE::FLOAT4X4:
        {
            const _float4x4* p = std::get_if<_float4x4>(&it.value.data);
            if (p) pVar->AsMatrix()->SetMatrix(reinterpret_cast<const float*>(p));
            break;
        }
        case PARAM_TYPE::TEXTURE_HANDLE:
        {
            const uint32_t* p = std::get_if<uint32_t>(&it.value.data);
            if (!p) break;

            const TEXTURE_ENTRY* pTex = SYS_RESOURCE.Get_Texture(*p);
            if (pTex && pTex->Is_Valid())
                pVar->AsShaderResource()->SetResource(pTex->SRV());
            break;
        }
        default:
            break;
        }
    }
}

void CRender_System::Apply_Pass_State_Skybox()
{
    Bind_BlendState_None();
    Bind_DepthState_Disabled();
    Bind_RasterizerState_CullCw();
}

void CRender_System::Apply_Pass_State_Priority()
{
    Bind_BlendState_None();
    Bind_DepthState_Default();
    Bind_RasterizerState_Default();
}

void CRender_System::Apply_Pass_State_NonBlend()
{
    Bind_BlendState_None();
    Bind_DepthState_Default();
    Bind_RasterizerState_Default();
}

void CRender_System::Apply_Pass_State_Light()
{
    Bind_BlendState_Add();
    Bind_DepthState_Disabled();
    Bind_RasterizerState_Default();
}

void CRender_System::Apply_Pass_State_Combined()
{
    Bind_BlendState_None();
    Bind_DepthState_Disabled();
    Bind_RasterizerState_Default();
}

void CRender_System::Apply_Pass_State_PostProcess()
{
    Bind_BlendState_None();
    Bind_DepthState_Disabled();
    Bind_RasterizerState_Default();
}

void CRender_System::Apply_Pass_State_Blend()
{
    SYS_CORE.Bind_SceneRTV();

    Bind_BlendState_Alpha();
    Bind_DepthState_ReadOnly();
    Bind_RasterizerState_Default();
}

void CRender_System::Apply_Pass_State_UI()
{
    Bind_BlendState_Alpha();
    Bind_DepthState_Disabled();
    Bind_RasterizerState_Default();
}

void CRender_System::Bind_BlendState_None()
{
    const FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    m_pContext->OMSetBlendState(m_pBlendState_None, blendFactor, 0xffffffff);
}

void CRender_System::Bind_BlendState_Alpha()
{
    const FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    m_pContext->OMSetBlendState(m_pBlendState_Alpha, blendFactor, 0xffffffff);
}

void CRender_System::Bind_BlendState_Add()
{
    const FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
    m_pContext->OMSetBlendState(m_pBlendState_Add, blendFactor, 0xffffffff);
}

void CRender_System::Bind_DepthState_Default()
{
    m_pContext->OMSetDepthStencilState(m_pDepthState_Default, 0);
}

void CRender_System::Bind_DepthState_ReadOnly()
{
    m_pContext->OMSetDepthStencilState(m_pDepthState_ReadOnly, 0);
}

void CRender_System::Bind_DepthState_Disabled()
{
    m_pContext->OMSetDepthStencilState(m_pDepthState_Disabled, 0);
}

void CRender_System::Bind_RasterizerState_Default()
{
    m_pContext->RSSetState(m_pRasterizerState_Default);
}

void CRender_System::Bind_RasterizerState_CullCw()
{
    m_pContext->RSSetState(m_pRasterizerState_CullCw);
}

void CRender_System::Bind_RasterizerState_CullNone()
{
    m_pContext->RSSetState(m_pRasterizerState_CullNone);
}

void CRender_System::Unbind_PS_SRVs()
{
    ID3D11ShaderResourceView* pNullSRVs[8] = {};
    m_pContext->PSSetShaderResources(0, 8, pNullSRVs);
}

void CRender_System::Set_PostProcessDesc(const POST_PROCESS_DESC& tPostProcessDesc)
{
    //m_tPostProcessDesc = tPostProcessDesc;
}

const POST_PROCESS_DESC& CRender_System::Get_PostProcessDesc() const
{
    return m_tPostProcessDesc;
}
