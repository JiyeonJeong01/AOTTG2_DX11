#include "CanvasRenderer_Processor.h"

#include "Engine_Log.h"
#include "Resource_System.h"
#include "Component_Spec.h"
#include "RectTransform_Processor.h"
#include "Engine_Math.h"
#include "GameObject.h"
#include "CRender_System.h"
#include "Shader.h"

NS_BEGIN(Engine)

CCanvasRenderer_Processor::CCanvasRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CRectTransform_Processor* pRTProcessor)
    : m_pDevice(pDevice), m_pContext(pContext), m_pRectTransform_Processor(pRTProcessor)
{
}

CCanvasRenderer_Processor::~CCanvasRenderer_Processor() = default;

HRESULT CCanvasRenderer_Processor::Initialize(_uint iWidth, _uint iHeight)
{
    IF_NULL_RETURN_MSG_BREAK(m_pRectTransform_Processor, E_FAIL, "Transform Processor is nullptr");

    m_hUIRectMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT);

    Math::Store(m_matProj, XMMatrixOrthographicOffCenterLH(0.f, SCAST(_float, iWidth), SCAST(_float, iHeight), 0.f, 0.f, 1.f));
    Math::Store(m_matView, XMMatrixIdentity());

    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;

    rd.ScissorEnable = FALSE;
    IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rd, m_rsNoScissor.GetAddressOf()), E_FAIL, "CCanvasRenderer_Processor initialize failed");

    rd.ScissorEnable = TRUE;
    IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rd, m_rsScissor.GetAddressOf()), E_FAIL, "CCanvasRenderer_Processor initialize failed");

    SYS_COMPONENT.Register_InitialSpecFactory<CCanvasRenderer, CANVAS_RENDERER_SPEC>(COMPONENT_TYPE::CANVAS_RENDERER);

    return S_OK;
}

void CCanvasRenderer_Processor::Update(_float fDT)
{

}

void CCanvasRenderer_Processor::LateUpdate(_float fDT)
{
}

void CCanvasRenderer_Processor::Render()
{
    if (!m_pRectTransform_Processor)
        return;

    std::vector<DRAW_CMD> cmds;
    cmds.reserve(128);

    Build_Queue(cmds);

    std::sort(cmds.begin(), cmds.end(), [](const DRAW_CMD& a, const DRAW_CMD& b)
        {
            return a.sortKey < b.sortKey;
        });

    for (auto& cmd : cmds)
        Execute_Draw(cmd);
}

void CCanvasRenderer_Processor::Begin_Frame()
{
}

void CCanvasRenderer_Processor::End_Frame()
{
}

void CCanvasRenderer_Processor::Build_Queue(std::vector<DRAW_CMD>& outCmds)
{
    if (!m_pRectTransform_Processor)
        return;

    if (m_hUIRectMesh == INVALID_HANDLE_UINT)
        return;

    const auto& Pages = m_Pool.GetPages();
    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Active(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnabled) continue;

            if (pData->hMaterial == INVALID_HANDLE_UINT)
                continue;

            DRAW_CMD tCmd = DRAW_CMD::Create_Canvas(pData->hMaterial, pData->hTexture, pData->hRectTransform, pData->flags, pData->sortZ, pData->rcUV, pData->vColor, pData->rcClip);
            tCmd.sortKey = Make_SortKey(*pData);
            outCmds.push_back(tCmd);
        }
    }
}

HRESULT CCanvasRenderer_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    auto* pData = m_Pool.Get_Data_By_Handle(handle);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid CanvasRenderer handle in Initialize_From_Spec");
    _DEBUG_ENGINE_ASSERT_MSG(pSpec != nullptr, "CanvasRenderer spec is nullptr");

    const auto* spec = SCAST(const CANVAS_RENDERER_SPEC*, pSpec);
    _DEBUG_ENGINE_ASSERT_MSG(spec->Get_Type() == COMPONENT_TYPE::CANVAS_RENDERER, "Spec type mismatch: CANVAS_RENDERER expected");

    // Material
    pData->hMaterial = SYS_RESOURCE.Load_Material_Temp(spec->materialGUID, 0);

    // Texture (optional)
    if (spec->textureGUID.Is_Valid())
        pData->hTexture = SYS_RESOURCE.Load_Texture(spec->textureGUID);
    else
        pData->hTexture = SYS_RESOURCE.Load_Texture(DefaultAssetGuid::TEXTURE_UI_DEFAULT);

    // Params
    pData->vColor = spec->vColor;
    pData->rcUV = spec->rcUV;
    pData->rcClip = spec->rcClip;
    pData->flags = spec->flags;
    pData->layer = spec->layer;
    pData->sortZ = spec->sortZ;

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE>
CCanvasRenderer_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::CANVAS_RENDERER, nullptr, "Wrong component type.");

    CANVAS_RENDERER_DATA* pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "Invalid CanvasRenderer handle in Build_Spec");

    auto spec = std::make_unique<CANVAS_RENDERER_SPEC>();

    spec->materialGUID = DEFAULT_ASSET_GUID::MATERIAL_UI_DEFAULT;

    /* --- Texture GUID --- */
    if (pData->hTexture != INVALID_HANDLE_UINT)
    {
        const TEXTURE_ENTRY* pTex = SYS_RESOURCE.Get_Texture(pData->hTexture);
        if (pTex && pTex->tGUID.Is_Valid())
            spec->textureGUID = pTex->tGUID;
        else
            spec->textureGUID = DefaultAssetGuid::TEXTURE_UI_DEFAULT;
    }

    spec->vColor = pData->vColor;
    spec->rcUV = pData->rcUV;
    spec->rcClip = pData->rcClip;

    spec->flags = pData->flags;
    spec->layer = pData->layer;
    spec->sortZ = pData->sortZ;

    return spec;
}

