#pragma once

#include "Client_Define.h"

NS_BEGIN(Client)

typedef struct tagPlayerInputCommand
{
    _float2 vMouseDelta{};
    _float3 vMove{};

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
    CRigidbody      rigidbody;
    CSpringJoint    springJoint;
    CMeshRenderer   meshRenderer;
} PLAYER_COMPONENTS;

typedef struct tagPlayerRuntimeRef
{
    CGameObject*                pGroundChecker = nullptr;
    class CODM_Gear*            pGear = nullptr;
    class CCameraController*    pCameraController = nullptr;
    class CPlayerStateMachine*  pFSM = nullptr;
} PLAYER_RUNTIME_REF;


enum class PLAYER_STATE { IDLE = 0, MOVE, JUMP, AIRBORNE, HOOK, ATTACK, SHOOT, RELOAD, DODGE, RESUPPLY, GRABBED, EMOTE, END };

NS_END
