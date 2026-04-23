#include "UI_HitController.h"
#include "Easing_Function.h"

NS_BEGIN(Client)

void CUI_HitController::Awake(void* pCtx)
{
    Set_References();

    Set_BloodVisible(false);
    Set_BloodAlpha(0, 0.f);
    Set_BloodAlpha(1, 0.f);
    Set_BloodAlpha(2, 0.f);

    Set_KillTextVisible(false);
    Set_KillTextAlpha(0.f);
    Reset_KillTextPosition();
}

void CUI_HitController::Start(void* pCtx)
{
}

void CUI_HitController::Priority_Update(void* pCtx, _float fDT)
{

}

void CUI_HitController::Update(void* pCtx, _float fDT)
{
    Update_BloodEffect(fDT);
    Update_KillTextEffect(fDT);
}

void CUI_HitController::Late_Update(void* pCtx, _float fDT)
{
}

void CUI_HitController::On_PlayerHitTitan()
{
    _float fAlpha0 = 0.f;
    _float fAlpha1 = 0.f;
    _float fAlpha2 = 0.f;

    if (m_crBloodImage0.Is_Valid())
        fAlpha0 = m_crBloodImage0->vColor.w;

    if (m_crBloodImage1.Is_Valid())
        fAlpha1 = m_crBloodImage1->vColor.w;

    if (m_crBloodImage2.Is_Valid())
        fAlpha2 = m_crBloodImage2->vColor.w;

    const _float fCurrentMaxAlpha = std::fmaxf(fAlpha0, std::fmaxf(fAlpha1, fAlpha2));

    Set_BloodVisible(true);

    /* alpha가 0인 경우에만 다시 0부터 페이드 시작 */
    if (fCurrentMaxAlpha <= 0.001f)
    {
        m_fBloodElapsed = 0.f;

        Set_BloodAlpha(0, 0.f);
        Set_BloodAlpha(1, 0.f);
        Set_BloodAlpha(2, 0.f);
    }
    /* alpha가 0이 아닌 경우, 다시 1부터 0으로 페이드 시작 */
    else 
    {
        m_fBloodElapsed = 0.f;

        if (fAlpha0 > 0.001f)
            Set_BloodAlpha(0, 1.f);
        if (fAlpha1 > 0.001f)
            Set_BloodAlpha(1, 1.f);
        if (fAlpha2 > 0.001f)
            Set_BloodAlpha(2, 1.f);
    }

    m_bBloodPlaying = true;
}

void CUI_HitController::On_PlayerKillTitan(_float fAccuracyScore)
{
    const std::wstring strText = Build_AccuracyText(fAccuracyScore);

    Set_KillTextString(strText);
    Reset_KillTextPosition();
    Set_KillTextAlpha(0.f);
    Set_KillTextVisible(true);

    m_fKillTextElapsed = 0.f;
    m_bKillTextPlaying = true;
}

void CUI_HitController::Set_References()
{
    /* goBlood : Image Component를 찾는다. */
    m_goBloodImage0 = GAME_INSTANCE.Find_GameObject(m_refBloodImage0.hObject);
    if (m_goBloodImage0)
        m_crBloodImage0 = m_goBloodImage0->Get_Component<CCanvasRenderer>();

    m_goBloodImage1 = GAME_INSTANCE.Find_GameObject(m_refBloodImage1.hObject);
    if (m_goBloodImage1)
        m_crBloodImage1 = m_goBloodImage1->Get_Component<CCanvasRenderer>();

    m_goBloodImage2 = GAME_INSTANCE.Find_GameObject(m_refBloodImage2.hObject);
    if (m_goBloodImage2)
        m_crBloodImage2 = m_goBloodImage2->Get_Component<CCanvasRenderer>();

    /* goDamageText : CanvasRenderer, CUIText, CRectTransform Component를 찾는다. */
    m_goDamageText0 = GAME_INSTANCE.Find_GameObject(m_refDamageText0.hObject);
    if (m_goDamageText0)
    {
        m_txtDamageText0 = m_goDamageText0->Get_Component<CUIText>();
        m_rtDamageText0 = m_goDamageText0->Get_Component<CRectTransform>();

        m_vDamageTextColor0 = m_txtDamageText0->color;
    }

    m_goDamageText1 = GAME_INSTANCE.Find_GameObject(m_refDamageText1.hObject);
    if (m_goDamageText1)
    {
        m_txtDamageText1 = m_goDamageText1->Get_Component<CUIText>();
        m_rtDamageText1 = m_goDamageText1->Get_Component<CRectTransform>();
        m_vDamageTextColor1 = m_txtDamageText1->color;
    }

    DEBUG_POINT;
}

