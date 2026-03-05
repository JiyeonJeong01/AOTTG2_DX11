// Render_System.cpp
#include "CRender_System.h"

#include "Component_System.h"
#include "Engine_Math.h"
#include "Event_System.h"
#include "Resource_System.h"
#include "WindowResize_Event.h"

// proxies
#include "Transform_Processor.h"
#include "RectTransform_Processor.h"

// resource types
#include "BuiltIn_GUID.h"
#include "Shader.h"
#include "Material.h"
#include "Mesh.h"
#include "Render_Context.h"
#include "Texture.h"

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

    /* UI 메쉬 = VtxRect 준비 */
    m_hUIRectMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT);
    IF_TRUE_RETURN_MSG_BREAK(m_hUIRectMesh == INVALID_HANDLE_UINT, E_FAIL, "UI rect mesh load failed");

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

    /* 렌더에 필요한 컴포넌트 프로세서 가져오기 */
    {
        CComponent_Processor* pBase = nullptr;
        SYS_COMPONENT.Bind_ComponentProcessor(COMPONENT_TYPE::TRANSFORM, &pBase);
        IF_NULL_RETURN_MSG_BREAK(pBase, E_FAIL, "Transform processor bind failed");
        m_pTransform_Processor = SCAST(CTransform_Processor*, pBase);

        pBase = nullptr;
        SYS_COMPONENT.Bind_ComponentProcessor(COMPONENT_TYPE::RECT_TRANSFORM, &pBase);
        IF_NULL_RETURN_MSG_BREAK(pBase, E_FAIL, "RectTransform processor bind failed");
        m_pRectTransform_Processor = SCAST(CRectTransform_Processor*, pBase);
    }

    m_upRenderContext = CRender_Context::Create(iWidth, iHeight);

    return S_OK;
}

void CRender_System::Priority_Update()
{
    bSubmittedThisFrame = false;

    m_matView = m_upRenderContext->Get_View();
    m_matProj = m_upRenderContext->Get_Proj();
    m_gUI = m_upRenderContext->Get_UI_Global();
}

void CRender_System::Build_RenderQueue()
{
    for (int i = 0; i < LAYER_TO_IDX(RENDER_LAYER::END); ++i)
        m_LayerCmds[i].clear();

    /* 모든 DRAW_CMD 빌드하기 */
    m_AllDrawCmds.clear();

    // TODO : 빌드 전 벡터 사이즈 조절 할 수 있는 로직 추가하기. 잦은 재할당 방지.

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

    //{
    //    auto& q = m_LayerCmds[LAYER_TO_IDX(RENDER_LAYER::UI)];
    //    std::sort(q.begin(), q.end(), [](const DRAW_CMD* a, const DRAW_CMD* b)
    //        {
    //            return a->canvas.sortZ < b->canvas.sortZ;
    //        });
    //}
}

void CRender_System::Execute_RenderQueue()
{
    Execute_Pass(RENDER_LAYER::PRIORITY);
    Execute_Pass(RENDER_LAYER::NONBLEND);
    Execute_Pass(RENDER_LAYER::BLEND);
    Execute_Pass(RENDER_LAYER::UI);
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

    default:
        break;
    }
}

