#include "UI_Processor.h"

#include "CanvasRenderer_Processor.h"
#include "RectTransform_Processor.h" 

#include "UIButton.h"
#include "UIImage.h"
//#include "UIText.h" 나중에 추가

#include "Engine_Log.h"
#include "Input_System.h"

NS_BEGIN(Engine)

static inline _float4 Get_State_Color(const UI_BUTTON_DATA& bData)
{
    switch (bData.eState)
    {
    case UI_BTN_STATE::Normal:   return bData.normal;
    case UI_BTN_STATE::Hover:    return bData.hover;
    case UI_BTN_STATE::Pressed:  return bData.pressed;
    case UI_BTN_STATE::Disabled: return bData.disabled;
    default:                     return bData.normal;
    }
}

static inline void Get_State_Sprite(const UI_BUTTON_DATA& bData, uint32_t& outTex, RECT_F& outUV)
{
    switch (bData.eState)
    {
    case UI_BTN_STATE::Hover:
        outTex = (bData.hoverTex != INVALID_HANDLE_UINT) ? bData.hoverTex : bData.normalTex;
        outUV = (bData.hoverTex != INVALID_HANDLE_UINT) ? bData.hoverUV : bData.normalUV;
        break;;
    case UI_BTN_STATE::Pressed:
        outTex = (bData.pressedTex != INVALID_HANDLE_UINT) ? bData.pressedTex : bData.normalTex;
        outUV = (bData.pressedTex != INVALID_HANDLE_UINT) ? bData.pressedUV : bData.normalUV;
        break;
    default:
        outTex = bData.normalTex;
        outUV = bData.normalUV;
        break;
    }
}

CUI_Processor::CUI_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CCanvasRenderer_Processor* pCanvasProcessor, CRectTransform_Processor* pRTProcessor)
    : CComponent_Processor()
    , m_pCanvasProcessor(pCanvasProcessor), m_pRectTransformProcessor(pRTProcessor)
{
}

CUI_Processor::~CUI_Processor() = default;

HRESULT CUI_Processor::Initialize()
{

    return S_OK;
}

void CUI_Processor::Update(_float fDT)
{
    Sync_Images_To_Canvas();   // PASS 1
    Update_Buttons(fDT);       // PASS 2 
}

void CUI_Processor::LateUpdate(_float fDT)
{
    /* TODO -------------------------------------------------------------------------
     * TODO CanvasRenderer 쪽에서 프레임 시작에 visualPriority=0 초기화하는 등 처리 필요
     * TODO ------------------------------------------------------------------------- */
}

void CUI_Processor::Render()
{
}

COMPONENT_HANDLE CUI_Processor::Create_Component_Data(COMPONENT_TYPE type, OBJECT_HANDLE hObject)
{
    switch (type)
    {
    case COMPONENT_TYPE::UI_BUTTON:
        return Create_Component_Data_Inner<CUIButton>(m_ButtonPool, hObject);
    case COMPONENT_TYPE::UI_IMAGE:
        return Create_Component_Data_Inner<CUIImage>(m_ImagePool, hObject);
    default:
        return COMPONENT_HANDLE{};
    }
}

void CUI_Processor::Remove_Component(COMPONENT_TYPE type, COMPONENT_HANDLE h)
{
    switch (type)
    {
    case COMPONENT_TYPE::UI_BUTTON:
        m_ButtonPool.Deallocate(h);
        break;
    case COMPONENT_TYPE::UI_IMAGE:
        m_ImagePool.Deallocate(h);
        break;
    default:
        break;
    }
}

HRESULT CUI_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::UI_BUTTON :
        return Initialize_From_Spec_UIButton(hComponent, pSpec);
    case COMPONENT_TYPE::UI_IMAGE :
        return Initialize_From_Spec_UIImage(hComponent, pSpec);
    }

    return S_OK;
}

