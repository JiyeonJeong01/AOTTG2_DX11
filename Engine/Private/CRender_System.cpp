#pragma region HEADER
// Render_System.cpp
#include "CRender_System.h"

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

static float s_fWidth = 0;
static float s_fHeight= 0;

HRESULT CRender_System::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWidth, _uint iHeight)
{
    s_fWidth = (_float)iWidth;
    s_fHeight = (_float)iHeight;

    m_pDevice = pDevice;
    m_pContext = pContext;

    IF_NULL_RETURN_MSG_BREAK(m_pDevice, E_FAIL, "RenderSystem device is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_pContext, E_FAIL, "RenderSystem context is nullptr");

    Create_RenderState();

    /* UI 메쉬 = VtxRect 준비 */
    m_hUIRectMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT);
    IF_TRUE_RETURN_MSG_BREAK(m_hUIRectMesh == INVALID_HANDLE_UINT, E_FAIL, "UI rect mesh load failed");

    /* 머테리얼 기본 텍스쳐 핸들 준비 */
    m_hDefaultBaseMap = SYS_RESOURCE.Load_Texture(DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT);
    IF_TRUE_RETURN_MSG_BREAK(m_hDefaultBaseMap == INVALID_HANDLE_UINT, E_FAIL, "DefaultBaseMap load failed");
    m_hDefaultNormalMap = SYS_RESOURCE.Load_Texture(DEFAULT_ASSET_GUID::TEXTURE_NORMALMAP_DEFAULT);
    IF_TRUE_RETURN_MSG_BREAK(m_hDefaultNormalMap == INVALID_HANDLE_UINT, E_FAIL, "DefaultNormalMap load failed");

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

    m_hVtxColShader = SYS_RESOURCE.Load_Shader(DEFAULT_ASSET_GUID::SHADER_VTXCOL);
    m_hVtxParticlePoint = SYS_RESOURCE.Load_Shader(DEFAULT_ASSET_GUID::SHADER_VTXPARTICLEPOINT);
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

    return S_OK;
}

void CRender_System::Submit_LineMesh(const DRAW_CMD& cmd)
{
    m_PendingDrawCmds.push_back(cmd);
}

void CRender_System::Build_RenderQueue()
{
    for (int i = 0; i < LAYER_TO_IDX(RENDER_LAYER::END); ++i)
        m_LayerCmds[i].clear();

    /* 모든 DRAW_CMD 빌드하기 */
    m_AllDrawCmds.clear();

    // TODO : 빌드 전 벡터 사이즈 조절 할 수 있는 로직 추가하기. 잦은 재할당 방지.

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
    /* TODO : 프로파일링 할 부분  */
    //{
    //    auto& q = m_LayerCmds[LAYER_TO_IDX(RENDER_LAYER::NONBLEND)];
    //    std::sort(q.begin(), q.end(), [](const DRAW_CMD* a, const DRAW_CMD* b)
    //        {
    //            return a->sortKey < b->sortKey;
    //        });
    //}
    //{
    //    auto& q = m_LayerCmds[LAYER_TO_IDX(RENDER_LAYER::BLEND)];
    //    std::sort(q.begin(), q.end(), [](const DRAW_CMD* a, const DRAW_CMD* b)
    //        {
    //            return a->sortKey > b->sortKey;
    //        });
    //}
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
    m_eCurLayer = RENDER_LAYER::SKY;
    Execute_Pass(RENDER_LAYER::SKY);

    Apply_Pass_State_Priority();
    m_eCurLayer = RENDER_LAYER::PRIORITY;
    Execute_Pass(RENDER_LAYER::PRIORITY);

    Apply_Pass_State_NonBlend();
    m_eCurLayer = RENDER_LAYER::NONBLEND;
    Execute_Pass(RENDER_LAYER::NONBLEND);

    Apply_Pass_State_Blend();
    m_eCurLayer = RENDER_LAYER::BLEND;
    Execute_Pass(RENDER_LAYER::BLEND);

    Apply_Pass_State_UI();
    m_eCurLayer = RENDER_LAYER::UI;
    Execute_Pass(RENDER_LAYER::UI);

    m_PendingDrawCmds.clear();
}

void CRender_System::Execute_Pass(RENDER_LAYER layer)
{
    const int idx = LAYER_TO_IDX(layer);
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
    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(tCmd.line.hMesh);
    IF_NULL_RETURN_MSG_BREAK(pMesh, , "pMesh is nullptr.");

    SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(m_hVtxColShader);
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

    const _float2 vPos = rt->vPosPx;

    DirectX::XMVECTOR vColor = DirectX::XMVectorSet(
        tCmd.text.vColor.x,
        tCmd.text.vColor.y,
        tCmd.text.vColor.z,
        tCmd.text.vColor.w
    );

    m_pSpriteBatch->Begin();

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


void CRender_System::Execute_Draw_Mesh_Inner(uint32_t hMesh, uint32_t hMaterial, COMPONENT_HANDLE hComponent, COMPONENT_HANDLE hAnimator,
    uint32_t hPerObjectParams, uint32_t iFirstIdx, uint32_t iNumIdx, const std::vector<_float4x4>* pSkinningMatrices, const _float4x4& matAttach, MESH_MODE eMode)
{
    /* 메쉬 + 머테리얼 + 셰이더 리소스 가져오기 */
    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(hMesh);
    IF_NULL_RETURN_MSG_BREAK(pMesh, , "Mesh is nullptr.");

    MATERIAL_ENTRY* pMat = SYS_RESOURCE.Get_Material(hMaterial);
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

        cout << "Apply_Block_To_Shader param = [" << it.strName << "]" << endl;

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

void CRender_System::Apply_Pass_State_Blend()
{
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

