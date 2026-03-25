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
#include "Component_Spec.h"
#include "magic_enum.hpp"

NS_BEGIN(Engine)
    namespace /* --- 내부 Utils --- */
{
    static inline _float4 Get_State_Color(const UI_BUTTON_DATA& bData)
    {
        /* 버튼 색 변화 */
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
        /* 버튼 텍스쳐 변화 */
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
}


CUI_Processor::CUI_Processor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CComponent_Processor()
    , m_pCanvasProcessor(nullptr), m_pRectTransformProcessor(nullptr)
{
}

CUI_Processor::~CUI_Processor() = default;

HRESULT CUI_Processor::Initialize()
{
    /* 팩토리 등록 */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CUIButton, UI_BUTTON_SPEC>();
        SYS_COMPONENT.Register_InitialSpecFactory<CUIImage, UI_IMAGE_SPEC>();

        SYS_COMPONENT.Register_BuildSpecFacotry<CUIButton>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CUIImage>();

        SYS_COMPONENT.Register_InitialSpecFactory<CUIText, UI_TEXT_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CUIText>();
    }

    /* 프로세서 바인딩 */
    m_pRectTransformProcessor = SYS_COMPONENT.Bind_Processor<CRectTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pRectTransformProcessor, E_FAIL, "RectTransform processor bind failed");

    m_pCanvasProcessor = SYS_COMPONENT.Bind_Processor<CCanvasRenderer_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pCanvasProcessor, E_FAIL, "Canvas Render processor bind failed");

    return S_OK;
}

void CUI_Processor::Update(_float fDT)
{
    Sync_Images_To_Canvas();
    Update_Buttons(fDT);     
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

/* 텍스트 */
void CUI_Processor::Build_RenderQueue(std::vector<DRAW_CMD>& outCmds)
{
    const auto& Pages = m_TextPool.GetPages();

    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            if (pData->hFont == INVALID_HANDLE_UINT)
                continue;

            DRAW_CMD tCmd = DRAW_CMD::Create_Text(
                pData->hFont,
                pData->hRectTransform,
                pData->flags,
                pData->sortZ,
                pData->color,
                pData->fScale,
                pData->visualPriority,
                &pData->strText,
                pData->rcClip
            );

            tCmd.sortKey = Make_Text_SortKey(*pData);
            tCmd.eLayer = RENDER_LAYER::UI;

            outCmds.push_back(tCmd);
        }
    }
}

COMPONENT_HANDLE CUI_Processor::Create_Component_Data(COMPONENT_TYPE type, OBJECT_HANDLE hObject)
{
    switch (type)
    {
    case COMPONENT_TYPE::UI_BUTTON:
        return Create_Component_Data_Inner<CUIButton>(m_ButtonPool, hObject);
    case COMPONENT_TYPE::UI_IMAGE:
        return Create_Component_Data_Inner<CUIImage>(m_ImagePool, hObject);
    case COMPONENT_TYPE::UI_TEXT:
        return Create_Component_Data_Inner<CUIText>(m_TextPool, hObject);
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
    case COMPONENT_TYPE::UI_TEXT:
        m_TextPool.Deallocate(h);
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
    case COMPONENT_TYPE::UI_TEXT:
        return Initialize_From_Spec_UIText(hComponent, pSpec);
    }

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CUI_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::UI_IMAGE:
    {
        UI_IMAGE_DATA* pData = m_ImagePool.Get_Data_By_Handle(hComponent);
        IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "pData is nullptr");

        UI_IMAGE_SPEC spec{};
        spec.bEnable = pData->bEnable;

        /* Handle -> GUID */
        const TEXTURE_ENTRY* pTextureEntry = SYS_RESOURCE.Get_Texture(pData->hTexture);
        spec.textureGuid = pTextureEntry ? pTextureEntry->tGUID : DEFAULT_ASSET_GUID::TEXTURE_UI_DEFAULT;

        spec.rcUV = pData->rcUV;
        spec.color = pData->color;
        spec.visualPriority = pData->visualPriority;

        return std::make_unique<UI_IMAGE_SPEC>(spec);
    }

    case COMPONENT_TYPE::UI_BUTTON:
    {
        UI_BUTTON_DATA* pData = m_ButtonPool.Get_Data_By_Handle(hComponent);
        IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "pData is nullptr");

        UI_BUTTON_SPEC spec{};
        spec.bEnable = pData->bEnable;
        spec.bInteractable = pData->bInteractable;

        spec.normal = pData->normal;
        spec.hover = pData->hover;
        spec.pressed = pData->pressed;
        spec.disabled = pData->disabled;

        /* Handle -> GUID (저장) */
        const TEXTURE_ENTRY* pEntry = nullptr;
        pEntry = SYS_RESOURCE.Get_Texture(pData->normalTex);
        spec.normalTexGuid = pEntry ? SYS_RESOURCE.Get_Texture(pData->normalTex)->tGUID : DEFAULT_ASSET_GUID::TEXTURE_UI_DEFAULT;
        spec.normalUV = pData->normalUV;

        pEntry = SYS_RESOURCE.Get_Texture(pData->hoverTex);
        spec.hoverTexGuid = pEntry ? pEntry->tGUID : DEFAULT_ASSET_GUID::TEXTURE_UI_DEFAULT;
        spec.hoverUV = pData->hoverUV;

        pEntry = SYS_RESOURCE.Get_Texture(pData->pressedTex);
        spec.pressedTexGuid = pEntry ? pEntry->tGUID : DEFAULT_ASSET_GUID::TEXTURE_UI_DEFAULT;
        spec.pressedUV = pData->pressedUV;

        spec.visualPriority = pData->visualPriority;

        return std::make_unique<UI_BUTTON_SPEC>(spec);
    }
    case COMPONENT_TYPE::UI_TEXT:
    {
        UI_TEXT_DATA* pData = m_TextPool.Get_Data_By_Handle(hComponent);
        IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "pData is nullptr");

        UI_TEXT_SPEC spec{};
        spec.bEnable = pData->bEnable;

        /* Handle -> GUID (저장) */
        const FONT_ENTRY* pEntry = SYS_RESOURCE.Get_Font(pData->hFont);
        spec.fontGuid = pEntry ? pEntry->tGUID : DEFAULT_ASSET_GUID::FONT_UI_DEFAULT;

        spec.strText = pData->strText;
        spec.color = pData->color;
        spec.fScale = pData->fScale;
        spec.visualPriority = pData->visualPriority;
        spec.flags = pData->flags;
        spec.sortZ = pData->sortZ;
        spec.rcClip = pData->rcClip;

        return std::make_unique<UI_TEXT_SPEC>(spec);
    }
    default:
        break;
    }

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

    case COMPONENT_TYPE::UI_TEXT:
        Set_Enable_Inner<CUIText>(m_TextPool, hComponent, bEnable);
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
    case COMPONENT_TYPE::UI_IMAGE:      return m_ImagePool.Get_Data_By_Handle(hComponent);
    case COMPONENT_TYPE::UI_BUTTON:     return m_ButtonPool.Get_Data_By_Handle(hComponent);
    case COMPONENT_TYPE::UI_TEXT:       return m_TextPool.Get_Data_By_Handle(hComponent);
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
            /* 캔버스 렌더러가 참조 중인 텍스쳐 등 정보 변경 */
            pCR->hTexture = pData->hTexture;
            pCR->rcUV = pData->rcUV;
            pCR->vColor = pData->color;
            pCR->visualPriority = pData->visualPriority;

            pData->dirty = false;
        }
    }
}

