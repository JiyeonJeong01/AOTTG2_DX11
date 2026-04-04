#pragma once

#include "Client_Define.h"
#include "Entity_Define.h"

NS_BEGIN(Client)

enum class TITAN_STATE { IDLE = 0, MOVE, CHASE, ATTACK, GRAB, HURT, DEAD, END };
enum class TITAN_IDLE { DEFAULT = 0, SIT, DEFENSE, END };
enum class TITAN_MOVE { WALK = 0, END };
enum class TITAN_CHASE { };
enum class TITAN_ATTACK { };
enum class TITAN_GRAB { LEFT, RIGHT, END };
enum class TITAN_HURT { };
enum class TITAN_DEAD { };

typedef struct tagTitanComponents
{
    CTransform      transform;
    CAnimator       animator;
    CCollider       collider;
    CRigidbody      rigidbody;
    CMeshRenderer   meshRenderer;
} TITAN_COMPONENTS;

typedef struct tagTitanStats
{
    _float          fCurSpeed = 3.f;
    _float          fMaxSpeed = 5.f;

    _float          fJump = 4.f;
    _float          fJumpDash = 0.02f;

    _float          fRotateSharpness = 3.f;
} TITAN_STATS;

typedef struct tagTitanRuntimeRef
{
    class CNormalTitanStateMachine* pFSM = nullptr;
    class CTargetSensor*            pSensor = nullptr;
    class CTitanBound_Controller*   pBoundCtlr = nullptr;
} TITAN_RUNTIME_REF;

typedef struct tagTitanContext
{
    /* 포인터 자체를 소유한 구조체들 */
    TITAN_COMPONENTS       tComponents{};

    TITAN_RUNTIME_REF       tRef{};

    /* 플레이어가 소유한 변수의 포인터를 가진 구조체들 */
    TITAN_STATS* pStats = nullptr;

    /* 헬퍼 */


} TITAN_CONTEXT;

NS_END
