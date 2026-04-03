#pragma once

#include "Client_Define.h"
#include "Entity_Define.h"

NS_BEGIN(Client)

enum class SCOUT_BEHAVIOR { IDLE, WAIT, MOVE, RUNAWAY, GRABBED, END };

typedef struct tagScoutComponents
{
    CTransform      transform;
    CAnimator       animator;
    CCollider       collider;
    CMeshRenderer   meshRenderer;
} SCOUT_COMPONENTS;

typedef struct tagScoutStats
{
    _float                      fCurSpeed = 10.f;
    _float                      fMaxSpeed = 13.f;

    _float                      fJump = 2.f;
    _float                      fJumpDash = 0.02f;
} SCOUT_STATS;

typedef struct tagScoutContext
{
    SCOUT_COMPONENTS    tComponents{};
    SCOUT_STATS*        pStat{};
} SCOUT_CONTEXT;


NS_END
