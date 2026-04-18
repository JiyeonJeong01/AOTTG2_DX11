#include "SpriteEffect_Processor.h"

#include "Engine_Log.h"
#include "Resource_System.h"
#include "Component_Spec.h"
#include "Transform_Processor.h"
#include "GameObject.h"
#include "CRender_System.h"

#include "BuiltIn_GUID.h"

NS_BEGIN(Engine)

CSpriteEffect_Processor::CSpriteEffect_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : m_pDevice(pDevice), m_pContext(pContext)
{
}

CSpriteEffect_Processor::~CSpriteEffect_Processor() = default;

HRESULT CSpriteEffect_Processor::Initialize()
{
    m_pTransform_Processor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransform_Processor, E_FAIL, "Transform Processor is nullptr");

    /* 기본 Rect Mesh */
    m_hUIRectMesh = SYS_RESOURCE.Load_Mesh(DEFAULT_ASSET_GUID::MESH_RECT);

    D3D11_RASTERIZER_DESC rd{};
    rd.FillMode = D3D11_FILL_SOLID;
    rd.CullMode = D3D11_CULL_NONE;
    rd.DepthClipEnable = TRUE;

    rd.ScissorEnable = FALSE;
    IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rd, m_rsNoScissor.GetAddressOf()),
        E_FAIL, "CSpriteEffect_Processor initialize failed");

    rd.ScissorEnable = TRUE;
    IF_FAIL_RETURN_MSG_BREAK(m_pDevice->CreateRasterizerState(&rd, m_rsScissor.GetAddressOf()),
        E_FAIL, "CSpriteEffect_Processor initialize failed");

    /* 팩토리 등록 */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CSpriteEffect, SPRITE_EFFECT_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CSpriteEffect>();
    }

    XMStoreFloat4x4(&m_matView, XMMatrixIdentity());
    XMStoreFloat4x4(&m_matProj, XMMatrixIdentity());

    return S_OK;
}

void CSpriteEffect_Processor::Update(_float fDT)
{
    const auto& Pages = m_Pool.GetPages();
    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData)
                continue;

            if (false == pData->bEnable)
                continue;

            if (false == pData->bPlay)
                continue;

            if (true == pData->bFinished)
                continue;

            if (pData->iTotalFrame <= 1)
                continue;

            if (pData->fFrameDuration <= 0.f)
                continue;

            pData->fAccTime += fDT;

            while (pData->fAccTime >= pData->fFrameDuration)
            {
                /* 초과시간 남은 만큼 보존 */
                pData->fAccTime -= pData->fFrameDuration;
                ++pData->iCurFrame;

                if (pData->iCurFrame >= pData->iTotalFrame)
                {
                    if (pData->bLoop)
                    {
                        pData->iCurFrame = 0;
                    }
                    else
                    {
                        pData->iCurFrame = (pData->iTotalFrame > 0) ? (pData->iTotalFrame - 1) : 0;
                        pData->bPlay = false;
                        pData->bFinished = true;
                        break;
                    }
                }
            }
        }
    }
}

void CSpriteEffect_Processor::LateUpdate(_float fDT)
{
}

void CSpriteEffect_Processor::Build_RenderQueue(vector<DRAW_CMD>& outCmds)
{
    if (!m_pTransform_Processor)
        return;

    const auto& Pages = m_Pool.GetPages();
    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            if (pData->hMaterial == INVALID_HANDLE_UINT)
                continue;

            if (pData->hTransform == INVALID_HANDLE)
                continue;

            DRAW_CMD tCmd{};

            /* --- 공통 --- */
            tCmd.kind = DRAW_TYPE::SPRITE_EFFECT;
            tCmd.eLayer = pData->layer;
            tCmd.sortKey = Make_SortKey(*pData);

            /* --- sprite 전용 --- */
            tCmd.sprite.hMaterial = pData->hMaterial;
            tCmd.sprite.hTexture = pData->hTexture;
            tCmd.sprite.hTransform = pData->hTransform;
            tCmd.sprite.hPerObjectParams = pData->hPerObjectParams;

            tCmd.sprite.iFrame = pData->iCurFrame;
            tCmd.sprite.iRow = pData->iRow;
            tCmd.sprite.iCol = pData->iCol;

            tCmd.sprite.vSize = pData->vSize;
            tCmd.sprite.vColor = pData->vColor;

            tCmd.sprite.bBillboard = pData->bBillboard;

            tCmd.sprite.pData = pData; /* 필요 시, 셰이더에서 추가 접근용 */

            outCmds.push_back(tCmd);
        }
    }
}

