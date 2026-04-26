#pragma once
#include "Eren_Struct.h"

NS_BEGIN(Client)

class CErenTitan;

class CErenSequenceDirector final : public IScript
{
public:
    CErenSequenceDirector();
    ~CErenSequenceDirector() override;

public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;
    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private:
    void Build_DefaultSequence();

    void Enter_CurrentStep();
    _bool Is_CurrentStepFinished();

    void Next_Step();

private:
    /* BORNE -> COMBAT -> PLAY_ANIM -> MOVE_TO -> LIFT_ROCK -> MOVE_ROCK -> FIX_ROCK -> END */
    void Command_Born();
    void Command_Combat();
    void Command_MoveTo();
    void Command_PlayAnim(const _char* pAnimName);
    void Command_LiftRock();
    void Command_MoveRock();
    void Command_FixRock();
    void Command_Ending();

private:
    _bool Check_BornFinished() const;
    _bool Check_CombatFinished() const;
    _bool Check_MoveToFinished() const;
    _bool Check_AnimFinished() const;
    _bool Check_LiftRockFinished() const;
    _bool Check_WalkRockFinished() const;
    _bool Check_FixRockFinished() const;

private:
    class CHUDController*       m_pHUD = nullptr;
    class CUI_NoticeController* m_pNotice = nullptr;

    CErenTitan*                 m_scEren = nullptr;
    CGameObject*                m_goEren = nullptr;
    CTransform                  m_trEren{};
    CAnimator                   m_animEren{};

private:
    std::vector<EREN_DIRECTOR_STEP>   m_vecSteps{};
    _uint                       m_iCurStep = 0;
    _float                      m_fStepElapsed = 0.f;
    _bool                       m_bSequenceEnd = false;

private :
    /* TODO : 에렌_거인_테스트 */
    // const _int                  m_iNumTotalCombatTitans = 2;
    const _int                  m_iNumTotalCombatTitans = 0;

    CCamera                     m_camCinematic{};

private :
    SCRIPT_OBJECT_REF           m_refHUDController{};
    SCRIPT_OBJECT_REF           m_refRock{};
    SCRIPT_OBJECT_REF           m_refCamCinematic{};

    SCRIPT_FIELDS_BEGIN(CErenSequenceDirector)
    SCRIPT_FIELD_OBJECT_REF(m_refHUDController)
    SCRIPT_FIELD_OBJECT_REF(m_refRock)
    SCRIPT_FIELD_OBJECT_REF(m_refCamCinematic)
    SCRIPT_FIELDS_END(CErenSequenceDirector)
    
};

NS_END
