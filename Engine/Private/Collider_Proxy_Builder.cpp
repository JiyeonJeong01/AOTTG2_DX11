#include "Collider_Proxy_Builder.h"
#include "Component_System.h"
#include "Transform_Processor.h"
#include "Engine_Math.h"

CCollider_Proxy_Builder::CCollider_Proxy_Builder()
{
}

CCollider_Proxy_Builder::~CCollider_Proxy_Builder()
{
}

HRESULT CCollider_Proxy_Builder::Initialize()
{
    m_pTransform_Processor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransform_Processor, E_FAIL, "Can't bind transform processor");
}

void CCollider_Proxy_Builder::Build_Collider_Proxy(COLLIDER_DATA* pData, COLLIDER_PROXY_DATA& outProxy)
{
    TRANSFORM_DATA* pTr = m_pTransform_Processor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, pData->hTransform)._Data();
    IF_NULL_RETURN_MSG_BREAK(pTr, , "Can't build proxy; pTr is nullptr");

    outProxy = {};
    outProxy.pCol = pData;
    Math::Store(outProxy.vCenterWorld, Get_ColliderCenter_World(pData, pTr));

    switch (pData->eShape)
    {
    case SHAPE::BOX :
        Build_Box_Proxy(pData, pTr, outProxy);
        return;
    case SHAPE::SPHERE :
        Build_Sphere_Proxy(pData, pTr, outProxy);
        return;
    case SHAPE::PLANE :
        Build_Plane_Proxy(pData, pTr, outProxy);
        return;
    }
}

/* 현재 구조상 BOX 콜라이더는 무조건 AABB만 진행한다. */
void CCollider_Proxy_Builder::Build_Box_Proxy(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr, COLLIDER_PROXY_DATA& outProxy)
{
    const _vector vCenter = Math::Load(outProxy.vCenterWorld);
    const _float3 vHalfLocal = pCol->box.vHalfExtentsLocal;
    const _float3 vScale = pTr->vScale;

    outProxy.box.vHalfExtentsWorld.x = fabsf(vHalfLocal.x * vScale.x);
    outProxy.box.vHalfExtentsWorld.y = fabsf(vHalfLocal.y * vScale.y);
    outProxy.box.vHalfExtentsWorld.z = fabsf(vHalfLocal.z * vScale.z);

    const _vector vHalf = Math::Load(outProxy.box.vHalfExtentsWorld);

    _float3 vMin{};
    _float3 vMax{};

    Math::Store(vMin, vCenter - vHalf);
    Math::Store(vMax, vCenter + vHalf);

    outProxy.aabbWorld.vMin = vMin;
    outProxy.aabbWorld.vMax = vMax;
}

void CCollider_Proxy_Builder::Build_Sphere_Proxy(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr, COLLIDER_PROXY_DATA& outProxy)
{
    const _vector vCenter = Math::Load(outProxy.vCenterWorld);

    outProxy.sphere.fRadiusWorld = Get_SphereRadius_World(pCol, pTr);

    const _float fRadius = outProxy.sphere.fRadiusWorld;
    const _vector vExtent = Math::Set_Vec(fRadius, fRadius, fRadius, 0.f);

    _float3 vMin{};
    _float3 vMax{};

    Math::Store(vMin, vCenter - vExtent);
    Math::Store(vMax, vCenter + vExtent);

    outProxy.aabbWorld.vMin = vMin;
    outProxy.aabbWorld.vMax = vMax;
}