void CUI_Processor::Sync_Images_To_Canvas()
{
    const auto& ImagePages = m_ImagePool.GetPages();
    for (const auto& upPage : ImagePages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Active(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->dirty)
                continue;

            auto crProxy = m_pCanvasProcessor->Get_Proxy(COMPONENT_TYPE::CANVAS_RENDERER, pData->hCanvasRenderer);
            CANVAS_RENDERER_DATA* pCR = crProxy._Data();
            if (!pCR)
            {
                pData->dirty = false;
                continue;
            }

            pCR->hTexture = pData->hTexture;
            pCR->rcUV = pData->rcUV;
            pCR->vColor = pData->color;

            pData->dirty = false;
        }
    }
}

void CUI_Processor::Apply_ButtonVisual(const UI_BUTTON_DATA& tData)
{
    auto crProxy = m_pCanvasProcessor->Get_Proxy(COMPONENT_TYPE::CANVAS_RENDERER, tData.hTargetCanvas);
    CANVAS_RENDERER_DATA* pCR = crProxy._Data();
    if (!pCR) return;

    pCR->vColor = Get_State_Color(tData);

    uint32_t hTex{};
    RECT_F   rcUV{};
    Get_State_Sprite(tData, hTex, rcUV);

    if (hTex != INVALID_HANDLE_UINT)
    {
        pCR->hTexture = hTex;
        pCR->rcUV = rcUV;
    }
}

void CUI_Processor::Update_Buttons(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    const POINT tMousePos = SYS_INPUT.Get_MousePos();
    const _bool bMouseDown = SYS_INPUT.Get_KeyDown(VK_LBUTTON);
    const _bool bMouseUp = SYS_INPUT.Get_KeyUp(VK_LBUTTON);

    const auto& ButtonPages = m_ButtonPool.GetPages();
    for (const auto& upPage : ButtonPages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Active(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData) continue;

            if (!pData->bInteractable)
            {
                if (pData->eState != UI_BTN_STATE::Disabled)
                {
                    pData->eState = UI_BTN_STATE::Disabled;
                    Apply_ButtonVisual(*pData);
                }
                continue;
            }

            auto rt = m_pRectTransformProcessor->Get_Proxy(COMPONENT_TYPE::RECT_TRANSFORM, pData->hRectTransform);
            if (!rt.Is_Valid())
                continue;

            const auto vPos = rt->vPosPx;
            const auto vSize = rt->vSizePx;

            RECT rcBound = {
                SCAST(LONG, (vPos.x - vSize.x * 0.5f)),
                SCAST(LONG, (vPos.y - vSize.y * 0.5f)),
                SCAST(LONG, (vPos.x + vSize.x * 0.5f)),
                SCAST(LONG, (vPos.y + vSize.y * 0.5f))
            };

            const _bool bHit = HitTest_Rect(rcBound, tMousePos);

            UI_BTN_STATE eNext = pData->eState;

            if (!bHit)
            {
                eNext = UI_BTN_STATE::Normal;
            }
            else
            {
                eNext = bMouseDown ? UI_BTN_STATE::Pressed : UI_BTN_STATE::Hover;

                if (bMouseUp && pData->eState == UI_BTN_STATE::Pressed)
                {
                    // TODO: 이벤트 시스템 연결
                    // SYS_EVENT.Enqueue(pData->onClickEventId, pData->hObject);
                }
            }

            if (eNext != pData->eState)
            {
                pData->eState = eNext;
                Apply_ButtonVisual(*pData);
            }
        }
    }
}

_bool CUI_Processor::HitTest_Rect(const RECT& rcScreen, const POINT& ptMouse) noexcept
{
    return PtInRect(&rcScreen, ptMouse) ? true : false;
}


HRESULT CUI_Processor::Initialize_From_Spec_UIButton(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    return S_OK;
}

HRESULT CUI_Processor::Initialize_From_Spec_UIImage(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    return S_OK;
}

std::unique_ptr<CUI_Processor> CUI_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, CCanvasRenderer_Processor* pCanvasProcessor, CRectTransform_Processor* pRTProcessor)
{
    auto pInstance = std::make_unique<CUI_Processor>(pDevice, pContext, pCanvasProcessor, pRTProcessor);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}

NS_END