void CCanvasRenderer_Processor::Initialize_Component_Data(COMPONENT_HANDLE hComponent)
{
    auto* pData = m_Pool.Get_Data_By_Handle(hComponent);
    if (!pData) return;

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    if (!pObj) return;

    pData->hRectTransform = pObj->Get_Component<CRectTransform>(COMPONENT_TYPE::RECT_TRANSFORM).Get_Handle();
}

uint64_t CCanvasRenderer_Processor::Make_SortKey(const CANVAS_RENDERER_DATA& tData) const
{
    uint64_t key = 0;

    // layer (4bit)
    const uint64_t layer = (uint64_t)((uint8_t)tData.layer & 0xF);

    // z (16bit quantize)
    _float z = tData.sortZ;
    if (z < 0.f) z = 0.f;
    if (z > 1.f) z = 1.f;
    const uint64_t zq = (uint64_t)(z * 65535.f + 0.5f);

    // material / texture는 하위 일부 비트만 사용
    const uint64_t material = (uint64_t)(tData.hMaterial & 0xFFFFF); // 20bit
    const uint64_t texture = (uint64_t)(tData.hTexture & 0xFFFFF); // 20bit

    key |= (layer << 60);
    key |= (zq << 44);
    key |= (material << 24);
    key |= (texture << 4);

    return key;
}

void CCanvasRenderer_Processor::Execute_Draw(const DRAW_CMD& tCmd)
{
    if (tCmd.kind != DRAW_TYPE::CANVAS)
        return;

    /* 1. Get MESH_ENTRY & MATERIAL_ENTRY */
    const MESH_ENTRY* pMesh = SYS_RESOURCE.Get_Mesh(m_hUIRectMesh);
    IF_NULL_RETURN_MSG_BREAK(pMesh, , "UI Rect Mesh is nullptr.");

    MATERIAL_ENTRY* pMat = SYS_RESOURCE.Get_Material(tCmd.canvas.hMaterial);
    IF_NULL_RETURN_MSG_BREAK(pMat, , "Material is nullptr.");

    /* 2. Get SHADER_ENTRY through MATERIAL_ENTRY */
    const SHADER_ENTRY* pShader = SYS_RESOURCE.Get_Shader(pMat->hShader);
    IF_NULL_RETURN_MSG_BREAK(pShader, , "Shader is nullptr.");

    const uint16_t passIndex = pMat->passIndex;
    if (passIndex >= pShader->pPasses.size())
        return;

    /* 3. Setting .fx variables */
    // RectTransform world
    const auto rectTransform = m_pRectTransform_Processor->Get_Proxy(COMPONENT_TYPE::RECT_TRANSFORM, tCmd.canvas.hRectTransform);
    IF_TRUE_RETURN_MSG_BREAK(!rectTransform.Is_Valid(), , "RectTransform Proxy is invalid.");

    const _matrix matWorld = Engine::Math::Load(rectTransform->matWorld);

    pMat->pWorld->SetMatrix(reinterpret_cast<const float*>(&matWorld));
    pMat->pView->SetMatrix(reinterpret_cast<const float*>(&m_matView));
    pMat->pProj->SetMatrix(reinterpret_cast<const float*>(&m_matProj));

    // ---- per-instance: Color / UV / Texture ----
    ID3DX11Effect* fx = pShader->pEffect.Get();
    IF_NULL_RETURN_MSG_BREAK(fx, , "Shader effect is nullptr.");

    // ---- per-instance: Color / UV / Texture ----
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

    // ---- Clip (Scissor) ----
    const _bool bClip = (tCmd.canvas.flags & CF_CLIP_RECT) != 0;
    if (bClip)
    {
        const RECT_F& c = tCmd.canvas.rcClip;

        D3D11_RECT r{};
        r.left = (LONG)c.fLeft;
        r.top = (LONG)c.fTop;
        r.right = (LONG)(c.fLeft + c.fRight);
        r.bottom = (LONG)(c.fTop + c.fBottom);

        m_pContext->RSSetState(m_rsScissor.Get());
        m_pContext->RSSetScissorRects(1, &r);
    }
    else
    {
        m_pContext->RSSetState(m_rsNoScissor.Get());
    }

    /* Apply Pass */
    ID3D11InputLayout* pIL = pShader->pPasses[passIndex].pInputLayout.Get();
    m_pContext->IASetInputLayout(pIL);

    ID3DX11EffectPass* pPass = pShader->pPasses[passIndex].pPass;
    if (!pPass) return;

    pPass->Apply(0, m_pContext);

    // Draw: UI는 보통 전체 draw (0,0)
    pMesh->Bind_IA(m_pContext);
    pMesh->Draw(m_pContext, 0, 0);
}

std::unique_ptr<CCanvasRenderer_Processor> CCanvasRenderer_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CRectTransform_Processor* pProcessor, _uint iWidth, _uint iHeight)
{
    auto pInstance = std::make_unique<CCanvasRenderer_Processor>(pDevice, pContext, pProcessor);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(iWidth, iHeight), nullptr, "Create instance failed");
    return pInstance;
}

NS_END
