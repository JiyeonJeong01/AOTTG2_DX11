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

    BODY_TYPE       eColType{ BODY_TYPE::STATIC };
    _bool           bOnCol{ false };

    GEOMETRY_TYPE   eShape{ GEOMETRY_TYPE::BOX };
    _float3         vPoint; /* vPosition의 캐시 용도 */
    _float3         vScale{ 1.f,1.f,1.f };
    _float3         vOffset{ 0.f, 0.f, 0.f };
    _bool           bDirty{ true };

    /* Shape params */
    union
    {
        struct /* Box */ 
        {
            _float3 vHalfExtentsLocal; /* (x,y,z) */
        } box;

        struct /* SPHERE */ 
        {
            _float  fRadiusLocal;
        } sphere;

        struct /* PLANE */ 
        {
            _float3 vNormalLocal;
            _float  fDistance;    /* plane: dot(n, x) + d = 0  (d = distance term) */
            _bool   bInfinite;
        } plane;
    };

    // broad-phase
    AABB aabbWorld;

} COLLIDER_DATA;

class ENGINE_DLL CCollider final : public CComponent_Proxy_Base<COLLIDER_DATA, CCollider, COMPONENT_TYPE::COLLIDER>
{
public:
    CCollider() : CComponent_Proxy_Base() {}
    CCollider(DataType* pData, COMPONENT_HANDLE handle)
        : CComponent_Proxy_Base(pData, handle) { }
    ~CCollider() override = default;
public:
    void Set_OnCol(_bool b);
    _bool Get_OnCol() const;

    void Set_OffsetLocal(const _float3& v);
    _float3 Get_OffsetLocal() const;

    const AABB& Get_AABBWorld() const;

public:
    // shape
    void Set_Box(const _float3& halfExtentsLocal); 
    void Set_Sphere(_float radiusLocal);
    void Set_Plane(const _float3& normalLocal, _float distance);
};



NS_END
