#pragma once

#include "Engine_Define.h"
#include "Component_Struct.h"
#include "Engine_Math.h"

NS_BEGIN(Engine)

struct tagColliderData;
typedef struct tagColliderData COLLIDER_DATA;

struct tagObjectHandle;
typedef struct tagObjectHandle OBJECT_HANDLE;

typedef struct tagAABB
{
    _float3 vMin{ 0.f,0.f,0.f };
    _float3 vMax{ 0.f,0.f,0.f };
    _bool   bValid{ true };
}AABB;

typedef struct ENGINE_DLL tagColliderProxyData final
{
    COLLIDER_DATA*  pCol = nullptr;
    _float3         vCenterWorld{};                 /* 월드 기준 충돌 중심점
                                                        - BOX    : 박스 중심
                                                        - SPHERE : 구 중심
                                                        - PLANE  : 평면 중심점
                                                        보통 transform + offset 반영 결과 */

    union
    {
        struct { _float3 vHalfExtentsWorld; } box;  /* scale 반영된 월드 반쪽 크기 */

        struct { _float  fRadiusWorld; } sphere;    /* scale 반영된 월드 반지름 */

        struct {
            _float3 vNormalWorld;                   /* 월드 공간 기준 평면 법선. 실제 signed distance 계산에 사용 */
            _float  fDistanceWorld;                 /* 월드 plane equation 값 */
            _bool   bInfinite;
            _float2 vDimension;
            _float3 vAxisUWorld;
            _float3 vAxisVWorld; } plane;
    };

    AABB    aabbWorld{};
} COLLIDER_PROXY_DATA;

typedef struct tagGridCoord
{
    int iX{}, iY{}, iZ{};
}GRID_COORD;

typedef struct tagGridKey
{
    int iX{}, iY{}, iZ{};
}GRID_KEY;

typedef struct tagColPair
{
    COLLIDER_PROXY_DATA* pColA{};
    COLLIDER_PROXY_DATA* pColB{};
    tagColPair(COLLIDER_PROXY_DATA* a, COLLIDER_PROXY_DATA* b) : pColA(a), pColB(b) {}
}COLLIDER_PAIR;

typedef struct tagCollision
{
    OBJECT_HANDLE       hObject{};			// 충돌 당한 오브젝트
    COLLIDER_DATA*      pCounterCollider{}; // 충돌 당한 오브젝트의 콜라이더

    _float3			vPoint;
}COLLISION_DESC;

typedef struct tagContactInfo
{
    COLLISION_DESC  tCollisionA{};
    COLLISION_DESC  tCollisionB{};
    COLLIDER_DATA*  pColA{};
    COLLIDER_DATA*  pColB{};
    _float3		vResolveN_A{};		// A가 겹침을 해결하는 방향
    _float3		vPoint{};
    float		fDepth{};
}CONTACT_DESC;

typedef struct tagPairKey
{
    uint32_t aKey;
    uint32_t bKey;

    explicit tagPairKey(uint32_t a, uint32_t b)
    {
        if (a > b)
        {
            aKey = b;
            bKey = a;
        }
        else
        {
            aKey = a;
            bKey = b;
        }
    }

    bool operator==(const tagPairKey& other) const
    {
        return aKey == other.aKey && bKey == other.bKey;
    }
}PAIR_KEY;

typedef struct PairKeyHash
{
    size_t operator()(const PAIR_KEY& k) const
    {
        return (static_cast<size_t>(k.aKey) << 32) ^ k.bKey;
    }
}PAIR_KEY_HASH;

typedef struct tagAxisMask
{
    bool bX = false;
    bool bY = false;
    bool bZ = false;
}AXIS_MASK;

NS_END
