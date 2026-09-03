#include "HUDController.h"

#include "ODM_Gear.h"
#include "UI_BladeController.h"
#include "UI_GasController.h"
#include "UI_SkillController.h"
#include "UI_ErenController.h"

#include "Player.h"
#include "CinematicSystem.h"

NS_BEGIN(Client)

void CHUDController::Awake(void* pCtx)
{

}

void CHUDController::Start(void* pCtx)
{
    CGameObject* pOwner = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(pOwner, , "pOwner is nullptr");

    m_pBladeCtrl = pOwner->Get_Script_InChildren<CUI_BladeController>();
    IF_NULL_RETURN_MSG_BREAK(m_pBladeCtrl, , "m_pBladeCtrl is nullptr");

    m_pGasCtrl = pOwner->Get_Script_InChildren<CUI_GasController>();
    IF_NULL_RETURN_MSG_BREAK(m_pGasCtrl, , "m_pGasCtrl is nullptr");

    m_pSkill = pOwner->Get_Script_InChildren<CUI_SkillController>();
    IF_NULL_RETURN_MSG_BREAK(m_pSkill, , "m_pSkill is nullptr");

    m_pErenUI = pOwner->Get_Script_InChildren<CUI_ErenController>();
    IF_NULL_RETURN_MSG_BREAK(m_pErenUI, , "m_pErenUI is nullptr");

    m_goCursor = GAME_INSTANCE.Find_GameObject(m_refCursor.hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goCursor, , "m_goCursor is nullptr");

    m_txtCursor = m_goCursor->Get_Component<CUIText>();
    IF_TRUE_RETURN_MSG_BREAK(!m_txtCursor.Is_Valid(), , "m_txtCursor is invalid");

    m_imgCursor = m_goCursor->Get_Component<CUIImage>();
    IF_TRUE_RETURN_MSG_BREAK(!m_txtCursor.Is_Valid(), , "m_txtCursor is invalid");

    Enable_HUD(false);
}

void CHUDController::Priority_Update(void* pCtx, _float fDT)
{
    if (!m_bInitialized)
    {
        CGameObject* goPlayer = GAME_INSTANCE.Find_GameObject(m_refPlayer.hObject);
        IF_NULL_RETURN_MSG_BREAK(goPlayer, , "goPlayer is nullptr");

        CPlayer* scPlayer = goPlayer->Get_Script_InChildren<CPlayer>();
        IF_NULL_RETURN_MSG_BREAK(scPlayer, , "scPlayer is nullptr");

        const auto& tContext = scPlayer->Get_PlayerContext();

        /* Player::Start의 Build_Context 완료 전에는 다음 프레임에서 다시 시도한다. */
        if (!tContext.tRef.pGear || !tContext.pBlade)
            return;

        m_pGasCtrl->Bind_PlayerContext(tContext);
        m_pBladeCtrl->Bind_PlayerContext(tContext);
        // m_pSkill->Bind_PlayerContext(tContext);

        m_pGear = tContext.tRef.pGear;

        m_bInitialized = true;
    }
}

void CHUDController::Update(void* pCtx, _float fDT)
{
}

void CHUDController::Late_Update(void* pCtx, _float fDT)
{
    const _bool bCinematicPlaying = SYS_CINEMATIC.Is_Playing();
    if (m_bCursorHiddenByCinematic != bCinematicPlaying)
    {
        m_bCursorHiddenByCinematic = bCinematicPlaying;

        if (m_txtCursor.Is_Valid())
            m_txtCursor.Set_Enable(!bCinematicPlaying);

        if (m_imgCursor.Is_Valid())
            m_imgCursor.Set_Enable(!bCinematicPlaying);
    }

    if (bCinematicPlaying)
        return;

    if (!m_txtCursor.Is_Valid())  return;
    if (!m_pGear) return;

    TRY_GRAPPLING_INFO tInfo{};
    if (m_pGear->Detect_GrapplingPoint(tInfo))
    {
        wstring strDist = to_wstring(To<_uint>(tInfo.fDist));
        const size_t iTargetWidth = 4;
        if (strDist.length() < iTargetWidth)
            strDist = wstring((iTargetWidth - strDist.length()) / 2, L' ') + strDist;
        m_txtCursor.Set_Text(strDist);
        m_txtCursor.Set_Color(m_vValidTargetColor);
    }
    else
    {
        m_txtCursor.Set_Text(L"????");
        m_txtCursor.Set_Color(m_vInvalidTargetColor);
    }
}

void CHUDController::Enable_HUD(_bool bEnable)
{
    CGameObject* goBlade = GAME_INSTANCE.Find_GameObject(m_pBladeCtrl->Get_Owner());
    IF_NULL_RETURN_MSG_BREAK(goBlade, , "goBlade is nullptr");
    goBlade->Set_Enable(bEnable);

    CGameObject* goGas = GAME_INSTANCE.Find_GameObject(m_pGasCtrl->Get_Owner());
    IF_NULL_RETURN_MSG_BREAK(goGas, , "goGas is nullptr");
    goGas->Set_Enable(bEnable);

    CGameObject* goSkill = GAME_INSTANCE.Find_GameObject(m_pSkill->Get_Owner());
    IF_NULL_RETURN_MSG_BREAK(goSkill, , "goSkill is nullptr");
    goSkill->Set_Enable(bEnable);

    CGameObject* goErenUI = GAME_INSTANCE.Find_GameObject(m_pErenUI->Get_Owner());
    IF_NULL_RETURN_MSG_BREAK(goErenUI, , "goSkill is nullptr");
    goErenUI->Set_Enable(bEnable);

    if (m_goCursor)
        m_goCursor->Set_Enable(bEnable);
}

NS_END;
