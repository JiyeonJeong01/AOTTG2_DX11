#pragma once
#include "CComponent_Proxy_Base.h"
#include "Event.h"
#include "Physics_Struct.h"

NS_BEGIN(Engine)

class CRigidbody;

typedef struct ENGINE_DLL tagColliderData final
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;
    _bool               bTrigger = false;

    COMPONENT_HANDLE    hSelf{ INVALID_HANDLE }; /* self */
    COMPONENT_HANDLE    hTransform{ INVALID_HANDLE };
    COMPONENT_HANDLE    hRigidbody{ INVALID_HANDLE };

    _bool               bOnCol{ false };

    SHAPE               eShape{ SHAPE::END };

    _float3             vOffset{ 0.f, 0.f, 0.f };   /* 위치로부터 로컬 거리 */
    _float3             vRotationOffset{ 0.f, 0.f, 0.f };       /* collider local euler rotation offset in degrees */

    _bool               bDirty{ true };

    CEvent<const COLLISION_DESC&> OnCollisionEnter;
    CEvent<const COLLISION_DESC&> OnCollisionStay;
    CEvent<const COLLISION_DESC&> OnCollisionExit;

    CEvent<const COLLISION_DESC&> OnTriggerEnter;
    CEvent<const COLLISION_DESC&> OnTriggerStay;
    CEvent<const COLLISION_DESC&> OnTriggerExit;

    /* Shape params : Local
       - 에디터/직렬화에서 보관하는 원본 로컬 값
       - 실제 충돌 계산 전에는 proxy 에서 월드값으로 변환해서 사용 */
    union
    {
        /* BOX */
        struct
        {
            _float3 vHalfExtentsLocal;  /* 로컬 공간 기준 반쪽 크기로, 최종 월드 OBB/AABB 계산의 기초값이다. */
        } box;

        /* SPHERE */
        struct
        {
            _float  fRadiusLocal;       /* 로컬 반지름으로, 월드에서는 scale 반영 후 fRadiusWorld 로 계산 */
        } sphere;

        /* PLANE */
        struct
        {
            /* Local */
            _float3 vNormalLocal;       /* 로컬 공간 기준 평면 법선이다. {0,1,0} 이면 로컬 +Y 방향을 바라보는 평면 */
            _bool   bInfinite;
            _float2 vDimension;         /* 유한 평면일 때의 전체 가로/세로 길이
                                        x : U축 방향 길이
                                        y : V축 방향 길이 */
        } plane;
    };

} COLLIDER_DATA;

class ENGINE_DLL CCollider final : public CComponent_Proxy_Base<COLLIDER_DATA, CCollider, COMPONENT_TYPE::COLLIDER>
{
public:
    CCollider() : CComponent_Proxy_Base() {}
    CCollider(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) {
    }
    ~CCollider() override = default;

public:
    _bool Get_OnCol() const;

    void Set_Offset(const _float3& vOffset);
    _float3 Get_Offset() const;

public:
    // shape
    void Set_Shape(SHAPE eShape);
    void Set_HalfExtents(_fvector vExtents);
    void Set_fRadius(_float fRadius);
    void Set_PlaneInfinite(_bool bInfinite);
};

NS_END
