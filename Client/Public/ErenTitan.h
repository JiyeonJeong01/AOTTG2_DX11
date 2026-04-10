#pragma once
#include "Eren_Struct.h"
#include "Script.h"

NS_BEGIN(Client)
class CGroundChecker;
class CTargetSensor;
class CHitBox;
NS_END

NS_BEGIN(Client)

class CErenTitan : public IScript
{
public:
    void Awake(void* pCtx) override;
    void Start(void* pCtx) override;

    void Priority_Update(void* pCtx, _float fDT) override;
    void Update(void* pCtx, _float fDT) override;
    void Late_Update(void* pCtx, _float fDT) override;

private :
    /* ----- Utils ----- */
    CGameObject*            m_goEren{};
    CGroundChecker*         m_pGroundChecker{};
    CTargetSensor*          m_pSensor{};
    std::unordered_map<std::string, CHitBox*>   m_AllHitBoxes;

    /* ----- Eren Stats ----- */
    const _float        m_fMaxSpeed = 10.f;

    /* Rotation */
    _float3             m_vPrevLook{};
    _float              m_fRotateSharpness = 0.5f;
    _float              m_fCurrentYaw = 0.f;
    _bool               m_bYawInitialized = false;

    /* ----- Components ----- */
    CTransform          m_trEren{};
    CRigidbody          m_rbEren{};
    CAnimator           m_animEren{};
    _int                m_iNumCombatTitans = 0;

    /* ----- Step Type ----- */
    EREN_STEP_TYPE      m_eStepType = EREN_STEP_TYPE::NONE;

    /* ----- Born ----- */
    EREN_BORN           m_eBorn = EREN_BORN::END;
    _bool               m_bBornCompleted = false;

    /* ----- Combat ------ */
    EREN_COMBAT                         m_eCombat = EREN_COMBAT::END;
    CGameObject*                        m_goLatestCombatTarget{};
    std::queue<CGameObject*>            m_goPendingCombatTarget;
    CTransform                          m_trLastestCombatTarget{};
    std::vector<EREN_COMBAT_PATTERN>    m_CombatPattern;

    _bool                       m_bCombatAttacking = false;
    _int                        m_iCurComboIndex = 0;
    _int                        m_iTotalComboIndex = 0;
    _float                      m_fElapsedAttackInterval = 0.f;
    const _float                m_fCombatSpeed = 4.f;
    const _float                m_fAttackInterval = 1.5f;
    _float                      m_fShouldRunDistanceSq = 400.f;
    _float                      m_fKeepDistanceSq = 0.f;
    _int                        m_iCurCombatCnt = 0;


private :
    void    Move_To(_fvector vDir, _float fDT, _float fSpeed);
    void    Look_To(_fvector vDir, _float fDT);

private :
    void    Process_Born(_float fDT);
    void    Process_Combat(_float fDT);

    void    On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void    On_AnimBornFinished(const _uint iIndex);
    void    On_AnimCombatFinished(const _uint iIndex);

    void    On_DetectedCombatTargets(CGameObject* goTitan);
    void    On_Hurt(const HIT_INFO& tHitBox, const std::string& strHurtBox);
    void    On_SuccessAttack(CGameObject* goTitan);

public :
    void    Set_ErenStep(EREN_STEP_TYPE eType);
    void    Start_Born();
    void    Start_Combat();

    _bool   Is_CombatAttacking() const;
    _bool   Validate_Target();
    void    Start_ComboAttack(EREN_COMBAT eCombat);
    _vector Get_AttackPower();

    _bool   Is_BornCompleted() const;
    _int    Get_CurCombatTitans() const;
    _bool   Is_LiftCompleted() const;
    _bool   Is_FixCompleted() const;

};



NS_END;
