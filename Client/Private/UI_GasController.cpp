#include "UI_GasController.h"
#include "Player_Struct.h"
#include "ODM_Gear.h"

NS_BEGIN(Client)

void CUI_GasController::Awake(void* pCtx)
{
}

void CUI_GasController::Start(void* pCtx)
{
    CGameObject* goLeftGasFill = GAME_INSTANCE.Find_GameObject(m_refLeftGasFill.hObject);
    IF_NULL_RETURN_MSG_BREAK(goLeftGasFill, , "goLeftGasFill is nullptr");
    CGameObject* goRightGasFill = GAME_INSTANCE.Find_GameObject(m_refRightGasFill.hObject);
    IF_NULL_RETURN_MSG_BREAK(goRightGasFill, , "goRightGasFill is nullptr");

    m_crLeftGasFill = goLeftGasFill->Get_Component<CCanvasRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_crLeftGasFill.Is_Valid(), , "r is invalid");
    m_crRightGasFill = goRightGasFill->Get_Component<CCanvasRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_crRightGasFill.Is_Valid(), , "m_crRightGasFill is invalid");

    m_fLeftCurSizePx = m_fRightCurSizePx = m_fTotalSizePx; /* 184.f */ 
}

void CUI_GasController::Priority_Update(void* pCtx, _float fDT)
{
}

void CUI_GasController::Update(void* pCtx, _float fDT)
{
}

void CUI_GasController::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);
    UNREFERENCED_PARAMETER(fDT);

    if (!m_pGear)
        return;

    const auto& tGas = m_pGear->Get_GasState();

    if (!m_crLeftGasFill.Is_Valid() || !m_crRightGasFill.Is_Valid())
        return;

    if (tGas.fMax <= 0.f)
    {
        m_fLeftCurSizePx = m_fTotalSizePx;
        m_fRightCurSizePx = -m_fTotalSizePx;

        m_crLeftGasFill.Set_ClipRect(RECT_F{ m_fLeftCurSizePx, 0.f, 0.f, 0.f });
        m_crRightGasFill.Set_ClipRect(RECT_F{ 0.f, 0.f, m_fRightCurSizePx, 0.f });
        return;
    }

    _float fRatio = tGas.fCurrent / tGas.fMax;
    fRatio = std::clamp(fRatio, 0.f, 1.f);

    const _float fEmptyRatio = 1.f - fRatio;

    m_fLeftCurSizePx = m_fTotalSizePx * fEmptyRatio;
    m_fRightCurSizePx = -m_fTotalSizePx * fEmptyRatio;

    m_crLeftGasFill.Set_ClipRect(RECT_F{ m_fLeftCurSizePx, 0.f, 0.f, 0.f });
    m_crRightGasFill.Set_ClipRect(RECT_F{ 0.f, 0.f, m_fRightCurSizePx, 0.f });
}

void CUI_GasController::Update_GasState()
{

}

void CUI_GasController::Bind_PlayerContext(const PLAYER_CONTEXT& tContext)
{
    m_pGear = tContext.tRef.pGear;
}

NS_END;
