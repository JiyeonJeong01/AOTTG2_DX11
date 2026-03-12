#include "MainMenu_Controller.h"
#include "Easing_Function.h"
#include "GameObject.h"
#include "Input_System.h"
#include "GameObject.h"

NS_BEGIN(Client)

void CMainMenu_Controller::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    for (_uint i = 0; i < 3; ++i)
    {
        m_fBtnCurScale[i] = 1.f;
        m_fBtnTargetScale[i] = 1.f;
        m_bBtnHover[i] = false;
    }

    m_vParallaxTarget = _float2(0.f, 0.f);
    m_vParallaxCurrent = _float2(0.f, 0.f);
    m_bCached = false;
}

void CMainMenu_Controller::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);
}

void CMainMenu_Controller::Priority_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    if (m_bCached)
        return;

    if (!m_refBtns[0].Is_Valid() || !m_refBtns[1].Is_Valid() || !m_refBtns[2].Is_Valid())
        return;

    if (!m_refImgBackground.Is_Valid() || !m_refImgPanel.Is_Valid())
        return;

    CGameObject* pBtn0 = SYS_GAMEOBJECT.Get_Wrapper(m_refBtns[0].hObject);
    CGameObject* pBtn1 = SYS_GAMEOBJECT.Get_Wrapper(m_refBtns[1].hObject);
    CGameObject* pBtn2 = SYS_GAMEOBJECT.Get_Wrapper(m_refBtns[2].hObject);
    CGameObject* pBg = SYS_GAMEOBJECT.Get_Wrapper(m_refImgBackground.hObject);
    CGameObject* pPanel = SYS_GAMEOBJECT.Get_Wrapper(m_refImgPanel.hObject);

    if (!pBtn0 || !pBtn1 || !pBtn2 || !pBg || !pPanel)
        return;

    m_Btns[0] = pBtn0->Get_Component<CUIButton>();
    m_BtnRTs[0] = pBtn0->Get_Component<CRectTransform>();

    m_Btns[1] = pBtn1->Get_Component<CUIButton>();
    m_BtnRTs[1] = pBtn1->Get_Component<CRectTransform>();

    m_Btns[2] = pBtn2->Get_Component<CUIButton>();
    m_BtnRTs[2] = pBtn2->Get_Component<CRectTransform>();

    m_ImgBackground = pBg->Get_Component<CUIImage>();
    m_RTBackground = pBg->Get_Component<CRectTransform>();

    m_ImgPanel = pPanel->Get_Component<CUIImage>();
    m_RTPanel = pPanel->Get_Component<CRectTransform>();

    if (!m_Btns[0].Is_Valid() || !m_Btns[1].Is_Valid() || !m_Btns[2].Is_Valid())
        return;

    if (!m_BtnRTs[0].Is_Valid() || !m_BtnRTs[1].Is_Valid() || !m_BtnRTs[2].Is_Valid())
        return;

    if (m_Btns[0].Is_Valid())
        m_Btns[0].OnHover().Add_Listener<CMainMenu_Controller>(&CMainMenu_Controller::On_Hover_Btn0, this);

    if (m_Btns[1].Is_Valid())
        m_Btns[1].OnHover().Add_Listener<CMainMenu_Controller>(&CMainMenu_Controller::On_Hover_Btn1, this);

    if (m_Btns[2].Is_Valid())
        m_Btns[2].OnHover().Add_Listener<CMainMenu_Controller>(&CMainMenu_Controller::On_Hover_Btn2, this);

    Cache_Base_UI_State();
    m_bCached = true;
}

void CMainMenu_Controller::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);

    On_Mouse_Move();

    Update_Parallax(fDT);
    Update_Button_Animations(fDT);
}

void CMainMenu_Controller::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    for (_uint i = 0; i < 3; ++i)
    {
        m_bBtnHover[i] = false;
        m_fBtnTargetScale[i] = 1.f;
    }
}

void CMainMenu_Controller::On_Hover_Btn0(Engine::BUTTON_EVENT_DATA& eData)
{
    UNREFERENCED_PARAMETER(eData);
    Set_BtnHover(0, true);
}

void CMainMenu_Controller::On_Hover_Btn1(Engine::BUTTON_EVENT_DATA& eData)
{
    UNREFERENCED_PARAMETER(eData);
    Set_BtnHover(1, true);
}

void CMainMenu_Controller::On_Hover_Btn2(Engine::BUTTON_EVENT_DATA& eData)
{
    UNREFERENCED_PARAMETER(eData);
    Set_BtnHover(2, true);
}

void CMainMenu_Controller::Set_BtnHover(_uint iIndex, _bool bHover)
{
    if (iIndex >= 3)
        return;

    m_bBtnHover[iIndex] = bHover;

    if (bHover)
        m_fBtnTargetScale[iIndex] = m_fBtnEffectScale;
    else
        m_fBtnTargetScale[iIndex] = 1.f;
}

