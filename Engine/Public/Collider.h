//#pragma once
//#include "CComponent_Proxy_Base.h"
//#include "Physics_Struct.h"
//
//NS_BEGIN(Engine)
//
//class CRigidbody;
//
//typedef struct ENGINE_DLL tagColliderData final
//{
//    // owner links (DOD: dependency resolved as handles)
//    COMPONENT_HANDLE hTransform{ INVALID_HANDLE_UINT };
//    COMPONENT_HANDLE hRigidbody{ INVALID_HANDLE_UINT }; // optional
//
//    // runtime flags
//    uint16_t        iId{ 0 };
//    BODY_TYPE       eColType{ BODY_TYPE::STATIC };
//    _bool           bOnCol{ false };
//
//    // common
//    COLLIDER_TYPE   eShape{ COLLIDER_TYPE::BOX };
//    _float3         vOffsetLocal{ 0.f,0.f,0.f }; // local-space offset
//    _bool           bDirty{ true };
//
//    // shape params (local space)
//    union
//    {
//        struct // BOX
//        {
//            _float3 vHalfExtentsLocal; // (x,y,z) half
//        } box;
//
//        struct // SPHERE
//        {
//            _float  fRadiusLocal;
//            _float  pad0[2];
//        } sphere;
//
//        struct // PLANE (local)
//        {
//            _float3 vNormalLocal; // should be normalized
//            _float  fDistance;    // plane: dot(n, x) + d = 0  (d = distance term)
//        } plane;
//    };
//
//    // broad-phase
//    AABB_DESC aabbWorld;
//
//} COLLIDER_DATA;
//
//class ENGINE_DLL CCollider final : public CComponent_Proxy_Base<COLLIDER_DATA, CCollider>
//{
//public:
//    CCollider() : CComponent_Proxy_Base() { m_eComType = COMPONENT_TYPE::COLLIDER; }
//    CCollider(DataType* pData, COMPONENT_HANDLE handle)
//        : CComponent_Proxy_Base(pData, handle) {
//        m_eComType = COMPONENT_TYPE::COLLIDER;
//    }
//    ~CCollider() override = default;
//
//public:
//    // dependency (보통 Create_From_Spec나 Resolve에서 세팅)
//    void Set_TransformHandle(COMPONENT_HANDLE h); // { if (m_pData) { m_pData->hTransform = h; m_pData->bDirty = true; } }
//    void Set_RigidbodyHandle(COMPONENT_HANDLE h); // { if (m_pData) { m_pData->hRigidbody = h; } }
//
//public:
//    // common
//    void Set_ColliderID(uint16_t id);// { IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr."); m_pData->iId = id; }
//    uint16_t Get_ColliderID() const; // { IF_NULL_RETURN_MSG_BREAK(m_pData, 0, "m_pData is nullptr."); return m_pData->iId; }
//
//    void Set_ColType(BODY_TYPE eBody); // { _DEBUG_NULL_BREAK_RETURN(m_pData, ); m_pData->eColType = e; }
//    BODY_TYPE Get_ColType() const; // { IF_NULL_RETURN_MSG_BREAK(m_pData, COL_TYPE::C_STATIC, "m_pData is nullptr."); return m_pData->eColType; }
//
//    void Set_OnCol(_bool b); // { _DEBUG_NULL_BREAK_RETURN(m_pData, ); m_pData->bOnCol = b; }
//    _bool Get_OnCol() const; // { IF_NULL_RETURN_MSG_BREAK(m_pData, false, "m_pData is nullptr."); return m_pData->bOnCol; }
//
//    void Set_OffsetLocal(const _float3& v); // { _DEBUG_NULL_BREAK_RETURN(m_pData, ); m_pData->vOffsetLocal = v; m_pData->bDirty = true; }
//    _float3 Get_OffsetLocal() const; // { IF_NULL_RETURN_MSG_BREAK(m_pData, _float3{}, "m_pData is nullptr."); return m_pData->vOffsetLocal; }
//
//    const AABB_DESC& Get_AABBWorld() const; // { IF_NULL_RETURN_MSG_BREAK(m_pData, *(AABB_DESC*)nullptr, "m_pData is nullptr."); return m_pData->aabbWorld; }
//
//public:
//    // shape
//    void Set_Box(const _float3& halfExtentsLocal); 
//    //{
//    //    _DEBUG_NULL_BREAK_RETURN(m_pData, );
//    //    m_pData->eShape = COLLIDER_SHAPE::BOX;
//    //    m_pData->box.vHalfExtentsLocal = halfExtentsLocal;
//    //    m_pData->bDirty = true;
//    //}
//
//    void Set_Sphere(_float radiusLocal);
//    //{
//    //    _DEBUG_NULL_BREAK_RETURN(m_pData, );
//    //    m_pData->eShape = COLLIDER_SHAPE::SPHERE;
//    //    m_pData->sphere.fRadiusLocal = radiusLocal;
//    //    m_pData->bDirty = true;
//    //}
//
//    void Set_Plane(const _float3& normalLocal, _float distance);
//    //{
//    //    _DEBUG_NULL_BREAK_RETURN(m_pData, );
//    //    m_pData->eShape = COLLIDER_SHAPE::PLANE;
//    //    m_pData->plane.vNormalLocal = normalLocal;
//    //    m_pData->plane.fDistance = distance;
//    //    m_pData->bDirty = true;
//    //}
//};
//
//
//
//NS_END
