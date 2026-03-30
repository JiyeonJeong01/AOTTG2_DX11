#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

enum class PLAYER_STATE { IDLE = 0, GROUNDED_MOVE, JUMP, AIRBORNE_MOVE, HOOK, GROUNDED_ATTACK, AIRBORNE_ATTACK, SHOOT, RELOAD, DODGE, RESUPPLY, GRABBED, EMOTE, END };

enum class AIRBORNE_STATE : uint8_t { AIR_BEGIN, AIR_LEFT, AIR_RIGHT, AIR_FRONT, AIR_BACK, AIR_FALL, END };
enum class GROUNDED_MOVE : uint8_t { RUN, SLIDE, DASH_LAND, END };
enum class JUMP : uint8_t { JUMP_BEGIN, RISE, FALL, END };



enum class SKILL : uint8_t { SPIN_H, THROW, SPIN_V, END };


typedef struct tagPlayerInputCommand
{
    _float2 vMouseDelta{};
    _float3 vMove{};
    _float3 vLook{};

    _bool bLeftAnchorPressed = false;   /* Q */
    _bool bLeftAnchorHeld = false;      /* Q */
    _bool bRightAnchorPressed = false;  /* E */
    _bool bRightAnchorHeld = false;     /* E */

    _bool bBoostPressed = false;        /* SHIFT */
    _bool bBoostHeld = false;           /* SHIFT */

    _bool bNormalAttackPressed = false; /* LBUTTON */
    _bool bStrongAttackPressed = false; /* RBUTTON */

    _bool bRopeReelHeld = false;        /* SPACE */

    _int iSwitchSkillDir = 0;         /* MOUSE Z */

    _bool bReloadBlade = false;         /* R */
    _bool bInteract = false;            /* F */

    /* TODO ::::::::::::::::::::::::::::::::: 확장 필수 ::::::::::::::::::::::::::::::::: */

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

    _float                      fJump = 12.f;
} PLAYER_STATS;

typedef struct tagPlayerRuntimeRef
{
    class CGroundChecker*       pGroundChecker = nullptr;
    class CODM_Gear*            pGear = nullptr;
    class CCameraController*    pCameraController = nullptr;
    class CPlayerStateMachine*  pFSM = nullptr;
} PLAYER_RUNTIME_REF;

typedef struct tagPlayerSkill
{
    SKILL           eSkill = SKILL::END;
    _float          fCoolDown{};
    _float          fElapsedCoolDown{};
    ASSET_GUID      tSpriteGUID{};
    std::string     strName{};
} PLAYER_SKILL;

typedef struct tagPlayerSkillSET
{
    static constexpr _uint      iNumSkills = 3;

    SKILL                       eSkill = SKILL::END;
    PLAYER_SKILL                skills[3];
} PLAYER_SKILLSET;

typedef struct tagPlayerContext
{
    /* 포인터 자체를 소유한 구조체들 */
    PLAYER_COMPONENTS       tComponents{};
    PLAYER_RUNTIME_REF      tRef{};

    /* 플레이어가 소유한 변수의 포인터를 가진 구조체들 */
    PLAYER_STATS*           pStats = nullptr;

    /* 헬퍼 */
    class CPlayer_SkillController*  pSkillController = nullptr;

} PLAYER_CONTEXT;

NS_END
