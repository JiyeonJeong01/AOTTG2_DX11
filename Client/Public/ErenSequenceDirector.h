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
    void Command_MoveTo(const _float3& vTargetPos);
    void Command_PlayAnim(const _char* pAnimName);
    void Command_LiftRock();
    void Command_MoveRock(const _float3& vTargetPos);
    void Command_FixRock(const _float3& vTargetPos);
    void Command_Ending();

private:
    _bool Check_BornFinished() const;
    _bool Check_CombatFinished() const;
    _bool Check_MoveToFinished(const _float3& vTargetPos) const;
    _bool Check_AnimFinished() const;
    _bool Check_LiftRockFinished() const;
    _bool Check_WalkRockFinished(const _float3& vTargetPos) const;
    _bool Check_FixRockFinished() const;

private:
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
    _float3                     m_vLiftRockSpot = {};
    _float3                     m_vFixRockSpot = {};
    const _int                  m_iNumTotalCombatTitans = 1;

SCRIPT_FIELDS_BEGIN(CErenSequenceDirector)
    SCRIPT_FIELD_FLOAT3(m_vLiftRockSpot)
    SCRIPT_FIELD_FLOAT3(m_vFixRockSpot)
SCRIPT_FIELDS_END(CErenSequenceDirector)

};

NS_END
