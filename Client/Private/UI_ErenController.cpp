#include "UI_ErenController.h"
#include "ErenTitan.h"

NS_BEGIN(Client)

void CUI_ErenController::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);
}

void CUI_ErenController::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    CGameObject* goEren = GAME_INSTANCE.Find_GameObject(m_refEren.hObject);
    IF_NULL_RETURN_MSG_BREAK(goEren, , "goEren is nullptr");

    m_pErenTitan = goEren->Get_Script_InChildren<CErenTitan>();
    IF_NULL_RETURN_MSG_BREAK(m_pErenTitan, , "m_pErenTitan is nullptr");

    m_pErenTitan->Subscribe_OnDamaged(&CUI_ErenController::On_ErenDamaged, this);

    CGameObject* goLifeFill = GAME_INSTANCE.Find_GameObject(m_refLifeFill.hObject);
    IF_NULL_RETURN_MSG_BREAK(goLifeFill, , "goLifeFill is nullptr");

    CGameObject* goLifeDelayFill = GAME_INSTANCE.Find_GameObject(m_refLifeDelayFill.hObject);
    IF_NULL_RETURN_MSG_BREAK(goLifeDelayFill, , "goLifeDelayFill is nullptr");

    m_crLifeFill = goLifeFill->Get_Component<CCanvasRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_crLifeFill.Is_Valid(), , "m_crLifeFill is invalid");

    m_crLifeDelayFill = goLifeDelayFill->Get_Component<CCanvasRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_crLifeDelayFill.Is_Valid(), , "m_crLifeDelayFill is invalid");

    m_fTargetLife = m_fMaxLife;
    m_fCurrentLife = m_fMaxLife;
    m_fDelayLife = m_fMaxLife;

    m_fCurClipRightPx = 0.f;
    m_fDelayClipRightPx = 0.f;
    m_fDelayElapsed = 0.f;

    Apply_LifeClipRect();
}

void CUI_ErenController::Priority_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CUI_ErenController::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);

    /* 현재 체력 바는 즉시 반영 */
    m_fCurrentLife = m_fTargetLife;

    /* 지연 체력 바는 잠깐 대기 후 부드럽게 따라감 */
    if (m_fDelayLife > m_fTargetLife)
    {
        m_fDelayElapsed += fDT;

        if (m_fDelayElapsed >= m_fDelayWaitTime)
        {
            const _float fDelta = m_fDelayLife - m_fTargetLife;
            _float fT = fDT * m_fDelayLerpSpeed;
            fT = std::clamp(fT, 0.f, 1.f);
            fT = Ease_OutCubic(fT);

            m_fDelayLife = std::lerp(m_fDelayLife, m_fTargetLife, fT);

            /* 거의 다 따라오면 딱 맞춰줌 */
            if (fDelta <= 0.05f || fabs(m_fDelayLife - m_fTargetLife) <= 0.05f)
                m_fDelayLife = m_fTargetLife;
        }
    }
    else
    {
        /** 회복되거나 동기화가 필요할 때는 바로 맞춤 */
        m_fDelayLife = m_fTargetLife;
        m_fDelayElapsed = 0.f;
    }
}

void CUI_ErenController::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    Apply_LifeClipRect();
}

void CUI_ErenController::On_ErenDamaged(_float fCurLife)
{
    m_fTargetLife = std::clamp(fCurLife, 0.f, m_fMaxLife);

    /* 현재 체력 바는 바로 반영 */
    m_fCurrentLife = m_fTargetLife;

    /* 지연 바는 새 데미지를 먹으면 다시 잠깐 대기 */
    m_fDelayElapsed = 0.f;
}

void CUI_ErenController::Apply_LifeClipRect()
{
    if (!m_crLifeFill.Is_Valid() || !m_crLifeDelayFill.Is_Valid())
        return;

    if (m_fMaxLife <= 0.f)
    {
        m_fCurClipRightPx = -m_fTotalSizePx;
        m_fDelayClipRightPx = -m_fTotalSizePx;

        m_crLifeFill.Set_ClipRect(RECT_F{ 0.f, 0.f, m_fCurClipRightPx, 0.f });
        m_crLifeDelayFill.Set_ClipRect(RECT_F{ 0.f, 0.f, m_fDelayClipRightPx, 0.f });
        return;
    }

    _float fCurRatio = m_fCurrentLife / m_fMaxLife;
    _float fDelayRatio = m_fDelayLife / m_fMaxLife;

    fCurRatio = std::clamp(fCurRatio, 0.f, 1.f);
    fDelayRatio = std::clamp(fDelayRatio, 0.f, 1.f);

    /* 오른쪽에서 왼쪽으로 줄어들도록 right 값을 음수로 민다 */
    const _float fCurEmptyRatio = 1.f - fCurRatio;
    const _float fDelayEmptyRatio = 1.f - fDelayRatio;

    m_fCurClipRightPx = -m_fTotalSizePx * fCurEmptyRatio;
    m_fDelayClipRightPx = -m_fTotalSizePx * fDelayEmptyRatio;

    m_crLifeFill.Set_ClipRect(RECT_F{ 0.f, 0.f, m_fCurClipRightPx, 0.f });
    m_crLifeDelayFill.Set_ClipRect(RECT_F{ 0.f, 0.f, m_fDelayClipRightPx, 0.f });
}

_float CUI_ErenController::Ease_OutCubic(_float t) const
{
    t = std::clamp(t, 0.f, 1.f);
    const _float fInv = 1.f - t;
    return 1.f - (fInv * fInv * fInv);
}

NS_END;