void CUI_HitController::Set_BloodVisible(_bool bVisible)
{
    if (m_goBloodImage0)
        m_goBloodImage0->Set_Enable(bVisible);

    if (m_goBloodImage1)
        m_goBloodImage1->Set_Enable(bVisible);

    if (m_goBloodImage2)
        m_goBloodImage2->Set_Enable(bVisible);
}

void CUI_HitController::Set_BloodAlpha(_uint iIndex, _float fAlpha)
{
    fAlpha = CEasingFunction::Clamp01(fAlpha);

    const _float4 vColor = { 1.f, 1.f, 1.f, fAlpha };

    switch (iIndex)
    {
    case 0:
        if (m_crBloodImage0.Is_Valid())
            m_crBloodImage0.Set_Color(vColor);
        break;

    case 1:
        if (m_crBloodImage1.Is_Valid())
            m_crBloodImage1.Set_Color(vColor);
        break;

    case 2:
        if (m_crBloodImage2.Is_Valid())
            m_crBloodImage2.Set_Color(vColor);
        break;

    default:
        break;
    }
}

void CUI_HitController::Update_BloodEffect(_float fDT)
{
    if (!m_bBloodPlaying)
        return;

    m_fBloodElapsed += fDT;

    /* 차례대로 등장하고 한 번에 사라지게 함 */
    /* start : 등장 효과가 가능한 시간 */
    const _float fStart0 = 0.f;
    const _float fStart1 = m_fBloodSequenceInterval;
    const _float fStart2 = m_fBloodSequenceInterval * 2.f;

    /* 유지 시간 + 페이드 시간 */
    const _float fPlusEndTime = m_fBloodHoldTime + m_fBloodFadeDuration; 

    const _float fEnd0 = fStart0 + fPlusEndTime;
    const _float fEnd1 = fStart1 + fPlusEndTime;
    const _float fEnd2 = fStart2 + fPlusEndTime;

    auto ComputeAlpha = [&](const _float fElapsed, const _float fStartTime) -> _float
        {
            if (fElapsed < fStartTime)
                return 0.f;

        /* 해당 이미지가 실제로 등장하기 시작한 시간 */
            const _float fLocalTime = fElapsed - fStartTime;

        /* 등장 ~ m_fBloodHoldTime 시간까지는 alpha = 1 */
            if (fLocalTime <= m_fBloodHoldTime)
                return 1.f;

            const _float fFadeTime = fLocalTime - m_fBloodHoldTime;
            const _float t = CEasingFunction::Clamp01(fFadeTime / m_fBloodFadeDuration);
            return 1.f - t;
        };

    Set_BloodAlpha(0, ComputeAlpha(m_fBloodElapsed, fStart0));
    Set_BloodAlpha(1, ComputeAlpha(m_fBloodElapsed, fStart1));
    Set_BloodAlpha(2, ComputeAlpha(m_fBloodElapsed, fStart2));

    /* 마지막 이미지 off */
    if (m_fBloodElapsed >= fEnd2)
    {
        m_bBloodPlaying = false;
        m_fBloodElapsed = 0.f;

        Set_BloodAlpha(0, 0.f);
        Set_BloodAlpha(1, 0.f);
        Set_BloodAlpha(2, 0.f);
        Set_BloodVisible(false);
    }
}

void CUI_HitController::Set_KillTextVisible(_bool bVisible)
{
    if (m_goDamageText0)
        m_goDamageText0->Set_Enable(bVisible);

    if (m_goDamageText1)
        m_goDamageText1->Set_Enable(bVisible);
}

void CUI_HitController::Set_KillTextAlpha(_float fAlpha)
{
    fAlpha = CEasingFunction::Clamp01(fAlpha);

    _float4 vColor0 = m_vDamageTextColor0;
    _float4 vColor1 = m_vDamageTextColor1;

    vColor0.w = vColor1.w = fAlpha;

    if (m_txtDamageText0.Is_Valid())
        m_txtDamageText0.Set_Color(vColor0);

    if (m_txtDamageText1.Is_Valid())
        m_txtDamageText1.Set_Color(vColor1);
}

