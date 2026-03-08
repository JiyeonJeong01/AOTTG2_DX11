#include "CanvasRenderer_Processor.h"

#include "Engine_Log.h"
#include "Resource_System.h"
#include "Component_Spec.h"
#include "RectTransform_Processor.h"
#include "Engine_Math.h"
#include "GameObject.h"
#include "CRender_System.h"
#include "Shader.h"

#include "BuiltIn_GUID.h"
#include "CRender_System.h"

NS_BEGIN(Engine)

CCanvasRenderer_Processor::CCanvasRenderer_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CRectTransform_Processor* pRTProcessor)
    : m_pDevice(pDevice), m_pContext(pContext), m_pRectTransform_Processor(pRTProcessor)
{
}

CCanvasRenderer_Processor::~CCanvasRenderer_Processor() = default;

HRESULT CCanvasRenderer_Processor::Initialize()
{
    IF_NULL_RETURN_MSG_BREAK(m_pRectTransform_Processor, E_FAIL, "Transform Processor is nullptr");

    /* RectTex 기본 메쉬 */
    m_hUIRectMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT);

    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;

    rd.ScissorEnable = FALSE;
    IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rd, m_rsNoScissor.GetAddressOf()), E_FAIL, "CCanvasRenderer_Processor initialize failed");

    rd.ScissorEnable = TRUE;
    IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rd, m_rsScissor.GetAddressOf()), E_FAIL, "CCanvasRenderer_Processor initialize failed");

    /* 팩토리 등록 */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CCanvasRenderer, CANVAS_RENDERER_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CCanvasRenderer>();
    }
    return S_OK;
}

void CCanvasRenderer_Processor::Update(_float fDT)
{
}

void CCanvasRenderer_Processor::LateUpdate(_float fDT)
{
}

void CCanvasRenderer_Processor::Build_RenderQueue(vector<DRAW_CMD>& outCmds)
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
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable) continue;

            if (pData->hMaterial == INVALID_HANDLE_UINT)
                continue;

            DRAW_CMD tCmd = DRAW_CMD::Create_Canvas(pData->hMaterial, pData->hTexture, pData->hRectTransform, pData->flags, pData->sortZ, pData->rcUV, pData->vColor, pData->rcClip);
            tCmd.sortKey = Make_SortKey(*pData);
            tCmd.eLayer = RENDER_LAYER::UI;
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
    pData->hMaterial = SYS_RESOURCE.Load_Material(spec->materialGUID);

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

    pData->hRectTransform = pObj->Get_Component<CRectTransform>().Get_Handle();
    pData->hMaterial = SYS_RESOURCE.Load_Material(DefaultAssetGuid::MATERIAL_VTXTEX);
    pData->layer = RENDER_LAYER::NONBLEND;
    pData->flags = RF_NONE;
    ASSET_GUID tmp("8976CDE9-2AE4-4DFC-A580-17E41FFB00D3");
    pData->hTexture = SYS_RESOURCE.Load_Texture(tmp);
}

uint64_t CCanvasRenderer_Processor::Make_SortKey(const CANVAS_RENDERER_DATA& tData) const
{
    uint64_t key = 0;

    const uint64_t layer = (uint64_t)((uint8_t)tData.layer & 0xF);

    _float z = tData.sortZ;
    if (z < 0.f) z = 0.f;
    if (z > 1.f) z = 1.f;
    const uint64_t zq = (uint64_t)(z * 65535.f + 0.5f);

    const uint64_t priority = (uint64_t)(tData.visualPriority & 0xFF);
    const uint64_t material = (uint64_t)(tData.hMaterial & 0xFFFFF);
    const uint64_t texture = (uint64_t)(tData.hTexture & 0xFFFFF);

    key |= (layer << 60);        // 4bit
    key |= (zq << 44);           // 16bit
    key |= (priority << 36);     // 8bit
    key |= (material << 16);     // 20bit
    key |= (texture >> 4);       // 남는 비트 맞춤용 예시

    return key;
}
std::unique_ptr<CCanvasRenderer_Processor> CCanvasRenderer_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CRectTransform_Processor* pProcessor)
{
    auto pInstance = std::make_unique<CCanvasRenderer_Processor>(pDevice, pContext, pProcessor);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}

NS_END
