#pragma once

#include "Client_Define.h"
#include "Entity_Define.h"

NS_BEGIN(Client)

enum class TITAN_POSE { STAND, SIT, CRAWL, END };

enum class TITAN_STATE { IDLE = 0, MOVE, CHASE, ATTACK, GRAB, HURT, STUNNED, ATTACK_EREN, DEAD, END };
enum class TITAN_IDLE { DEFAULT = 0, SIT, DEFENSE, END };
enum class TITAN_MOVE { WALK = 0, END };
enum class TITAN_CHASE { END };
enum class TITAN_ATTACK { END };
enum class TITAN_GRAB { LEFT, RIGHT, END };
enum class TITAN_HURT { STAND_EYE = 0, STAND_ARM_L, STAND_ARM_R, STAND_LEG_L, STAND_LEG_R, SIT_EYE, CRAWL_EYE, END };
enum class TITAN_ATTACK_EREN { WAIT, THROW, PUNCH };
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
    _float          fCurSpeed = 8.f;
    _float          fMaxSpeed = 10.f;

    _float          fJump = 4.f;
    _float          fJumpDash = 0.02f;

    _float          fRotateSharpness = 3.f;

    const _uint     iMaxStunned = 5;
} TITAN_STATS;

typedef struct tagTitanRuntimeRef
{
    class CTitanStateMachine*       pFSM = nullptr;
    class CTargetSensor*            pSensor = nullptr;
    class CTitanBound_Controller*   pBoundCtlr = nullptr;
    class Engine::CNavMesh*         pNav = nullptr;
    unordered_map<std::string, class CHitBox*>*  pAllHitBoxes;

    TITAN_POSE*                     pPose = nullptr;

    _uint*                          m_pStunnedAcc = nullptr;
} TITAN_RUNTIME_REF;

typedef struct tagTitanContext
{
    /* 포인터 자체를 소유한 구조체들 */
    TITAN_COMPONENTS        tComponents{};

    TITAN_RUNTIME_REF       tRef{};

    /* 플레이어가 소유한 변수의 포인터를 가진 구조체들 */
    TITAN_STATS*            pStats = nullptr;
    PATROL_INFO*            pPatrol = nullptr;

    /* 헬퍼 */


} TITAN_CONTEXT;

inline constexpr const char* TITAN_PUNCH_ATTACK_L = "Hand_Attack_L";
inline constexpr const char* TITAN_PUNCH_ATTACK_R = "Hand_Attack_R";
inline constexpr const char* TITAN_ROCK_1 = "TitanThrowRock1";
inline constexpr const char* TITAN_ROCK_2 = "TitanThrowRock2";
inline constexpr const char* TITAN_ROCK_3 = "TitanThrowRock3";
inline constexpr const char* TITAN_ROCK_4 = "TitanThrowRock4";
inline constexpr const char* TITAN_ROCK_5 = "TitanThrowRock5";

NS_END
