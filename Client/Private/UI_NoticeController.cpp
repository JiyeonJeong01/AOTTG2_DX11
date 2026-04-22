#include "UI_NoticeController.h"

#include "GameInstance.h"
#include "Easing_Function.h"

NS_BEGIN(Client)

CUI_NoticeController::CUI_NoticeController()
{
}

void CUI_NoticeController::Awake(void* pCtx)
{
    m_pNoticeRootObject = GAME_INSTANCE.Find_GameObject(m_refNoticeRoot.hObject);
    if (m_pNoticeRootObject)
    {
        m_rtNotice = m_pNoticeRootObject->Get_Component<CRectTransform>();
        m_txtNotice = m_pNoticeRootObject->Get_Component<CUIText>();
    }

    Initialize_NoticeText();

    Set_UIPosition(m_vHiddenPos);
    Set_UIActive(false);

    m_eState = NOTICE_STATE::IDLE;
    m_eCurNoticeType = NOTICE_TYPE::NONE;
    m_fStateTime = 0.f;
    m_fHoldTime = 0.f;
    m_bPlaying = false;
}

void CUI_NoticeController::Start(void* pCtx)
{
}

void CUI_NoticeController::Priority_Update(void* pCtx, _float fDT)
{
}

void CUI_NoticeController::Update(void* pCtx, _float fDT)
{
    if (!m_bPlaying)
        return;

    Update_NoticeState(fDT);
}

void CUI_NoticeController::Late_Update(void* pCtx, _float fDT)
{
}

void CUI_NoticeController::Show_Notice(NOTICE_TYPE eType, _float fDelayTime)
{
    if (!m_pNoticeRootObject || !m_rtNotice.Is_Valid() || !m_txtNotice.Is_Valid())
        return;

    if (eType == NOTICE_TYPE::NONE || eType >= NOTICE_TYPE::END)
        return;

    m_eCurNoticeType = eType;
    m_fHoldTime = fmaxf(0.f, fDelayTime);
    m_fStateTime = 0.f;
    m_eState = NOTICE_STATE::ENTER;
    m_bPlaying = true;

    Set_NoticeText(eType);
    Set_UIPosition(m_vHiddenPos);
    Set_UIActive(true);
}

void CUI_NoticeController::Initialize_NoticeText()
{
    m_strNotice.clear();
    m_strNotice.resize(To<_uint>(NOTICE_TYPE::END));

    m_strNotice[To<_uint>(NOTICE_TYPE::NONE)] = L"";
    m_strNotice[To<_uint>(NOTICE_TYPE::SAVE_EREN)] = L"에렌을 엄호하세요";
    m_strNotice[To<_uint>(NOTICE_TYPE::REQUEST_RESUPPLY)] = L"동료에게 가스를 보급하세요";
}

void CUI_NoticeController::Set_NoticeText(NOTICE_TYPE eType)
{
    if (!m_txtNotice.Is_Valid())
        return;

    const _uint iIndex = To<_uint>(eType);
    if (iIndex >= m_strNotice.size())
        return;

    m_txtNotice.Set_Text(m_strNotice[iIndex]);
}

void CUI_NoticeController::Set_UIActive(_bool bEnable)
{
    if (m_pNoticeRootObject)
        m_pNoticeRootObject->Set_Enable(bEnable);
}

void CUI_NoticeController::Set_UIPosition(const _float2& vPos)
{
    if (!m_rtNotice.Is_Valid())
        return;

    m_rtNotice.Set_PositionPx(vPos.x, vPos.y);
}

void CUI_NoticeController::Update_NoticeState(_float fDT)
{
    m_fStateTime += fDT;

    switch (m_eState)
    {
    case NOTICE_STATE::IDLE:
        break;

    case NOTICE_STATE::ENTER:
    {
        const _float fDuration = fmaxf(0.0001f, m_fEnterDuration);
        const _float t = fminf(1.f, m_fStateTime / fDuration);
        const _float fEase = CEasingFunction::EaseOutCubic(t);

        Set_UIPosition(CEasingFunction::Lerp(m_vHiddenPos, m_vShownPos, fEase));

        if (t >= 1.f)
        {
            m_eState = NOTICE_STATE::HOLD;
            m_fStateTime = 0.f;
            Set_UIPosition(m_vShownPos);
        }
        break;
    }

    case NOTICE_STATE::HOLD:
    {
        if (m_fStateTime >= m_fHoldTime)
        {
            m_eState = NOTICE_STATE::EXIT;
            m_fStateTime = 0.f;
        }
        break;
    }

    case NOTICE_STATE::EXIT:
    {
        const _float fDuration = fmaxf(0.0001f, m_fExitDuration);
        const _float t = fminf(1.f, m_fStateTime / fDuration);
        const _float fEase = CEasingFunction::EaseOutCubic(t);

        Set_UIPosition(CEasingFunction::Lerp(m_vShownPos, m_vHiddenPos, fEase));

        if (t >= 1.f)
        {
            m_eState = NOTICE_STATE::IDLE;
            m_eCurNoticeType = NOTICE_TYPE::NONE;
            m_fStateTime = 0.f;
            m_fHoldTime = 0.f;
            m_bPlaying = false;

            Set_UIPosition(m_vHiddenPos);
            Set_UIActive(false);
        }
        break;
    }

    default:
        break;
    }
}

NS_END