HRESULT CSpriteEffect_Processor::Initialize_From_Spec(
    COMPONENT_TYPE eComType,
    COMPONENT_HANDLE handle,
    const COMPONENT_SPEC_BASE* pSpec)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::SPRITE_EFFECT, E_FAIL, "Wrong component type");

    auto* pData = m_Pool.Get_Data_By_Handle(handle);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid SpriteEffect handle");
    _DEBUG_ENGINE_ASSERT_MSG(pSpec != nullptr, "Spec is nullptr");

    const auto* spec = SCAST(const SPRITE_EFFECT_SPEC*, pSpec);

    pData->hTransform = spec->hTransform;

    pData->hMaterial = SYS_RESOURCE.Load_Material(spec->materialGUID);

    if (spec->textureGUID.Is_Valid())
        pData->hTexture = SYS_RESOURCE.Load_Texture(spec->textureGUID);
    else
        pData->hTexture = INVALID_HANDLE_UINT;

    pData->hPerObjectParams = spec->hPerObjectParams;

    pData->layer = spec->layer;
    pData->flags = spec->flags;

    pData->iRow = spec->iRow;
    pData->iCol = spec->iCol;
    pData->iTotalFrame = spec->iTotalFrame;
    pData->fFrameDuration = spec->fFrameDuration;

    pData->bLoop = spec->bLoop;
    pData->bPlay = spec->bPlay;
    pData->bBillboard = spec->bBillboard;
    pData->bFinished = false;

    pData->vSize = spec->vSize;
    pData->vColor = spec->vColor;

    /* runtime reset */
    pData->fAccTime = 0.f;
    pData->iCurFrame = 0;

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE>
CSpriteEffect_Processor::Build_Spec(
    COMPONENT_TYPE eComType,
    COMPONENT_HANDLE hComponent)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::SPRITE_EFFECT, nullptr, "Wrong component type.");

    SPRITE_EFFECT_DATA* pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "Invalid SpriteEffect handle");

    auto spec = std::make_unique<SPRITE_EFFECT_SPEC>();

    spec->hTransform = pData->hTransform;
    spec->hPerObjectParams = pData->hPerObjectParams;

    if (pData->hMaterial != INVALID_HANDLE_UINT)
    {
        const MATERIAL_ENTRY* pMtrl = SYS_RESOURCE.Get_Material(pData->hMaterial);
        if (pMtrl && pMtrl->tGUID.Is_Valid())
            spec->materialGUID = pMtrl->tGUID;
        else
            spec->materialGUID = DEFAULT_ASSET_GUID::MATERIAL_VTXTEX;
    }
    else
    {
        spec->materialGUID = DEFAULT_ASSET_GUID::MATERIAL_VTXTEX;
    }

    if (pData->hTexture != INVALID_HANDLE_UINT)
    {
        const TEXTURE_ENTRY* pTex = SYS_RESOURCE.Get_Texture(pData->hTexture);
        if (pTex && pTex->tGUID.Is_Valid())
            spec->textureGUID = pTex->tGUID;
    }

    spec->layer = pData->layer;
    spec->flags = pData->flags;

    spec->iRow = pData->iRow;
    spec->iCol = pData->iCol;
    spec->iTotalFrame = pData->iTotalFrame;
    spec->fFrameDuration = pData->fFrameDuration;

    spec->bLoop = pData->bLoop;
    spec->bPlay = pData->bPlay;
    spec->bBillboard = pData->bBillboard;

    spec->vSize = pData->vSize;
    spec->vColor = pData->vColor;

    return spec;
}

void CSpriteEffect_Processor::Initialize_Component_Data(COMPONENT_HANDLE hComponent)
{
    auto* pData = m_Pool.Get_Data_By_Handle(hComponent);
    if (!pData)
        return;

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    if (!pObj)
        return;

    pData->hTransform = pObj->Get_Component<CTransform>().Get_Handle();

    pData->hMaterial = SYS_RESOURCE.Load_Material(DEFAULT_ASSET_GUID::MATERIAL_VTXTEX);
    pData->hTexture = INVALID_HANDLE_UINT;
    pData->hPerObjectParams = INVALID_HANDLE_UINT;

    pData->layer = RENDER_LAYER::BLEND;
    pData->flags = RF_NONE;

    pData->iRow = 1;
    pData->iCol = 1;
    pData->iTotalFrame = 1;
    pData->fFrameDuration = 0.05f;

    pData->bLoop = false;
    pData->bPlay = true;
    pData->bBillboard = true;
    pData->bFinished = false;

    pData->vSize = { 1.f, 1.f };
    pData->vColor = { 1.f, 1.f, 1.f, 1.f };

    pData->fAccTime = 0.f;
    pData->iCurFrame = 0;
}

uint64_t CSpriteEffect_Processor::Make_SortKey(const SPRITE_EFFECT_DATA& tData) const
{
    uint64_t key = 0;

    const uint64_t layer = (uint64_t)((uint8_t)tData.layer & 0xF);
    const uint64_t material = (uint64_t)(tData.hMaterial & 0xFFFFF); // 20bit
    const uint64_t texture = (uint64_t)(tData.hTexture & 0xFFFFF);  // 20bit

    key |= (layer << 60);       // 4bit
    key |= (material << 40);    // 20bit
    key |= (texture << 20);     // 20bit

    return key;
}

std::unique_ptr<CSpriteEffect_Processor> CSpriteEffect_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    auto pInstance = std::make_unique<CSpriteEffect_Processor>(pDevice, pContext);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}

NS_END
