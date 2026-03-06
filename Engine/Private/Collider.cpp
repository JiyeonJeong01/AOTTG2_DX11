#include "Collider.h"
#include "Engine_Math.h"

_bool CCollider::Get_OnCol() const
{
    return m_pData->bOnCol;
}

void CCollider::Set_Offset(const _float3& vOffset)
{
    m_pData->vOffset = vOffset;
    m_pData->bDirty = true;
}

_float3 CCollider::Get_Offset() const
{
    return m_pData->vOffset;
}

const AABB& CCollider::Get_AABBWorld() const
{
    return m_pData->aabbLocal;
}

void CCollider::Set_Shape(SHAPE eShape)
{
    if (m_pData->eShape != SHAPE::END)
        return; /* 다른 타입으로 도중에 변경 불가 */
    switch (eShape)
    {
    case SHAPE::BOX:
        m_pData->box.vHalfExtentsLocal = _float3{ 0.5f, 0.5f, 0.5f };
        break;

    case SHAPE::SPHERE:
        m_pData->sphere.fRadiusLocal = 0.5f;
        break;

    case SHAPE::PLANE:
        m_pData->plane.vNormalLocal = _float3{ 0.f, 1.f, 0.f };
        m_pData->plane.fDistance = 0.f;
        m_pData->plane.bInfinite = true;
        break;

    case SHAPE::END:
    default:
        return;
    }
    m_pData->eShape = eShape;
    m_pData->bDirty = true;
}

void CCollider::Set_HalfExtents(_fvector vExtents)
{
    if (m_pData->eShape != SHAPE::BOX)
        return;
    Math::Store(m_pData->box.vHalfExtentsLocal, vExtents);
    m_pData->bDirty = true;
}

void CCollider::Set_fRadius(_float fRadius)
{
    if (m_pData->eShape != SHAPE::SPHERE)
        return;
    m_pData->sphere.fRadiusLocal = fRadius;
    m_pData->bDirty = true;
}

void CCollider::Set_PlaneInfinite(_bool bInfinite)
{
    if (m_pData->eShape != SHAPE::PLANE)
        return;
    m_pData->plane.bInfinite = bInfinite;
    m_pData->bDirty = true;
}
