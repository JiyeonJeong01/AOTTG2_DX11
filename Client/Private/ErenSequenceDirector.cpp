#include "ErenSequenceDirector.h"
#include "AnimationClip_Eren.h"
#include "ErenTitan.h"
#include "HUDController.h"
#include "UI_NoticeController.h"

NS_BEGIN(Client)

CErenSequenceDirector::CErenSequenceDirector()
{
}

CErenSequenceDirector::~CErenSequenceDirector()
{
}

void CErenSequenceDirector::Awake(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    m_goEren = GAME_INSTANCE.Find_GameObject(m_hObject);
    IF_NULL_RETURN_MSG_BREAK(m_goEren, , "m_goEren is nullptr.");

    m_scEren = m_goEren->Get_Script_InChildren<CErenTitan>();
    IF_NULL_RETURN_MSG_BREAK(m_scEren, , "m_scEren is nullptr.");

    m_trEren = m_goEren->Get_Component<CTransform>();
    IF_TRUE_RETURN_MSG_BREAK(m_trEren.Is_Valid() == false, , "m_trEren is invalid.");

    m_animEren = m_goEren->Get_Component<CAnimator>();
    IF_TRUE_RETURN_MSG_BREAK(m_animEren.Is_Valid() == false, , "m_animEren is invalid.");

    CGameObject* goHUD = GAME_INSTANCE.Find_GameObject(m_refHUDController.hObject);
    IF_NULL_RETURN_MSG_BREAK(goHUD, , "goHUD is nullptr.");

    m_pHUD = goHUD->Get_Script<CHUDController>();
    m_pNotice = goHUD->Get_Script_InChildren<CUI_NoticeController>();
    IF_NULL_RETURN_MSG_BREAK(m_pHUD, , "m_pHUD is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(m_pNotice, , "m_pNotice is nullptr.");
}

void CErenSequenceDirector::Start(void* pCtx)
{
    UNREFERENCED_PARAMETER(pCtx);

    Build_DefaultSequence();

    m_iCurStep = 0;
    m_fStepElapsed = 0.f;
    m_bSequenceEnd = m_vecSteps.empty();

    if (m_bSequenceEnd == false)
        Enter_CurrentStep();


    CGameObject* goRock = GAME_INSTANCE.Find_GameObject(m_refRock.hObject);
    CTransform trRock{};
    if (goRock)
    {
        trRock = goRock->Get_Component<CTransform>();
        trRock.Set_Position(XMVectorSet(98.512f, 8.38f, -153.41f, 0.f));
    }
    else
        __debugbreak();
}

void CErenSequenceDirector::Priority_Update(void* pCtx, _float fDT)
{
    IScript::Priority_Update(pCtx, fDT);
}

void CErenSequenceDirector::Late_Update(void* pCtx, _float fDT)
{
    IScript::Late_Update(pCtx, fDT);
}

void CErenSequenceDirector::Update(void* pCtx, _float fDT)
{
    UNREFERENCED_PARAMETER(pCtx);

    if (m_bSequenceEnd)
        return;

    if (m_iCurStep >= m_vecSteps.size())
    {
        m_bSequenceEnd = true;
        return;
    }

    m_fStepElapsed += fDT;

    if (Is_CurrentStepFinished())
        Next_Step();
}

void CErenSequenceDirector::Build_DefaultSequence()
{
    /* BORNE -> COMBAT -> MOVE_TO -> LIFT_ROCK -> MOVE_ROCK -> FIX_ROCK -> END */
    m_vecSteps.clear();

    {
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::BORNE;
            m_vecSteps.push_back(tStep);
        }
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::COMBAT;
            m_vecSteps.push_back(tStep);
        }
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::COMBAT;
            m_vecSteps.push_back(tStep);
        }
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::MOVE_TO;
            m_vecSteps.push_back(tStep);
        }
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::LIFT_ROCK;
            m_vecSteps.push_back(tStep);
        }
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::MOVE_ROCK;
            m_vecSteps.push_back(tStep);
        }
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::FIX_ROCK;
            m_vecSteps.push_back(tStep);
        }
        {
            EREN_DIRECTOR_STEP tStep{};
            tStep.eType = EREN_STEP_TYPE::END;
            m_vecSteps.push_back(tStep);
        }
    }
}

