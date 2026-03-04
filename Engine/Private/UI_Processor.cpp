#include "UI_Processor.h"

#include "Component_System.h"
#include "Resource_System.h"
#include "CanvasRenderer_Processor.h"
#include "RectTransform_Processor.h" 

#include "UIButton.h"
#include "UIImage.h"
//#include "UIText.h" 나중에 추가

#include "BuiltIn_GUID.h"
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

CUI_Processor::CUI_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent_Processor()
    , m_pCanvasProcessor(nullptr), m_pRectTransformProcessor(nullptr)
{
}

CUI_Processor::~CUI_Processor() = default;

HRESULT CUI_Processor::Initialize()
{
    //SYS_COMPONENT.Register_InitialSpecFactory<CUIImage, UIButtonSpec>(COMPONENT_TYPE::UI_BUTTON); /* TODO 만든 뒤 등록하기  
    //SYS_COMPONENT.Register_InitialSpecFactory<CUIImage, UIImageSpec>(COMPONENT_TYPE::UI_IMAGE);
    SYS_COMPONENT.Register_BuildSpecFacotry<CUIButton>(COMPONENT_TYPE::UI_BUTTON);
    SYS_COMPONENT.Register_BuildSpecFacotry<CUIImage>(COMPONENT_TYPE::UI_IMAGE);


    CComponent_Processor* pBase = nullptr;
    SYS_COMPONENT.Bind_ComponentProcessor(COMPONENT_TYPE::RECT_TRANSFORM, &pBase);
    IF_NULL_RETURN_MSG_BREAK(pBase, E_FAIL, "RectTransform processor bind failed");
    m_pRectTransformProcessor = SCAST(CRectTransform_Processor*, pBase);

    pBase = nullptr;
    SYS_COMPONENT.Bind_ComponentProcessor(COMPONENT_TYPE::CANVAS_RENDERER, &pBase);
    IF_NULL_RETURN_MSG_BREAK(pBase, E_FAIL, "Canvas Render processor bind failed");
    m_pCanvasProcessor = SCAST(CCanvasRenderer_Processor*, pBase);

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

std::unique_ptr<COMPONENT_SPEC_BASE> CUI_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    return nullptr;
}

void CUI_Processor::Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::UI_IMAGE:
        Set_Enable_Inner<CUIImage>(m_ImagePool, hComponent, bEnable);
        return;

    case COMPONENT_TYPE::UI_BUTTON:
        Set_Enable_Inner<CUIButton>(m_ButtonPool, hComponent, bEnable);
        return;

    default:
        break;
    }

    _DEBUG_WARN("CUI_Processor::Remove_Component - unsupported component type");
}

void* CUI_Processor::Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept
{
    switch (eComType)
    {
    case COMPONENT_TYPE::UI_IMAGE:  return m_ImagePool.Get_Data_By_Handle(hComponent);
    case COMPONENT_TYPE::UI_BUTTON: return m_ButtonPool.Get_Data_By_Handle(hComponent);
    default: return nullptr;
    }
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
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->dirty || !pData->bEnable)
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
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable) continue;

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

HRESULT CUI_Processor::Initialize_Component_Data(COMPONENT_TYPE eComType, COMPONENT_HANDLE h)
{
    void* pRaw = Get_DataPtr(eComType, h);
    IF_NULL_RETURN_MSG_BREAK(pRaw, E_FAIL, "Initialize_Component_Data failed: Get_DataPtr returned null");

    if (eComType == COMPONENT_TYPE::UI_IMAGE)
    {
        auto* pData = SCAST(UI_IMAGE_DATA*, pRaw);

        pData->hTexture = SYS_RESOURCE.Load_Texture(DefaultAssetGuid::TEXTURE_UI_DEFAULT);

        CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Initialize_Component_Data(UI_IMAGE) failed: invalid hObject");

        /* CanvasRenderer 보장 및 캐싱 */
        {
            auto cr = pObj->Get_Component<CCanvasRenderer>();
            COMPONENT_HANDLE hCR = cr.Get_Handle();

            if (!hCR.Is_Valid()) 
            {
                cr = pObj->Add_Component<CCanvasRenderer>();
                cr.Set_Texture(pData->hTexture);
                hCR = cr.Get_Handle();
                
                IF_TRUE_RETURN_MSG_BREAK(!hCR.Is_Valid(), E_FAIL, "Initialize_Component_Data(UI_IMAGE) failed: add CanvasRenderer failed");
            }

            pData->hCanvasRenderer = hCR;
        }

        /* 기본 값 추가 */
        pData->bEnable = true;
        pData->dirty = true;
        // pData->visualPriority = 0;

        return S_OK;
    }

    if (eComType == COMPONENT_TYPE::UI_BUTTON)
    {
        auto* pData = SCAST(UI_BUTTON_DATA*, pRaw);

        CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Initialize_Component_Data(UI_BUTTON) failed: invalid hObject");

        // 1) RectTransform 확보 (없으면 추가)
        {
            auto rt = pObj->Get_Component<CRectTransform>();
            COMPONENT_HANDLE hRT = rt.Get_Handle();

            if (!hRT.Is_Valid())
            {
                rt = pObj->Add_Component<CRectTransform>();
                hRT = rt.Get_Handle();

                IF_TRUE_RETURN_MSG_BREAK(!hRT.Is_Valid(), E_FAIL, "Initialize_Component_Data(UI_BUTTON) failed: add RectTransform failed");
            }

            pData->hRectTransform = hRT;
        }

        // 2) Button이 실제로 때릴 CanvasRenderer 확보 (없으면 추가)
        //    - 보통 버튼은 "자기 이미지(CanvasRenderer)"를 타겟으로 씁니다.
        {
            auto cr = pObj->Get_Component<CCanvasRenderer>();
            COMPONENT_HANDLE hCR = cr.Get_Handle();

            if (!hCR.Is_Valid())
            {
                cr = pObj->Add_Component<CCanvasRenderer>();
                hCR = cr.Get_Handle();

                IF_TRUE_RETURN_MSG_BREAK(!hCR.Is_Valid(), E_FAIL, "Initialize_Component_Data(UI_BUTTON) failed: add CanvasRenderer failed");
            }

            // 타겟 캔버스가 비어있으면 기본으로 자기 CanvasRenderer를 타겟으로
            if (!pData->hTargetCanvas.Is_Valid())
                pData->hTargetCanvas = hCR;
        }

        // 3) 기본 상태값
        pData->bEnable = true;
        pData->eState = UI_BTN_STATE::Normal;
        pData->bInteractable = true;

        // 4) 비주얼 우선순위(버튼이 더 높게 덮어쓰게)
        // pData->visualPriority = 10;

        // 5) 최초 1회 비주얼 적용(선택)
        Apply_ButtonVisual(*pData);

        return S_OK;
    }

    return E_FAIL;
}

std::unique_ptr<CUI_Processor> CUI_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    auto pInstance = std::make_unique<CUI_Processor>(pDevice, pContext);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}

NS_END