void CMainMenu_Controller::On_Mouse_Move()
{
    long iMove = 0;
    _float2 vDelta{ 0.f, 0.f };

    if (iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::VERTICAL))
        vDelta.y = To<_float>(iMove);

    if (iMove = SYS_INPUT.Get_DIMouseMove(MOUSE_MOVE_AXIS::HORIZONTAL))
        vDelta.x = To<_float>(iMove);

    m_vParallaxTarget.x += vDelta.x * m_fParallaxDeltaScale;
    m_vParallaxTarget.y += vDelta.y * m_fParallaxDeltaScale;

    const _float fMaxX = 40.f;
    const _float fMaxY = 24.f;

    if (m_vParallaxTarget.x > fMaxX) m_vParallaxTarget.x = fMaxX;
    if (m_vParallaxTarget.x < -fMaxX) m_vParallaxTarget.x = -fMaxX;
    if (m_vParallaxTarget.y > fMaxY) m_vParallaxTarget.y = fMaxY;
    if (m_vParallaxTarget.y < -fMaxY) m_vParallaxTarget.y = -fMaxY;
}

void CMainMenu_Controller::Cache_Base_UI_State()
{
    for (_uint i = 0; i < 3; ++i)
    {
        if (m_BtnRTs[i].Is_Valid())
        {
            m_vBaseBtnPos[i] = m_BtnRTs[i].Get_PositionPx();
            m_vBaseBtnSize[i] = m_BtnRTs[i].Get_SizePx();
        }
    }

    if (m_RTBackground.Is_Valid())
        m_vBaseBackgroundPos = m_RTBackground.Get_PositionPx();

    if (m_RTPanel.Is_Valid())
        m_vBasePanelPos = m_RTPanel.Get_PositionPx();
}

void CMainMenu_Controller::Update_Parallax(_float fDT)
{
    m_vParallaxTarget = CEasingFunction::DampedLerp(
        m_vParallaxTarget,
        _float2(0.f, 0.f),
        m_fParallaxReturnSharpness,
        fDT
    );

    m_vParallaxCurrent = CEasingFunction::DampedLerp(
        m_vParallaxCurrent,
        m_vParallaxTarget,
        m_fParallaxFollowSharpness,
        fDT
    );

    if (m_RTBackground.Is_Valid())
    {
        _float2 vPos = _float2(
            m_vBaseBackgroundPos.x + m_vParallaxCurrent.x * m_fBackgroundParallaxStrength,
            m_vBaseBackgroundPos.y + m_vParallaxCurrent.y * m_fBackgroundParallaxStrength
        );

        if (vPos.x > m_vBaseBackgroundPos.x + m_vBackgroundMoveBound.x)
            vPos.x = m_vBaseBackgroundPos.x + m_vBackgroundMoveBound.x;
        if (vPos.x < m_vBaseBackgroundPos.x - m_vBackgroundMoveBound.x)
            vPos.x = m_vBaseBackgroundPos.x - m_vBackgroundMoveBound.x;

        if (vPos.y > m_vBaseBackgroundPos.y + m_vBackgroundMoveBound.y)
            vPos.y = m_vBaseBackgroundPos.y + m_vBackgroundMoveBound.y;
        if (vPos.y < m_vBaseBackgroundPos.y - m_vBackgroundMoveBound.y)
            vPos.y = m_vBaseBackgroundPos.y - m_vBackgroundMoveBound.y;

        m_RTBackground.Set_PositionPx(vPos.x, vPos.y);
    }

    if (m_RTPanel.Is_Valid())
    {
        _float2 vPos = _float2(
            m_vBasePanelPos.x + m_vParallaxCurrent.x * m_fPanelParallaxStrength,
            m_vBasePanelPos.y + m_vParallaxCurrent.y * m_fPanelParallaxStrength
        );

        if (vPos.x > m_vBasePanelPos.x + m_vPanelMoveBound.x)
            vPos.x = m_vBasePanelPos.x + m_vPanelMoveBound.x;
        if (vPos.x < m_vBasePanelPos.x - m_vPanelMoveBound.x)
            vPos.x = m_vBasePanelPos.x - m_vPanelMoveBound.x;

        if (vPos.y > m_vBasePanelPos.y + m_vPanelMoveBound.y)
            vPos.y = m_vBasePanelPos.y + m_vPanelMoveBound.y;
        if (vPos.y < m_vBasePanelPos.y - m_vPanelMoveBound.y)
            vPos.y = m_vBasePanelPos.y - m_vPanelMoveBound.y;

        m_RTPanel.Set_PositionPx(vPos.x, vPos.y);
    }
}

void CMainMenu_Controller::Update_Button_Animations(_float fDT)
{
    for (_uint i = 0; i < 3; ++i)
    {
        if (!m_BtnRTs[i].Is_Valid())
            continue;

        _float fTargetScale = 1.f;

        if (m_bBtnHover[i])
            fTargetScale = m_fBtnEffectScale;

        m_fBtnTargetScale[i] = fTargetScale;

        m_fBtnCurScale[i] = CEasingFunction::DampedLerp(
            m_fBtnCurScale[i],
            m_fBtnTargetScale[i],
            m_fBtnScaleSharpness,
            fDT
        );

        const _float2 vBaseSize = m_vBaseBtnSize[i];
        const _float fScale = m_fBtnCurScale[i];

        m_BtnRTs[i].Set_SizePx(
            vBaseSize.x * fScale,
            vBaseSize.y * fScale
        );
    }
}

NS_END