void CErenSequenceDirector::Enter_CurrentStep()
{
    if (m_iCurStep >= m_vecSteps.size())
    {
        m_bSequenceEnd = true;
        return;
    }

    m_fStepElapsed = 0.f;

    const EREN_DIRECTOR_STEP& tStep = m_vecSteps[m_iCurStep];

    switch (tStep.eType)
    {
    case EREN_STEP_TYPE::BORNE:
        Command_Born();
        break;

    case EREN_STEP_TYPE::COMBAT:
        Command_Combat();
        break;

    case EREN_STEP_TYPE::MOVE_TO:
        Command_MoveTo();
        break;

    case EREN_STEP_TYPE::LIFT_ROCK:
        Command_LiftRock();
        break;

    case EREN_STEP_TYPE::MOVE_ROCK:
        Command_MoveRock();
        break;

    case EREN_STEP_TYPE::FIX_ROCK:
        Command_FixRock();
        break;

    case EREN_STEP_TYPE::END:
        Command_Ending();
        m_bSequenceEnd = true;
        break;

    default:
        break;
    }
}

_bool CErenSequenceDirector::Is_CurrentStepFinished()
{
    if (m_iCurStep >= m_vecSteps.size())
        return true;

    const EREN_DIRECTOR_STEP& tStep = m_vecSteps[m_iCurStep];

    switch (tStep.eType)
    {
    case EREN_STEP_TYPE::BORNE:
        return Check_BornFinished();
    case EREN_STEP_TYPE::COMBAT:
        return Check_CombatFinished();
    case EREN_STEP_TYPE::MOVE_TO:
        return Check_MoveToFinished();
    case EREN_STEP_TYPE::LIFT_ROCK:
        return Check_LiftRockFinished();
    case EREN_STEP_TYPE::MOVE_ROCK:
        return Check_WalkRockFinished();
    case EREN_STEP_TYPE::FIX_ROCK:
        return Check_FixRockFinished();
    case EREN_STEP_TYPE::END:
        return true;
    }

    return true;
}

void CErenSequenceDirector::Next_Step()
{
    ++m_iCurStep;

    if (m_iCurStep >= m_vecSteps.size())
    {
        m_bSequenceEnd = true;
        return;
    }

    Enter_CurrentStep();
}

void CErenSequenceDirector::Command_Born()
{
    m_scEren->Start_Born();
}

void CErenSequenceDirector::Command_MoveTo()
{
    if (m_trEren.Is_Valid() == false)
        return;

    m_scEren->Start_MoveTo();
}

void CErenSequenceDirector::Command_PlayAnim(const _char* pAnimName)
{
    if (m_animEren.Is_Valid() == false)
        return;

    UNREFERENCED_PARAMETER(pAnimName);

    /*
       m_animEren->Play_Animation(pAnimName, false);
    */
}

void CErenSequenceDirector::Command_Combat()
{
    m_scEren->Start_Combat();

    if (!m_pNotice)
    {
        __debugbreak();        return;
    }

    m_pNotice->Show_Notice(NOTICE_TYPE::SAVE_EREN, 3.f);
}


_bool CErenSequenceDirector::Check_AnimFinished() const
{
    if (m_animEren.Is_Valid() == false)
        return true;

    /*
       return m_animEren->Is_Finished();
    */

    return true;
}

void CErenSequenceDirector::Command_LiftRock()
{
    m_scEren->Start_LiftUp();
}

void CErenSequenceDirector::Command_MoveRock()
{
    m_scEren->Start_MoveRock();
}

void CErenSequenceDirector::Command_FixRock()
{
    m_scEren->Start_FixRock();
}

void CErenSequenceDirector::Command_Ending()
{
}

_bool CErenSequenceDirector::Check_BornFinished() const
{
    return m_scEren->Is_BornCompleted();
}

_bool CErenSequenceDirector::Check_CombatFinished() const
{
    return m_scEren->Get_CurCombatTitans() >= m_iNumTotalCombatTitans;
}

_bool CErenSequenceDirector::Check_MoveToFinished() const
{
    return m_scEren->Is_MoveToCompleted();
}

_bool CErenSequenceDirector::Check_LiftRockFinished() const
{
    return m_scEren->Is_LiftCompleted();
}

_bool CErenSequenceDirector::Check_WalkRockFinished() const
{
    return m_scEren->Is_MoveRockCompleted();
}

_bool CErenSequenceDirector::Check_FixRockFinished() const
{
    return m_scEren->Is_FixCompleted();
}



NS_END
