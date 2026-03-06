#pragma once
#include "CComponent_Proxy_Base.h"
#include "Physics_Struct.h"

NS_BEGIN(Engine)

class CRigidbody;

typedef struct ENGINE_DLL tagColliderData final
{
    OBJECT_HANDLE       hObject{};
    _bool               bEnable = false;

    COMPONENT_HANDLE    hTransform{ INVALID_HANDLE_UINT };
    COMPONENT_HANDLE    hRigidbody{ INVALID_HANDLE_UINT };

    BODY_TYPE           eColType{ BODY_TYPE::STATIC };
    _bool               bOnCol{ false };

    SHAPE               eShape{ SHAPE::END };

    _float3             vPoint{};                    /* transform world position cache */
    _float3             vScale{ 1.f, 1.f, 1.f };
    _float3             vOffset{ 0.f, 0.f, 0.f };
    _bool               bDirty{ true };

    /* Shape params : Local */
    union
    {
        /* BOX */
        struct
        {
            _float3 vHalfExtentsLocal;
        } box;

        /* SPHERE */
        struct
        {
            _float  fRadiusLocal;
        } sphere;

        /* PLANE */
        struct
        {
            /* Local */
            _float3 vNormalLocal;
            _float  fDistance;               /* dot(n, x) + d = 0 */
            _bool   bInfinite;
            _float2 vDimension;
        } plane;
    };

    /* Broad-phase */
    AABB                aabbLocal{};

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

    const AABB& Get_AABBWorld() const;
    _float3 Get_CenterWorld() const;
    _float Get_RadiusWorld() const;

public:
    // shape
    void Set_Shape(SHAPE eShape);
    void Set_HalfExtents(_fvector vExtents);
    void Set_fRadius(_float fRadius);
    void Set_PlaneInfinite(_bool bInfinite);
};

NS_END
