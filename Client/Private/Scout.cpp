#include "Scout.h"
#include "ScoutBehavior.h"
#include "Scout_Scriptable_Object.h"
#include "ScoutBehavior_RequestResupply.h"
#include "HUDController.h"
#include "UI_NoticeController.h"

NS_BEGIN(Client)

void CScout::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    m_goOwner = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goOwner, , "m_goOwner is nullptr");

    m_trOwner = m_goOwner->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(!m_trOwner.Is_Valid(), , "m_trOwner is invalid");

    m_cldrOwner = m_goOwner->Get_Component<CCollider>();
    IF_TRUE_RETURN_MSG_BREAK(!m_cldrOwner.Is_Valid(), , "m_cldrOwner is invalid");

    m_animOwner = m_goOwner->Get_Component<CAnimator>();
    IF_TRUE_RETURN_MSG_BREAK(!m_animOwner.Is_Valid(), , "m_animOwner is invalid");

    m_mrOwner = m_goOwner->Get_Component<CMeshRenderer>();
    IF_TRUE_RETURN_MSG_BREAK(!m_mrOwner.Is_Valid(), , "m_mrOwner is invalid");


    CGameObject* goHUD = GAME_INSTANCE.Find_GameObject(m_refHUDController.hObject);
    IF_NULL_RETURN_MSG_BREAK(goHUD, , "goHUD is nullptr.");
    m_pHUD = goHUD->Get_Script<CHUDController>();
    m_pNotice = goHUD->Get_Script_InChildren<CUI_NoticeController>();
    IF_NULL_RETURN_MSG_BREAK(m_pHUD, , "m_pHUD is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_pNotice, , "m_pNotice is nullptr.");

}

void CScout::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    SetUp_Context();
    SetUp_Behavior();
}

void CScout::Priority_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);

    if (SYS_INPUT.Get_KeyDown(VK_F1))
    {
        To<CScoutBehavior_RequestResupply*>(m_pBehavior)->Process_Start();
    }

    if (m_pBehavior)
        m_pBehavior->Priority_Update(fDT);
}

void CScout::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);

    if (m_pBehavior)
        m_pBehavior->Update(fDT);
}

void CScout::Late_Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);

    if (m_pBehavior)
        m_pBehavior->Late_Update(fDT);
}

void CScout::Move(_fvector vDir, _float fDT)
{
    UNREFERENCED_PARAMETER(vDir);
    UNREFERENCED_PARAMETER(fDT);
}

void CScout::SetUp_Context()
{
    m_tContext.eBehaviour = SCOUT_BEHAVIOR::NONE;
    m_tContext.tComponents.transform = m_trOwner;
    m_tContext.tComponents.animator = m_animOwner;
    m_tContext.tComponents.collider = m_cldrOwner;
    m_tContext.tComponents.meshRenderer = m_mrOwner;
    m_tContext.pStat = &m_tStats;
}

void CScout::SetUp_Behavior()
{
    Clear_Behavior();

    auto* scScoutSO = m_goOwner->Get_Script<CScout_Scriptable_Object>();
    IF_NULL_RETURN_MSG_BREAK(scScoutSO, , "scScoutSO is nullptr");

    const SCOUT_BEHAVIOR eBehavior = scScoutSO->Get_Behavior();
    m_tContext.eBehaviour = eBehavior;
    CGameObject* pStaging = nullptr;
    CGameObject* pCinematic = nullptr;

    CGameObject* pFade = nullptr;
    CGameObject* pDialogue = nullptr;

    switch (eBehavior)
    {
    case SCOUT_BEHAVIOR::REQUEST_RESUPPLY:
        m_pBehavior = new CScoutBehavior_RequestResupply(m_goOwner, this, eBehavior);
        pStaging = GAME_INSTANCE.Find_GameObject(m_refStagingCamera.hObject);
        pCinematic = GAME_INSTANCE.Find_GameObject(m_refCinematicCamera.hObject);

        pFade = GAME_INSTANCE.Find_GameObject(m_refFadeUI.hObject);
        pDialogue = GAME_INSTANCE.Find_GameObject(m_refDialogueUI.hObject);

        To< CScoutBehavior_RequestResupply*>(m_pBehavior)->Set_SpecialCamera(pStaging, pCinematic);
        To< CScoutBehavior_RequestResupply*>(m_pBehavior)->Set_UI(m_pNotice, pDialogue, pFade);

        break;

    case SCOUT_BEHAVIOR::NONE:
    default:
        m_pBehavior = nullptr;
        break;
    }

    if (m_pBehavior == nullptr)
        return;

    m_pBehavior->Cache_ScoutContext(m_tContext);
    m_pBehavior->SetUp_CachedScoutContext();
    m_pBehavior->Initialize();
}

void CScout::Clear_Behavior()
{
    if (m_pBehavior == nullptr)
        return;

    delete m_pBehavior;
    m_pBehavior = nullptr;
}

void CScout::Set_ActCase()
{
}

void CScout::On_Grabbed(SIDE eSide, CTitan* pTitan)
{
    UNREFERENCED_PARAMETER(eSide);
    UNREFERENCED_PARAMETER(pTitan);
}

NS_END
