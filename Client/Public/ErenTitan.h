#pragma once
#include "Eren_Struct.h"
#include "Script.h"

NS_BEGIN(Client)
class CGroundChecker;
class CTargetSensor;
class CHitBox;
class CHurtBox;
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
    CGameObject*                        m_goEren{};
    CGroundChecker*                     m_pGroundChecker{};
    CTargetSensor*                      m_pSensor{};
    std::unordered_map<std::string, CHitBox*>   m_AllHitBoxes;
    CHurtBox*                           m_scHurtBox = nullptr;
    _uint                               m_iHurtAnimIndex = INVALID_ANIM_CLIP_INDEX;

    /* ----- Eren Stats ----- */
    const _float                        m_fMaxSpeed = 10.f;     /* rigidbody 기반 이동에 대한 제한 */
    _float                              m_fWalkSpeed = 2.f;     
    _float                              m_fRunSpeed = 4.f;
    const _float                        m_fTotalLife = 100.f;
    _float                              m_fCurLife = m_fTotalLife;

    /* Rotation */
    _float3                             m_vPrevLook{};
    _float                              m_fRotateSharpness = 4.5f;  /* 회전 반응 정도 */
    _float                              m_fCurrentYaw = 0.f;        
    _bool                               m_bYawInitialized = false;

    /* ----- Components ----- */
    CTransform                          m_trEren{};
    CRigidbody                          m_rbEren{};
    CAnimator                           m_animEren{};

    /* ----- Step Type ----- */
    EREN_STEP_TYPE                      m_eStepType = EREN_STEP_TYPE::NONE;

    /* ----- Born ----- */
    EREN_BORN                           m_eBorn = EREN_BORN::END;
    _bool                               m_bBornCompleted = false;

    /* ----- Combat ------ */
    EREN_COMBAT                         m_eCombat = EREN_COMBAT::END;
    CGameObject*                        m_goLatestCombatTarget{};
    std::queue<CGameObject*>            m_goPendingCombatTarget;
    CTransform                          m_trLastestCombatTarget{};
    std::vector<EREN_COMBAT_PATTERN>    m_CombatPattern;

    _bool                               m_bCombatAttacking = false;
    _int                                m_iCurComboIndex = 0;
    _int                                m_iTotalComboIndex = 0;
    _float                              m_fCombatSpeed = 4.f;
    _float                              m_fShouldRunDistance = 13.5f;
    _float                              m_fKeepDistance = 0.f;
    _int                                m_iCurCombatCnt = 0;

    /* ----- Move To ----- */
    _float3                             m_vTargetPos{};
    _float                              m_fMoveToArriveDist = 3.f;       /* 목표 지점 도착 판정 거리 */
    _float                              m_fMoveToResumeDist = 18.f;      /* 전투 중 이 거리보다 멀어지면 다시 목표 지점으로 복귀 */
    _float                              m_fShouldAttackDist = 15.5f;
    _bool                               m_bMoveToArrived = false;        /* 목표 지점 도착 여부 */

    /* ----- Etc ----- */
    CEvent<_float>                      m_OnDamaged;

private :
    void    Move_To(_fvector vDir, _float fDT, _float fSpeed);
    void    Look_To(_fvector vDir, _float fDT);

private :
    void    Process_Born(_float fDT);
    void    Process_Combat(_float fDT);
    void    Process_MoveTo(_float fDT);

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
    void    Start_MoveTo();

    void    Activate_Hitbox(const std::string& strKey, _bool bActive);

    _bool   Is_CombatAttacking() const;
    _bool   Validate_Target();
    void    Start_ComboAttack(EREN_COMBAT eCombat);
    _vector Get_AttackPower();

    _bool   Is_BornCompleted() const;
    _int    Get_CurCombatTitans() const;
    _bool   Is_LiftCompleted() const;
    _bool   Is_FixCompleted() const;

public :
    template <typename T>
    ListenerID Subscribe_OnDamaged(void(T::* func)(_float), T* pInstance)
    {
        return m_OnDamaged.Add_Listener(func, pInstance);
    }


};



NS_END;