void CUI_Processor::Apply_ButtonVisual(const UI_BUTTON_DATA& tData)
{
    auto crProxy = m_pCanvasProcessor->Get_Proxy(COMPONENT_TYPE::CANVAS_RENDERER, tData.hTargetCanvas);
    IF_TRUE_RETURN_MSG_BREAK(crProxy.Is_Valid() == false, , "UIButton is not valid");

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
        pCR->visualPriority = tData.visualPriority;
    }
}

void CUI_Processor::Update_Buttons(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    const POINT tMousePos = SYS_INPUT.Get_GameMousePos();
    if (tMousePos.x < 0 || tMousePos.y < 0)
        return;
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

            /* 상호작용이 꺼진 버튼 */
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

            /* 마우스 오버 감지 */
            const _bool bHit = HitTest_Rect(rcBound, tMousePos);

            UI_BTN_STATE ePrev = pData->eState;
            UI_BTN_STATE eNext = ePrev;

            if (!bHit) /* 마우스 오버가 아닌 경우 */
            {
                eNext = UI_BTN_STATE::Normal;
            }
            else  /* 마우스 오버된 경우 */
            {
                eNext = bMouseDown ? UI_BTN_STATE::Pressed : UI_BTN_STATE::Hover;
                CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);

                if (ePrev == UI_BTN_STATE::Normal && eNext == UI_BTN_STATE::Hover && pObj) /* 오버 */
                {
                    BUTTON_EVENT_DATA onHover{ pObj };
                    pData->OnHover.Invoke(onHover);
                }
                if (bMouseDown && eNext == UI_BTN_STATE::Pressed && pObj) /* 클릭 */
                {
                    BUTTON_EVENT_DATA onClick{ pObj };
                    pData->OnClick.Invoke(onClick);
                    eNext = UI_BTN_STATE::Hover;
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

uint64_t CUI_Processor::Make_Text_SortKey(const UI_TEXT_DATA& tData) const
{
    uint64_t key = 0;

    const uint64_t layer = (uint64_t)((uint8_t)RENDER_LAYER::UI & 0xF);

    _float z = tData.sortZ;
    if (z < 0.f) z = 0.f;
    if (z > 1.f) z = 1.f;
    const uint64_t zq = (uint64_t)(z * 65535.f + 0.5f);

    const uint64_t priority = (uint64_t)(tData.visualPriority & 0xFF);
    const uint64_t font = (uint64_t)(tData.hFont & 0xFFFFF);

    key |= (layer << 60);
    key |= (zq << 44);
    key |= (priority << 36);
    key |= (font << 16);

    return key;
}

_bool CUI_Processor::HitTest_Rect(const RECT& rcScreen, const POINT& ptMouse) noexcept
{
    return PtInRect(&rcScreen, ptMouse) ? true : false;
}

HRESULT CUI_Processor::Initialize_From_Spec_UIButton(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_NULL_RETURN_MSG_BREAK(pSpec, E_FAIL, "pSpec is nulptr");

    const UI_BUTTON_SPEC* p = static_cast<const UI_BUTTON_SPEC*>(pSpec);

    UI_BUTTON_DATA* pData = m_ButtonPool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "pData is nulptr");

    pData->bEnable = p->bEnable;
    pData->bInteractable = p->bInteractable;

    pData->normal = p->normal;
    pData->hover = p->hover;
    pData->pressed = p->pressed;
    pData->disabled = p->disabled;

    /* GUID -> Handle */
    pData->normalTex = SYS_RESOURCE.Load_Texture(p->normalTexGuid);
    pData->normalUV = p->normalUV;

    pData->hoverTex = SYS_RESOURCE.Load_Texture(p->hoverTexGuid);
    pData->hoverUV = p->hoverUV;

    pData->pressedTex = SYS_RESOURCE.Load_Texture(p->pressedTexGuid);
    pData->pressedUV = p->pressedUV;

    pData->visualPriority = p->visualPriority;

    pData->eState = UI_BTN_STATE::Normal;

    return S_OK;
}

HRESULT CUI_Processor::Initialize_From_Spec_UIImage(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_NULL_RETURN_MSG_BREAK(pSpec, E_FAIL, "pSpec is nulptr");

    const UI_IMAGE_SPEC* p = static_cast<const UI_IMAGE_SPEC*>(pSpec);

    UI_IMAGE_DATA* pData = m_ImagePool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "pData is nulptr");

    pData->bEnable = p->bEnable;

    /* GUID -> Handle */
    pData->hTexture = SYS_RESOURCE.Load_Texture(p->textureGuid);

    pData->rcUV = p->rcUV;
    pData->color = p->color;
    pData->visualPriority = p->visualPriority;

    pData->dirty = true;

    return S_OK;
}