/* 반드시 local normal의 기본 값이 (0, 0, 1이어야 한다.) */
void CCollider_Proxy_Builder::Build_Plane_Proxy(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr, COLLIDER_PROXY_DATA& outProxy)
{
    const _vector vCenter = Math::Load(outProxy.vCenterWorld);

    outProxy.plane.bInfinite = pCol->plane.bInfinite;
    outProxy.plane.vDimension = pCol->plane.vDimension;

    const _vector vLocalN = Math::Load(pCol->plane.vNormalLocal);
    const _vector vRotQ = Math::Load(pTr->vRotationQuat);

    const _vector vWorldN = Math::Normalize(Math::Rotate(vLocalN, vRotQ));
    Math::Store(outProxy.plane.vNormalWorld, vWorldN);

    outProxy.plane.fDistanceWorld = -Math::Get_X(Math::Dot(vWorldN, vCenter));
    if (outProxy.plane.bInfinite)
    {
        const _float INF_EXT = 1e6f;

        _float3 vMin{};
        _float3 vMax{};

        vMin.x = outProxy.vCenterWorld.x - INF_EXT;
        vMin.y = outProxy.vCenterWorld.y - INF_EXT;
        vMin.z = outProxy.vCenterWorld.z - INF_EXT;

        vMax.x = outProxy.vCenterWorld.x + INF_EXT;
        vMax.y = outProxy.vCenterWorld.y + INF_EXT;
        vMax.z = outProxy.vCenterWorld.z + INF_EXT;

        outProxy.aabbWorld.vMin = vMin;
        outProxy.aabbWorld.vMax = vMax;
        return;
    }

    _vector vRef = Math::Set_Vec(0.f, 1.f, 0.f, 0.f);
    if (fabsf(Math::Get_X(Math::Dot(vRef, vWorldN))) > 0.99f)
        vRef = Math::Set_Vec(1.f, 0.f, 0.f, 0.f);

    const _vector vU = Math::Normalize(Math::Cross(vRef, vWorldN));
    const _vector vV = Math::Normalize(Math::Cross(vWorldN, vU));

    Math::Store(outProxy.plane.vAxisUWorld, vU);
    Math::Store(outProxy.plane.vAxisVWorld, vV);

    const _float fHalfW = outProxy.plane.vDimension.x * 0.5f;
    const _float fHalfH = outProxy.plane.vDimension.y * 0.5f;

    const _vector c0 = vCenter + vU * fHalfW + vV * fHalfH;
    const _vector c1 = vCenter + vU * fHalfW - vV * fHalfH;
    const _vector c2 = vCenter - vU * fHalfW + vV * fHalfH;
    const _vector c3 = vCenter - vU * fHalfW - vV * fHalfH;

    _float3 p0{}, p1{}, p2{}, p3{};
    Math::Store(p0, c0);
    Math::Store(p1, c1);
    Math::Store(p2, c2);
    Math::Store(p3, c3);

    _float3 vMin = p0;
    _float3 vMax = p0;

    auto Expand = [&](_float3 p)
        {
            vMin.x = (p.x < vMin.x) ? p.x : vMin.x;
            vMin.y = (p.y < vMin.y) ? p.y : vMin.y;
            vMin.z = (p.z < vMin.z) ? p.z : vMin.z;

            vMax.x = (p.x > vMax.x) ? p.x : vMax.x;
            vMax.y = (p.y > vMax.y) ? p.y : vMax.y;
            vMax.z = (p.z > vMax.z) ? p.z : vMax.z;
        };

    Expand(p1);
    Expand(p2);
    Expand(p3);

    const _float thickEps = 1e-2f;
    vMin.x -= thickEps; vMin.y -= thickEps; vMin.z -= thickEps;
    vMax.x += thickEps; vMax.y += thickEps; vMax.z += thickEps;

    outProxy.aabbWorld.vMin = vMin;
    outProxy.aabbWorld.vMax = vMax;
}

_vector CCollider_Proxy_Builder::Get_ColliderCenter_World(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr)
{
    return (Math::Load(pTr->vPosition) + Math::Load(pCol->vOffset));
}

_float CCollider_Proxy_Builder::Get_SphereRadius_World(COLLIDER_DATA* pCol, TRANSFORM_DATA* pTr)
{
    const _float fScaleX = fabsf(pTr->vScale.x);
    const _float fScaleY = fabsf(pTr->vScale.y);
    const _float fScaleZ = fabsf(pTr->vScale.z);

    const _float fScaleXY = (fScaleX > fScaleY) ? fScaleX : fScaleY;
    const _float fMax = (fScaleXY > fScaleZ) ? fScaleXY : fScaleZ;

    return pCol->sphere.fRadiusLocal * fMax;
}

std::unique_ptr<CCollider_Proxy_Builder> CCollider_Proxy_Builder::Create()
{
    auto pInstance = std::make_unique<CCollider_Proxy_Builder>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "pInstance create failed");
    return pInstance;
}
