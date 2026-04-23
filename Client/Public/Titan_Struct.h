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
enum class TITAN_ATTACK_EREN { WAIT, THROW, PUNCH, JUMP };
enum class TITAN_DEAD { STAND_DEAD , SIT_DEAD, CRAWL_DEAD };

typedef struct tagTitanScriptablebject
{
    _float      fCurSpeed{};
    _float      fMaxSpeed{};

    _char       szIdleAnim[32];
    _char       szMoveAnim[32];

    _float      fMaxIdleTime{};
    _float      fMaxMoveTime{};

    _int       iHitEffect = 0;

    CGameObject*    goEren = nullptr;
}TITAN_SCRIPTABLE_OBJECT;

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
    _float          fCurSpeed = 20.f;       /* 무게가 10임 */
    _float          fMaxSpeed = 20.f;

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
    class CGroundChecker*           pGroundChecker = nullptr;

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

    /* 거인 정보 */
    TITAN_SCRIPTABLE_OBJECT* pSO = nullptr;

    _float                  fStayAttackErenDist = 12.5f; /* AttackEeren : eren을 공격해야 하는 거리 */

} TITAN_CONTEXT;



typedef struct tagTitanDustDesc
{
    _bool   bPlayed = false;
    _float  fTrackPosition = 0.f;
    _float3 vOffset = {};
} TITAN_DUST_DESC;

typedef struct tagTitanDustRuntime
{
    TITAN_DUST_DESC  tLeft{};
    TITAN_DUST_DESC  tRight{};

    _float      fPrevTrackPosition = 0.f;
    _bool       bFirstUpdate = true;

private:
    _bool Is_Passed_TrackPosition(_float fPrev, _float fCur, _float fTarget) const
    {
        if (fPrev <= fCur)
        {
            return (fPrev < fTarget && fTarget <= fCur);
        }
        else
        {
            /* 트랙이 루프되어 1 -> 0 으로 넘어간 상황 */
            return (fPrev < fTarget && fTarget <= 1.f) ||
                (0.f <= fTarget && fTarget <= fCur);
        }
    }

    _float3 Calc_WorldPos(const CTransform& tr, const _float3& vOffset) const
    {
        const _vector vLocalOffset = XMVectorSet(vOffset.x, vOffset.y, vOffset.z, 0.f);
        const _vector vRotation = XMLoadFloat4(&tr->vRotationQuat);
        const _vector vWorldOffset = XMVector3Rotate(vLocalOffset, vRotation);
        const _vector vWorldPos = XMLoadFloat3(&tr->vPosition) + vWorldOffset;

        _float3 vResult{};
        XMStoreFloat3(&vResult, vWorldPos);
        return vResult;
    }

public:
    void Reset_PlayState()
    {
        tLeft.bPlayed = false;
        tRight.bPlayed = false;
        bFirstUpdate = true;
        fPrevTrackPosition = 0.f;
    }

    _bool Try_PlayDust(const CAnimator& anim, const CTransform& tr, _float3& outWorldPos)
    {
        if (!anim.Is_Valid() || !tr.Is_Valid())
            return false;

        const _float fTrackPosition = anim->fTrackPosition;

        if (bFirstUpdate)
        {
            bFirstUpdate = false;
            fPrevTrackPosition = fTrackPosition;
            return false;
        }

        if (!tLeft.bPlayed && Is_Passed_TrackPosition(fPrevTrackPosition, fTrackPosition, tLeft.fTrackPosition))
        {
            tLeft.bPlayed = true;
            tRight.bPlayed = false;
            outWorldPos = Calc_WorldPos(tr, tLeft.vOffset);
            fPrevTrackPosition = fTrackPosition;
            return true;
        }

        if (!tRight.bPlayed && Is_Passed_TrackPosition(fPrevTrackPosition, fTrackPosition, tRight.fTrackPosition))
        {
            tRight.bPlayed = true;
            tLeft.bPlayed = false;
            outWorldPos = Calc_WorldPos(tr, tRight.vOffset);
            fPrevTrackPosition = fTrackPosition;
            return true;
        }

        fPrevTrackPosition = fTrackPosition;
        return false;
    }

} TITAN_DUST_RUNTIME;

inline constexpr const char* TITAN_WEAK_POINT = "WeakPoint";
inline constexpr const char* TITAN_PUNCH_ATTACK_L = "Hand_Attack_L";
inline constexpr const char* TITAN_PUNCH_ATTACK_R = "Hand_Attack_R";
inline constexpr const char* TITAN_ROCK_1 = "TitanThrowRock1";
inline constexpr const char* TITAN_ROCK_2 = "TitanThrowRock2";
inline constexpr const char* TITAN_ROCK_3 = "TitanThrowRock3";
inline constexpr const char* TITAN_ROCK_4 = "TitanThrowRock4";
inline constexpr const char* TITAN_ROCK_5 = "TitanThrowRock5";
inline constexpr const char* TITAN_CRAWLER_BODY = "TitanCrawlerBody";

NS_END
