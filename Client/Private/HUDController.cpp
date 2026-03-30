#include "HUDController.h"

#include "UI_BladeController.h"
#include "UI_GasController.h"
#include "UI_SkillController.h"

NS_BEGIN(Client)

void CHUDController::Awake(void* pCtx)
{
    CGameObject* pOwner = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(pOwner, , "pOwner is nullptr");

    m_pBlade = pOwner->Get_Script_InChildren<CUI_BladeController>();
    IF_NULL_RETURN_MSG_BREAK(m_pBlade, , "m_pBlade is nullptr");

    m_pGas = pOwner->Get_Script_InChildren<CUI_GasController>();
    IF_NULL_RETURN_MSG_BREAK(m_pGas, , "m_pGas is nullptr");

    m_pSkill = pOwner->Get_Script_InChildren<CUI_SkillController>();
    IF_NULL_RETURN_MSG_BREAK(m_pSkill, , "m_pSkill is nullptr");
}

void CHUDController::Start(void* pCtx)
{
}

void CHUDController::Priority_Update(void* pCtx, _float fDT)
{
}

void CHUDController::Update(void* pCtx, _float fDT)
{
}

void CHUDController::Late_Update(void* pCtx, _float fDT)
{
}

NS_END;
