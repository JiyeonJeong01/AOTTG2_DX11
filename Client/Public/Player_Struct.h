#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

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

typedef struct tagPlayerInfo
{
    _float                      fCurSpeed = 10.f;
    _float                      fMaxSpeed = 13.f;

    _float                      fJump = 12.f;

    uint32_t                    iAnimFlag = 0;

} PLAYER_INFO;

typedef struct tagPlayerRuntimeRef
{
    class CGroundChecker*       pGroundChecker = nullptr;
    class CODM_Gear*            pGear = nullptr;
    class CCameraController*    pCameraController = nullptr;
    class CPlayerStateMachine*  pFSM = nullptr;
} PLAYER_RUNTIME_REF;




enum class PLAYER_STATE { IDLE = 0, GROUNDED_MOVE, JUMP, AIRBORNE_MOVE, HOOK, GROUNDED_ATTACK, AIRBORNE_ATTACK, SHOOT, RELOAD, DODGE, RESUPPLY, GRABBED, EMOTE, END };

enum class AIRBORNE_STATE   : uint8_t { AIR_BEGIN, AIR_LEFT, AIR_RIGHT, AIR_FRONT, AIR_BACK, AIR_FALL, END };
enum class GROUNDED_MOVE    : uint8_t { RUN, SLIDE, DASH_LAND, END };
enum class JUMP             : uint8_t { JUMP_BEGIN, RISE, FALL, END };


NS_END