void CRender_System::Execute_Draw_Mesh(const DRAW_CMD& cmd)
{
    if (!SYS_RESOURCE.Is_ModelHandle(cmd.mesh.hMesh))
    {
        Execute_Draw_Mesh_Inner(cmd.mesh.hMesh, cmd.mesh.hMaterial, cmd.mesh.hTransform, cmd.mesh.hPerObjectParams,
            cmd.mesh.firstIndex, cmd.mesh.indexCount);
        return;
    }

    const MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(cmd.mesh.hMesh);
    IF_NULL_RETURN_MSG_BREAK(pModel, , "Model is nullptr.");

    for (const auto& part : pModel->parts)
    {
        /* 파트 머티리얼 없으면 렌더러 머티리얼로 fallback */
        const uint32_t hMat = (part.hMaterial != INVALID_HANDLE_UINT) ? part.hMaterial : cmd.mesh.hMaterial;

        /* 파트 메쉬가 이상하면 건너뜀  */
        if (part.hMesh == INVALID_HANDLE_UINT)
            continue;

        Execute_Draw_Mesh_Inner(part.hMesh, hMat, cmd.mesh.hTransform, cmd.mesh.hPerObjectParams,
            cmd.mesh.firstIndex, cmd.mesh.indexCount);
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

    auto gUI = m_upRenderContext->Get_UI_Global();

    pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pMat->pView->SetMatrix(reinterpret_cast<const float*>(&m_gUI.matView));
    pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&m_gUI.matProj));

    if (pMat->pColor && pMat->pColor->IsValid())
        pMat->pColor->SetFloatVector(reinterpret_cast<const float*>(&tCmd.canvas.vColor));

    if (pMat->pUV && pMat->pUV->IsValid())
    {
        _float4 uv4 = { tCmd.canvas.rcUV.fLeft, tCmd.canvas.rcUV.fTop, tCmd.canvas.rcUV.fRight, tCmd.canvas.rcUV.fBottom };
        pMat->pUV->SetFloatVector(reinterpret_cast<const float*>(&uv4));
    }

    if (pMat->pMainTex && pMat->pMainTex->IsValid())
    {
        ID3D11ShaderResourceView* srv = nullptr;
        if (tCmd.canvas.hTexture != INVALID_HANDLE_UINT)
        {
            const TEXTURE_ENTRY* tex = SYS_RESOURCE.Get_Texture(tCmd.canvas.hTexture);
            srv = (tex && tex->Is_Valid()) ? tex->SRV() : nullptr;
        }
        pMat->pMainTex->SetResource(srv);
    }

    const _bool bClip = (tCmd.canvas.flags & CF_CLIP_RECT) != 0;
    if (bClip)
    {
        const RECT_F& c = tCmd.canvas.rcClip;

        D3D11_RECT r{};
        r.left = (LONG)c.fLeft;
        r.top = (LONG)c.fTop;
        r.right = (LONG)(c.fLeft + c.fRight);
        r.bottom = (LONG)(c.fTop + c.fBottom);

        if (r.left > r.right)  std::swap(r.left, r.right);
        if (r.top > r.bottom) std::swap(r.top, r.bottom);

        m_pContext->RSSetState(m_rsScissor.Get());
        m_pContext->RSSetScissorRects(1, &r);
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

void CRender_System::Render()
{
    m_pContext->RSSetState(m_rsNoScissor.Get());
    Build_RenderQueue();
    Execute_RenderQueue();
}


void CRender_System::Execute_Draw_Mesh_Inner(uint32_t hMesh, uint32_t hMaterial, COMPONENT_HANDLE hComponent, uint32_t hPerObjectParams, uint32_t iFirstIdx, uint32_t iNumIdx)
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

    const _matrix matWorld = Engine::Math::Load(tr->matWorld);

    IF_NULL_RETURN_MSG_BREAK(pMat->pWorld, , "pWorld is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pView, , "pView is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pMat->pProj, , "pProj is nullptr.");

    pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pMat->pView->SetMatrix(reinterpret_cast<const float*>(&m_matView));     /* TODO : 렌더링 최적화 !! 프레임 당 한 번으로 수정 */
    pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&m_matProj));     /* TODO : 렌더링 최적화 !! 프레임 당 한 번으로 수정 */

    /* 재질은 머테리얼이 담당 */
    Apply_Block_To_Shader(pShader, pMat->materialParams);

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

    pPass->Apply(0, m_pContext);

    pMesh->Bind_IA(m_pContext);
    pMesh->Draw(m_pContext, iFirstIdx, iNumIdx);
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
            _DEBUG_ERROR_BREAK("EffectVariable is nullptr");
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

