//#include "Collider.h"
//#include "Engine_Log.h"
//
//void CCollider::Set_TransformHandle(COMPONENT_HANDLE h)
//{
//    if (m_pData)
//    {
//        m_pData->hTransform = h;
//        m_pData->bDirty = true;
//    }
//}
//
//void CCollider::Set_RigidbodyHandle(COMPONENT_HANDLE h)
//{
//    if (m_pData)
//    {
//        m_pData->hRigidbody = h;
//    }
//}
//
//void CCollider::Set_ColliderID(uint16_t id)
//{
//    IF_NULL_RETURN_MSG_BREAK(m_pData, , "m_pData is nullptr");
//    m_pData->iId = id;
//}
//
//uint16_t CCollider::Get_ColliderID() const
//{
//    IF_NULL_RETURN_MSG_BREAK(m_pData, 0, "m_pData is nullptr.");
//    return m_pData->iId;
//}
//
//void CCollider::Set_ColType(BODY_TYPE eBody)
//{
//}
//
//BODY_TYPE CCollider::Get_ColType() const
//{
//}
//
//void CCollider::Set_OnCol(_bool b)
//{
//}
//
//_bool CCollider::Get_OnCol() const
//{
//}
//
//void CCollider::Set_OffsetLocal(const _float3& v)
//{
//}
//
//_float3 CCollider::Get_OffsetLocal() const
//{
//}
//
//const AABB_DESC& CCollider::Get_AABBWorld() const
//{
//}
//
//void CCollider::Set_Box(const _float3& halfExtentsLocal)
//{
//}
//
//void CCollider::Set_Sphere(_float radiusLocal)
//{
//}
//
//void CCollider::Set_Plane(const _float3& normalLocal, _float distance)
//{
//}
