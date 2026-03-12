#pragma once
#include "Client_Define.h"
#include "Script.h"
#include "UIButton.h"
#include "UIImage.h"
#include "RectTransform.h"
#include "BUTTON_EVENT_DATA.h"

NS_BEGIN(Client)

class CMainMenu_Controller : public IScript
{
public:
    _float              m_fBtnEffectScale = 1.8f;                               /* 마우스 오버 시 버튼 스케일 목표 */
    SCRIPT_OBJECT_REF   m_refBtns[3];
    SCRIPT_OBJECT_REF   m_refImgBackground;
    SCRIPT_OBJECT_REF   m_refImgPanel;
                                                                                /* p : parallax */
    _float              m_fBackgroundParallaxStrength = 2.0f;                   /* strength : p를 얼마나 크게 반영할지 정하는 배수 */
    _float              m_fPanelParallaxStrength = 0.45f;                       
    _float              m_fParallaxDeltaScale = 0.25f;                          /* 마우스 이동 delta를 p 목표 값으로 변화하는 민감도 */
    _float              m_fParallaxReturnSharpness = 3.5f;                      /* p 목표값이 원점으로 되돌아가는 속도 */
    _float              m_fParallaxFollowSharpness = 8.0f;                      /* 실제 출력 값이 목표 p를 따라가는 속도 */

    _float              m_fBtnScaleSharpness = 3.0f;
    _float              m_fBtnPopExtra = 0.08f;

    _float2             m_vBackgroundMoveBound = _float2(800, 500.f);
    _float2             m_vPanelMoveBound = _float2(500.f, 200.f);
    _bool               m_bCached = false;

public:
    SCRIPT_FIELDS_BEGIN(CMainMenu_Controller)
        SCRIPT_FIELD_FLOAT(m_fBtnEffectScale)
        SCRIPT_FIELD_OBJECT_REF(m_refBtns[0])
        SCRIPT_FIELD_OBJECT_REF(m_refBtns[1])
        SCRIPT_FIELD_OBJECT_REF(m_refBtns[2])
        SCRIPT_FIELD_OBJECT_REF(m_refImgBackground)
        SCRIPT_FIELD_OBJECT_REF(m_refImgPanel)
    SCRIPT_FIELDS_END(CMainMenu_Controller)

private:
    CUIButton       m_Btns[3];
    CRectTransform  m_BtnRTs[3];

    CUIImage        m_ImgBackground;
    CUIImage        m_ImgPanel;
    CRectTransform  m_RTBackground;
    CRectTransform  m_RTPanel;

private:
    _float2         m_vBaseBtnPos[3]{};
    _float2         m_vBaseBtnSize[3]{};
    _float          m_fBtnCurScale[3]{ 1.f, 1.f, 1.f };
    _float          m_fBtnTargetScale[3]{ 1.f, 1.f, 1.f };
    _bool           m_bBtnHover[3]{ false, false, false };

    _float2         m_vBaseBackgroundPos{};
    _float2         m_vBasePanelPos{};

    _float2         m_vParallaxTarget{};
    _float2         m_vParallaxCurrent{};

private:
    void On_Hover_Btn0(Engine::BUTTON_EVENT_DATA& eData);
    void On_Hover_Btn1(Engine::BUTTON_EVENT_DATA& eData);
    void On_Hover_Btn2(Engine::BUTTON_EVENT_DATA& eData);

private:
    void On_Mouse_Move();

private:
    void Cache_Base_UI_State();
    void Update_Parallax(_float fDT);
    void Update_Button_Animations(_float fDT);
    void Set_BtnHover(_uint iIndex, _bool bHover);

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;
};

NS_END;
