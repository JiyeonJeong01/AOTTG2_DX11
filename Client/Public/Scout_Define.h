#pragma once

#include "Client_Define.h"
#include "Entity_Define.h"

NS_BEGIN(Client)

enum class SCOUT_BEHAVIOR : uint32_t
{
    NONE,
    REQUEST_RESUPPLY,
    RESCUE_DIALOGUE,
    END
};

typedef struct tagScoutComponents
{
    CTransform      transform;
    CAnimator       animator;
    CCollider       collider;
    CMeshRenderer   meshRenderer;
} SCOUT_COMPONENTS;

typedef struct tagScoutStats
{
    _float                      fCurSpeed = 2;
    _float                      fMaxSpeed = 2;

    _float                      fJump = 2.f;
    _float                      fJumpDash = 0.02f;
} SCOUT_STATS;

typedef struct tagScoutContext
{
    SCOUT_BEHAVIOR      eBehaviour = SCOUT_BEHAVIOR::NONE;
    SCOUT_COMPONENTS    tComponents{};
    SCOUT_STATS*        pStat{};
} SCOUT_CONTEXT;


NS_END