HRESULT CUI_Processor::Initialize_From_Spec_UIText(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_NULL_RETURN_MSG_BREAK(pSpec, E_FAIL, "pSpec is nulptr");

    const UI_TEXT_SPEC* p = static_cast<const UI_TEXT_SPEC*>(pSpec);

    UI_TEXT_DATA* pData = m_TextPool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "pData is nulptr");

    pData->bEnable = p->bEnable;
    pData->hFont = SYS_RESOURCE.Load_Font(p->fontGuid);
    pData->strText = p->strText;
    pData->color = p->color;
    pData->fScale = p->fScale;
    pData->visualPriority = p->visualPriority;
    pData->dirty = true;

    pData->flags = p->flags;
    pData->sortZ = p->sortZ;
    pData->rcClip = p->rcClip;

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

        /* RectTransform 보장 */
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

        /* CanvasRenderer 보장 및 캐싱 */
        {
            auto cr = pObj->Get_Component<CCanvasRenderer>();
            COMPONENT_HANDLE hCR = cr.Get_Handle();

            if (!hCR.Is_Valid())
            {
                cr = pObj->Add_Component<CCanvasRenderer>();
                hCR = cr.Get_Handle();

                IF_TRUE_RETURN_MSG_BREAK(!hCR.Is_Valid(), E_FAIL, "Initialize_Component_Data(UI_BUTTON) failed: add CanvasRenderer failed");
            }

            /* 타겟 캔버스가 비어있으면 기본으로 자기 CanvasRenderer를 타겟으로 */
            if (!pData->hTargetCanvas.Is_Valid())
                pData->hTargetCanvas = hCR;
        }

        /* 기본 상태값 */
        pData->bEnable = true;
        pData->eState = UI_BTN_STATE::Normal;
        pData->bInteractable = true;

        // pData->visualPriority = 10;
        Apply_ButtonVisual(*pData);

        return S_OK;
    }

    if (eComType == COMPONENT_TYPE::UI_TEXT)
    {
        auto* pData = SCAST(UI_TEXT_DATA*, pRaw);

        CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Initialize_Component_Data(UI_TEXT) failed: invalid hObject");

        /* RectTransform 보장 */
        {
            auto rt = pObj->Get_Component<CRectTransform>();
            COMPONENT_HANDLE hRT = rt.Get_Handle();

            if (!hRT.Is_Valid())
            {
                rt = pObj->Add_Component<CRectTransform>();
                hRT = rt.Get_Handle();
                IF_TRUE_RETURN_MSG_BREAK(!hRT.Is_Valid(), E_FAIL, "Initialize_Component_Data(UI_TEXT) failed: add RectTransform failed");
            }
            pData->hRectTransform = hRT;
        }

        pData->bEnable = true;
        pData->fScale = 1.f;
        pData->color = _float4{ 1.f, 1.f, 1.f, 1.f };
        pData->dirty = true;

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
