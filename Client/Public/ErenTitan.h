#pragma once
#include "Eren_Struct.h"
#include "Script.h"

NS_BEGIN(Engine)
class CNavMesh;
NS_END

NS_BEGIN(Client)
class CGroundChecker;
class CTargetSensor;
class CHitBox;
class CHurtBox;
class CAttacher;
NS_END

NS_BEGIN(Client)

class CErenTitan : public IScript
{
public :
    CErenTitan();
    ~CErenTitan() override;
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
    CAttacher*                          m_scAttach{};

    /* ----- Eren Stats ----- */

    /* TODO : 에렌_거인_테스트 */
    //const _float                        m_fMaxSpeed = 30.f;     /* rigidbody 기반 이동에 대한 제한 */
    //_float                              m_fWalkSpeed = 30.f;     
    //_float                              m_fRunSpeed = 30.f;

    const _float                        m_fMaxSpeed = 14.f;     /* rigidbody 기반 이동에 대한 제한 */
    _float                              m_fWalkSpeed = 11.f;     
    _float                              m_fRunSpeed = 14.f;

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
    _float                              m_fMoveToResumeDist = 25.f;      /* 전투 중 이 거리보다 멀어지면 다시 목표 지점으로 복귀 */
    _float                              m_fShouldAttackDist = 15.5f;
    _bool                               m_bMoveToArrived = false;        /* 목표 지점 도착 여부 */
    std::vector<_float3>                m_vecMovePath;
    _int                                m_iCurMovePathIndex = 0;
    _float                              m_fMovePathReachDist = 4.f;

    /* ----- Lift Rock ----- */
    _bool                               m_bLiftCompleted = false;
    _float                              m_fTotalDelayToLift = 0.5f;
    _float                              m_fElapsedDelayToLift = 0.f;
    _bool                               m_bLiftAnimStarted = false;
    _float                              m_fTotalWaitToAttach = 0.3f;
    _float                              m_fElapsedWaitToAttach = 0.f;

    /* ----- Move Rock ----- */
    _bool                               m_bMoveRockCompleted = false;

    /* ----- Fix Rock ----- */
    _bool                               m_bFixAnimStarted = false;
    _bool                               m_bFixCompleted = false;
    _bool                               m_bReleaseRock = false;
    _float                              m_fElapsedDelayToFix = 0.f;
    _float                              m_fTotalDelayToFix = 0.5f;


    /* ----- Etc ----- */
    CEvent<_float>                      m_OnDamaged;
    _uint                               m_iRunAnimIndex = INVALID_ANIM_CLIP_INDEX;
    _uint                               m_iWalkAnimIndex = INVALID_ANIM_CLIP_INDEX;
    _uint                               m_iLiftAnimIndex = INVALID_ANIM_CLIP_INDEX;
    _uint                               m_iMoveRockAnimIndex = INVALID_ANIM_CLIP_INDEX;
    _uint                               m_iHurtAnimIndex = INVALID_ANIM_CLIP_INDEX;

private :
    void    Move_To(_fvector vDir, _float fDT, _float fSpeed);
    void    Look_To(_fvector vDir, _float fDT);

private :
    void    Process_Born(_float fDT);
    void    Process_Combat(_float fDT);
    void    Process_MoveTo(_float fDT);
    void    Process_Lift(_float fDT);
    void    Process_MoveRock(_float fDT);
    void    Process_FixRock(_float fDT);

    void    On_AnimFinished(const Engine::ANIMATION_EVENT_DATA& tData);
    void    On_AnimBornFinished(const _uint iIndex);
    void    On_AnimCombatFinished(const _uint iIndex);
    void    On_AnimLiftFinished(const _uint iIndex);

    void    On_DetectedCombatTargets(CGameObject* goTitan);
    void    On_Hurt(const HIT_INFO& tHitBox, const std::string& strHurtBox);
    void    On_SuccessAttack(CGameObject* goTitan, const HIT_INFO& tHitInfo);

public :
    void    Set_ErenStep(EREN_STEP_TYPE eType);
    void    Start_Born();
    void    Start_Combat();
    void    Start_MoveTo();
    void    Start_LiftUp();
    void    Start_MoveRock();
    void    Start_FixRock();

    _bool   Is_BornCompleted() const;
    _int    Get_CurCombatTitans() const;
    _bool   Is_MoveToCompleted() const;
    _bool   Is_LiftCompleted() const;
    _bool   Is_MoveRockCompleted() const;
    _bool   Is_FixCompleted() const;

public :
    template <typename T>
    ListenerID Subscribe_OnDamaged(void(T::* func)(_float), T* pInstance)
    {
        return m_OnDamaged.Add_Listener(func, pInstance);
    }


private :
    void    Activate_Hitbox(const std::string& strKey, _bool bActive);
    _bool   Is_CombatAttacking() const;
    _bool   Validate_Target();
    void    Start_ComboAttack(EREN_COMBAT eCombat);
    _vector Get_AttackPower();
    _bool   Is_MovePathPointArrived(const _float3& vCurPos, const _float3& vTargetPos) const;

};



NS_END;
