#include "UI_BladeController.h"
#include "Player_Struct.h"
#include "GameObject.h"
#include "CanvasRenderer.h"

NS_BEGIN(Client)

void CUI_BladeController::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);
}

void CUI_BladeController::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    CGameObject* goAtkRemainLeft = GAME_INSTANCE.Find_GameObject(m_refAtkRemainLeft.hObject);
    IF_NULL_RETURN_MSG_BREAK(goAtkRemainLeft, , "goAtkRemainLeft is nullptr");

    CGameObject* goAtkRemainRight = GAME_INSTANCE.Find_GameObject(m_refAtkRemainRight.hObject);
    IF_NULL_RETURN_MSG_BREAK(goAtkRemainRight, , "goAtkRemainRight is nullptr");

    m_crAtkRemainLeft = goAtkRemainLeft->Get_Component<CCanvasRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_crAtkRemainLeft.Is_Valid(), , "m_crAtkRemainLeft is invalid");

    m_crAtkRemainRight = goAtkRemainRight->Get_Component<CCanvasRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_crAtkRemainRight.Is_Valid(), , "m_crAtkRemainRight is invalid");

    for (_int i = 0; i < 8; ++i)
    {
        CGameObject* goBladeRemain = GAME_INSTANCE.Find_GameObject(m_refBladeRemain[i].hObject);
        IF_NULL_RETURN_MSG_BREAK(goBladeRemain, , "goBladeRemain is nullptr");

        m_crRemain[i] = goBladeRemain->Get_Component<CCanvasRenderer>();
        IF_TRUE_RETURN_MSG_BREAK(!m_crRemain[i].Is_Valid(), , "m_crRemain[i] is invalid");
    }
}

void CUI_BladeController::Priority_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CUI_BladeController::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);
}

void CUI_BladeController::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    if (!m_pBlade)
        return;

    if (!m_crAtkRemainLeft.Is_Valid() || !m_crAtkRemainRight.Is_Valid())
        return;

    constexpr _float fTotalSizePx = 173.f;

    /* -------- 현재 칼날 공격 횟수 -------- */
    _float fRatio = 0.f;
    if (m_pBlade->iNumAtkPerBlade > 0)
        fRatio = static_cast<_float>(m_pBlade->iCurAtkRemain) / static_cast<_float>(m_pBlade->iNumAtkPerBlade);

    fRatio = std::clamp(fRatio, 0.f, 1.f);

    const _float fEmptyRatio = 1.f - fRatio;
    const _float fLeftClip = fTotalSizePx * fEmptyRatio;
    const _float fRightClip = -fTotalSizePx * fEmptyRatio;

    m_crAtkRemainLeft.Set_ClipRect(RECT_F{ fLeftClip, 0.f, 0.f, 0.f });
    m_crAtkRemainRight.Set_ClipRect(RECT_F{ 0.f, 0.f, fRightClip, 0.f });

    /* -------- 예비 칼날 개수 -------- */
    _int iReserveBladeCount = m_pBlade->iCurBladesRemain - 1;
    iReserveBladeCount = std::clamp(iReserveBladeCount, 0, 4);

    for (_int i = 0; i < 8; ++i)
    {
        if (!m_crRemain[i].Is_Valid())
            continue;

        m_crRemain[i].Set_Enable(false);
    }

    for (_int i = 0; i < iReserveBladeCount; ++i)
    {
        const _int iLeftIndex = 3 - i;
        const _int iRightIndex = 4 + i;

        if (m_crRemain[iLeftIndex].Is_Valid())
            m_crRemain[iLeftIndex].Set_Enable(true);

        if (m_crRemain[iRightIndex].Is_Valid())
            m_crRemain[iRightIndex].Set_Enable(true);
    }
}

void CUI_BladeController::Bind_PlayerContext(const PLAYER_CONTEXT& tContext)
{
    m_pBlade = tContext.pBlade;
}

NS_END