void CUI_HitController::Set_KillTextString(const std::wstring& strText)
{
    if (m_txtDamageText0.Is_Valid())
        m_txtDamageText0.Set_Text(strText);

    if (m_txtDamageText1.Is_Valid())
        m_txtDamageText1.Set_Text(strText);
}

void CUI_HitController::Reset_KillTextPosition()
{
    if (m_rtDamageText0.Is_Valid())
        m_rtDamageText0.Set_PositionPx(m_vDamageText0StartLocalPos.x, m_vDamageText0StartLocalPos.y);

    if (m_rtDamageText1.Is_Valid())
        m_rtDamageText1.Set_PositionPx(m_vDamageText1StartLocalPos.x, m_vDamageText1StartLocalPos.y);
}

void CUI_HitController::Update_KillTextEffect(_float fDT)
{
    if (!m_bKillTextPlaying)
        return;

    m_fKillTextElapsed += fDT;


    const _float fPhase0End = m_fKillTextRiseInDuration;
    const _float fPhase1End = fPhase0End + m_fKillTextHoldDuration;
    const _float fPhase2End = fPhase1End + m_fKillTextFadeOutDuration;

    _float fAlpha = 0.f;

    _float2 vOffset0 = { 0.f, 0.f };
    _float2 vOffset1 = { 0.f, 0.f };

    /* phase0 : 올라오며 등장 */
    if (m_fKillTextElapsed <= fPhase0End)
    {
        const _float t = CEasingFunction::Clamp01(m_fKillTextElapsed / m_fKillTextRiseInDuration);
        const _float eased = CEasingFunction::EaseOutCubic(t);

        fAlpha = eased;
        vOffset0.x = CEasingFunction::Lerp(0.f, m_vDamageTextRiseOffset.x, eased);
        vOffset0.y = CEasingFunction::Lerp(0.f, m_vDamageTextRiseOffset.y, eased);

        vOffset1 = vOffset0;
    }
    /* phase1 : 제자리 유지 */
    else if (m_fKillTextElapsed <= fPhase1End)
    {
        fAlpha = 1.f;
        vOffset0 = m_vDamageTextRiseOffset;
        vOffset1 = m_vDamageTextRiseOffset;
    }
    /* phase2 : 페이드 하며 사라지기 */
    else if (m_fKillTextElapsed <= fPhase2End)
    {
        const _float t = CEasingFunction::Clamp01((m_fKillTextElapsed - fPhase1End) / m_fKillTextFadeOutDuration);
        const _float eased = CEasingFunction::EaseOutCubic(t);

        fAlpha = 1.f - eased;

        vOffset0.x = m_vDamageTextRiseOffset.x + CEasingFunction::Lerp(0.f, m_vDamageTextFadeOffset.x, eased);
        vOffset0.y = m_vDamageTextRiseOffset.y + CEasingFunction::Lerp(0.f, m_vDamageTextFadeOffset.y, eased);

        vOffset1 = vOffset0;
    }
    else
    {
        m_bKillTextPlaying = false;
        m_fKillTextElapsed = 0.f;

        Set_KillTextAlpha(0.f);
        Reset_KillTextPosition();
        Set_KillTextVisible(false);
        return;
    }

    if (m_rtDamageText0.Is_Valid())
    {
        _float2 vPos = m_vDamageText0StartLocalPos;
        vPos.x += vOffset0.x;
        vPos.y += vOffset0.y;
        m_rtDamageText0.Set_PositionPx(vPos.x, vPos.y);
    }

    if (m_rtDamageText1.Is_Valid())
    {
        _float2 vPos = m_vDamageText1StartLocalPos;
        vPos.x += vOffset1.x;
        vPos.y += vOffset1.y;
        m_rtDamageText1.Set_PositionPx(vPos.x, vPos.y);
    }

    Set_KillTextAlpha(fAlpha);
}

std::wstring CUI_HitController::Build_AccuracyText(_float fAccuracyScore) const
{
    _float fScore = CEasingFunction::Clamp01(fAccuracyScore) * 1000.f;

    wchar_t szText[64] = {};
    swprintf_s(szText, L"%.0f", fScore);

    return std::wstring(szText);
}

NS_END;
