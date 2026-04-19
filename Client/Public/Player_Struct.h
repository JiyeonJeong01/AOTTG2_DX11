#pragma once

#include "Client_Define.h"
#include "Entity_Define.h"

NS_BEGIN(Client)

enum class PLAYER_STATE { IDLE = 0, GROUNDED_MOVE, JUMP, AIRBORNE_MOVE, HOOK, GROUNDED_ATTACK, AIRBORNE_ATTACK, SHOOT, RELOAD, DODGE, RESUPPLY, GRABBED, EMOTE, END };

enum class AIRBORNE_MOVE : uint8_t { AIR_BEGIN, AIR, AIR_LEFT, AIR_RIGHT, AIR_FRONT, AIR_BACK, AIR_FALL, SLIDE_LEFT, SLIDE_RIGHT, SLIDE_FRONT, END };
enum class AIRBORNE_ATTACK : uint8_t { NORMAL, SPIN_H, THROW, SPIN_V, STRONG, END };
enum class GROUNDED_MOVE : uint8_t { RUN, SLIDE, DASH_LAND, END };
enum class GROUNDED_ATTACK : uint8_t { ATK, END };
enum class JUMP : uint8_t { JUMP_BEGIN, RISE, DASH, FALL, END };
enum class RELOAD : uint8_t { GROUNDED, AIR, END };
enum class GRABBED : uint8_t { LEFT, RIGHT, END };


enum class SKILL_TYPE : uint8_t { SPIN_H, THROW, SPIN_V, END };


typedef struct tagPlayerInputCommand
{
    _float2 vMouseDelta{};
    _float3 vMove{};
    _float3 vLook{};

    _bool bLeftAnchorPressed = false;   /* Q */
    _bool bLeftAnchorHeld = false;      /* Q */
    _bool bRightAnchorPressed = false;  /* E */
    _bool bRightAnchorHeld = false;     /* E */

    _bool bBoostPressed = false;        /* SHIFT(jump) */ 
    _bool bBoostHeld = false;           /* SHIFT(jump) */

    _bool bNormalAttackPressed = false; /* LBUTTON */
    _bool bStrongAttackPressed = false; /* RBUTTON */

    _bool bRopeReelHeld = false;        /* SPACE */

    _int iSwitchSkillDir = 0;         /* MOUSE Z */

    _bool bReloadBlade = false;         /* R */
    _bool bInteract = false;            /* F */
    _bool bDodge = false;               /* C */

}PLAYER_INPUT_COMMAND;

typedef struct tagPlayerComponents
{
    CTransform      transform;
    CAnimator       animator;
    CCollider       collider;
    CRigidbody      rigidbody;
    CSpringJoint    springJoint;
    CMeshRenderer   meshRenderer;
} PLAYER_COMPONENTS;

typedef struct tagPlayerStats
{
    _float                      fCurSpeed = 10.f;
    _float                      fMaxSpeed = 13.f;

    _float                      fJump = 5.f;
    _float                      fJumpDashH = 0.03f;
    _float                      fJumpDashV = 0.5f;

    /* 그래플링 */
    _float                      fSpringNormal = 4.f;
    _float                      fDamperNormal = 3.f;
    _float                      fSpringReel = 8.f;
    _float                      fDamperReel = 5.f;

} PLAYER_STATS;

typedef struct tagPlayerRuntimeRef
{
    class CGroundChecker*       pGroundChecker = nullptr;
    class CODM_Gear*            pGear = nullptr;
    class CCameraController*    pCameraController = nullptr;
    class CPlayerStateMachine*  pFSM = nullptr;
    class CTargetSensor*        pSensor = nullptr;
    class CTrail*               pTrail = nullptr;
    std::unordered_map<std::string, class CHitBox*>* pAllHitBoxes = nullptr;
} PLAYER_RUNTIME_REF;

typedef struct tagPlayerSkill
{
    SKILL_TYPE      eSkill = SKILL_TYPE::END;
    _bool           bCoolDownCompleted = true;
    _float          fCoolDown{};
    _float          fElapsedCoolDown{};
    ASSET_GUID      tSpriteGUID{};
    std::string     strName{};
} PLAYER_SKILL;

typedef struct tagPlayerSkillSET
{
    static constexpr _uint      iNumSkills = 3;

    SKILL_TYPE                  eCurSkill = SKILL_TYPE::END;
    PLAYER_SKILL                skills[3];
} PLAYER_SKILLSET;

typedef struct tagBladeDurability
{
    const _int     iNumAtkPerBlade = 8;                /* 칼날 당 최대 공격 횟수 */
    const _int     iTotalNumBlades = 5;                /* 전체 칼날 개수 */

    _int           iCurAtkRemain = iNumAtkPerBlade;    /* 현재 남은 공격 횟수 */
    _int           iCurBladesRemain = iTotalNumBlades; /* 현재 남은 칼날 개수 */

    CGameObject* pLeftBlade = nullptr;
    CGameObject* pRightBlade = nullptr;

    void    Enable_Blades(_bool bEnable)
    {
        if (pLeftBlade)
            pLeftBlade->Set_Enable(bEnable);
        if (pRightBlade)
            pRightBlade->Set_Enable(bEnable);
    }

    _bool   Can_ConsumeBladeAtk()
    {
        return iCurAtkRemain > 0;
    }

    _bool Consume_Blade()
    {
        if (!Can_ConsumeBladeAtk())
            return false;

        --iCurAtkRemain;

        if (iCurAtkRemain <= 0)
            Enable_Blades(false);

        return true;
    }

    _bool Can_ReloadBlade()
    {
        return iCurBladesRemain > 0;
    }

    _bool Reload_Blade()
    {
        if (!Can_ReloadBlade())
            return false;

        --iCurBladesRemain;
        iCurAtkRemain = iNumAtkPerBlade;

        return true;
    }
} BLADE_DURABILITY;

typedef struct tagPlayerContext
{
    /* 포인터 자체를 소유한 구조체들 */
    PLAYER_COMPONENTS       tComponents{};
    PLAYER_RUNTIME_REF      tRef{};

    /* 플레이어가 소유한 변수의 포인터를 가진 구조체들 */
    PLAYER_STATS*           pStats = nullptr;
    BLADE_DURABILITY*       pBlade = nullptr;

    /* 헬퍼 */
    class CPlayer_SkillController*  pSkillController = nullptr;
    class CVFX_Manager*             pVFX_Manager = nullptr;

    class CHitBox* pHitBox = nullptr;

    _float                  fOriginDrag = 0.5f;

} PLAYER_CONTEXT;

inline constexpr const char* PLAYER_BLADE_ATTACK = "Blade_Attack";

NS_END
