#include "Collision_Detector.h"
#include "Component_System.h"
#include "Physics_Processor.h"
#include "Transform_Processor.h"
#include "Engine_Math.h"

namespace
{
    inline _vector Get_ColliderCenter(COLLIDER_PROXY_DATA* pCol)
    {
        return Math::Load(pCol->vCenterWorld);
    }

    inline _float Get_SphereWorldRadius(COLLIDER_PROXY_DATA* pCol)
    {
        return pCol->sphere.fRadiusWorld;
    }

    inline _vector Get_PlaneNormal(COLLIDER_PROXY_DATA* pCol)
    {
        return Math::Load(pCol->plane.vNormalWorld);
    }

    inline _float Get_PlaneD(COLLIDER_PROXY_DATA* pCol)
    {
        return pCol->plane.fDistanceWorld;
    }

    inline _float Calculate_SignedDistToPlane(COLLIDER_PROXY_DATA* pCol, _fvector vPoint)
    {
        const _vector vPlaneN = Get_PlaneNormal(pCol);
        const _float fPlaneD = Get_PlaneD(pCol);
        return XMVectorGetX(XMVector3Dot(vPlaneN, vPoint)) + fPlaneD;
    }

    inline _vector Safe_Normalize(_fvector v, _fvector vFallback)
    {
        const _float fLenSq = XMVectorGetX(XMVector3LengthSq(v));
        if (fLenSq <= 1e-8f)
            return vFallback;

        return XMVector3Normalize(v);
    }
}

CCollision_Detector::CCollision_Detector(CPhysics_Processor* pPhysics)
    : m_pPhysics_Processor(pPhysics)
{
}

CCollision_Detector::~CCollision_Detector()
{
}

HRESULT CCollision_Detector::Initialize()
{
    m_pTransform_Processor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransform_Processor, E_FAIL, "PhysicsPorcessor bind failed");

    Register_DetectTable();

    return S_OK;
}

void CCollision_Detector::Generate_BroadPhase_Pairs(const vector<COLLIDER_PROXY_DATA>& allColliders, vector<COLLIDER_PAIR>& outPair)
{
    outPair.clear();

    const uint32_t iTotalCnt = static_cast<uint32_t>(allColliders.size());
    if (iTotalCnt < 2)
        return;

    for (uint32_t i = 0; i < iTotalCnt - 1; ++i)
    {
        const COLLIDER_PROXY_DATA* pColA = &allColliders[i];
        if (!pColA || !pColA->pCol || !pColA->pCol->bEnable)
            continue;

        for (uint32_t j = i + 1; j < iTotalCnt; ++j)
        {
            const COLLIDER_PROXY_DATA* pColB = &allColliders[j];
            if (!pColB || !pColB->pCol || !pColB->pCol->bEnable)
                continue;

            outPair.emplace_back(const_cast<COLLIDER_PROXY_DATA*>(pColA), const_cast<COLLIDER_PROXY_DATA*>(pColB));
        }
    }
}

void CCollision_Detector::Process_NarrowPhase(const vector<COLLIDER_PAIR>& pairs)
{
    for (const auto& pair : pairs)
    {
        COLLIDER_PROXY_DATA* pColA = pair.pColA;
        COLLIDER_PROXY_DATA* pColB = pair.pColB;

        if (!pColA || !pColB || !pColA->pCol || !pColB->pCol)
            continue;

        SHAPE eColA = pColA->pCol->eShape;
        SHAPE eColB = pColB->pCol->eShape;

        bool bOnCollision = false;
        CONTACT_DESC tContact{};

        auto& fn = m_DetectTable[To<size_t>(eColA)][To<size_t>(eColB)];
        if (!fn)
            continue;

        bOnCollision = fn(&tContact, pColA, pColB);
        if (bOnCollision)
        {
            _DEBUG_INFO("On Collision");
            Fill_ContactInfo(tContact);
        }
    }
}

_bool CCollision_Detector::Detect_SphereCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)
{
    if (!pOut || !pColA || !pColB || !pColA->pCol || !pColB->pCol)
        return false;

    if (pColA->pCol->eShape != SHAPE::SPHERE || pColB->pCol->eShape != SHAPE::SPHERE)
        return false;

    const _vector vCenterA = Get_ColliderCenter(pColA);
    const _vector vCenterB = Get_ColliderCenter(pColB);

    const _vector vDiff = vCenterB - vCenterA;
    const _float fDistSq = Math::Get_X(XMVector3LengthSq(vDiff));

    const _float fRadiusA = Get_SphereWorldRadius(pColA);
    const _float fRadiusB = Get_SphereWorldRadius(pColB);
    const _float fRadiusSum = fRadiusA + fRadiusB;
    const _float fRadiusSumSq = fRadiusSum * fRadiusSum;

    if (fDistSq > fRadiusSumSq)
        return false;

    const _float fDist = sqrtf(fDistSq);

    const _vector vResolve_N = Safe_Normalize(vCenterA - vCenterB, Math::Right_Vec());
    const _vector vPoint = vCenterA - vResolve_N * fRadiusA;

    pOut->pColA = pColA->pCol;
    pOut->pColB = pColB->pCol;
    Math::Store(pOut->vResolveN_A, vResolve_N);
    Math::Store(pOut->vPoint, vPoint);
    pOut->fDepth = fRadiusSum - fDist;

    return true;
}

_bool CCollision_Detector::Detect_SpherePlaneCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)
{
    if (!pOut || !pColA || !pColB || !pColA->pCol || !pColB->pCol)
        return false;

    if (pColA->pCol->eShape != SHAPE::SPHERE || pColB->pCol->eShape != SHAPE::PLANE)
        return false;

    const _vector vSphereCenter = Get_ColliderCenter(pColA);
    const _float fSphereRadius = Get_SphereWorldRadius(pColA);

    const _float fSignedDist = Calculate_SignedDistToPlane(pColB, vSphereCenter);
    const _float fAbsDist = fabsf(fSignedDist);

    const _float fDiff = fAbsDist - fSphereRadius;
    const _float fEpsilon = 1e-5f;

    if (fDiff > fEpsilon)
        return false;

    const _vector vPlaneN = Get_PlaneNormal(pColB);
    const _vector vSep = (fSignedDist >= 0.f) ? vPlaneN : -vPlaneN;
    const _vector vPlanePoint = vSphereCenter - vPlaneN * fSignedDist;

    if (!pColB->plane.bInfinite)
    {
        // TODO : finite plane 영역 판정 필요
    }

    pOut->pColA = pColA->pCol;
    pOut->pColB = pColB->pCol;
    Math::Store(pOut->vResolveN_A, vSep);
    Math::Store(pOut->vPoint, vPlanePoint);
    pOut->fDepth = -fDiff;

    return true;
}

_bool CCollision_Detector::Detect_BoxPlaneCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)
{
    if (!pOut || !pColA || !pColB || !pColA->pCol || !pColB->pCol)
        return false;

    if (pColA->pCol->eShape != SHAPE::BOX || pColB->pCol->eShape != SHAPE::PLANE)
        return false;

    const _vector vBoxCenter = Get_ColliderCenter(pColA);
    const _vector vPlaneN = Get_PlaneNormal(pColB);

    const _float fSignedDist = Calculate_SignedDistToPlane(pColB, vBoxCenter);
    const _float fAbsDist = fabsf(fSignedDist);

    const _float3 vHalf = pColA->box.vHalfExtentsWorld;

    /* AABB를 plane normal에 투영한 반경 */
    const _float fBoxRad =
        fabsf(Math::Get_X(Math::Dot(vPlaneN, Math::Right_Vec()))) * vHalf.x +
        fabsf(Math::Get_X(Math::Dot(vPlaneN, Math::Up_Vec()))) * vHalf.y +
        fabsf(Math::Get_X(Math::Dot(vPlaneN, Math::Look_Vec()))) * vHalf.z;

    const _float fDiff = fAbsDist - fBoxRad;
    const _float fEpsilon = 1e-5f;
    if (fDiff > fEpsilon)
        return false;

    const _vector vSep = (fSignedDist >= 0.f) ? vPlaneN : -vPlaneN;
    const _vector vPlanePoint = vBoxCenter - vPlaneN * fSignedDist;

    if (!pColB->plane.bInfinite)
    {
        // TODO : finite plane 영역 판정 필요
    }

    pOut->pColA = pColA->pCol;
    pOut->pColB = pColB->pCol;
    Math::Store(pOut->vResolveN_A, vSep);
    Math::Store(pOut->vPoint, vPlanePoint);
    pOut->fDepth = fBoxRad - fAbsDist;

    return true;
}

_bool CCollision_Detector::Detect_BoxSphereCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)
{
    if (!pOut || !pColA || !pColB || !pColA->pCol || !pColB->pCol)
        return false;

    if (pColA->pCol->eShape != SHAPE::BOX || pColB->pCol->eShape != SHAPE::SPHERE)
        return false;

    const _vector vSphereCenter = Get_ColliderCenter(pColB);
    const _float fSphereRadius = Get_SphereWorldRadius(pColB);

    const _float3& vBoxMin = pColA->aabbWorld.vMin;
    const _float3& vBoxMax = pColA->aabbWorld.vMax;

    _float3 vClosest{};
    vClosest.x = (Math::Get_X(vSphereCenter) < vBoxMin.x) ? vBoxMin.x : ((Math::Get_X(vSphereCenter) > vBoxMax.x) ? vBoxMax.x : Math::Get_X(vSphereCenter));
    vClosest.y = (Math::Get_Y(vSphereCenter) < vBoxMin.y) ? vBoxMin.y : ((Math::Get_Y(vSphereCenter) > vBoxMax.y) ? vBoxMax.y : Math::Get_Y(vSphereCenter));
    vClosest.z = (Math::Get_Z(vSphereCenter) < vBoxMin.z) ? vBoxMin.z : ((Math::Get_Z(vSphereCenter) > vBoxMax.z) ? vBoxMax.z : Math::Get_Z(vSphereCenter));

    const _vector vClosestPoint = Math::Load(vClosest);
    const _vector vDiff = vSphereCenter - vClosestPoint;
    const _float fDistSq = Math::Get_X(XMVector3LengthSq(vDiff));
    const _float fRadiusSq = fSphereRadius * fSphereRadius;

    if (fDistSq > fRadiusSq)
        return false;

    const _float fDist = sqrtf(fDistSq);

    _vector vResolveN = Math::Right_Vec();
    _vector vPoint = vClosestPoint;
    _float fDepth = 0.f;

    if (fDist > 1e-8f)
    {
        vResolveN = XMVector3Normalize(vDiff);   /* Box -> Sphere 방향, Sphere를 Box 밖으로 밀기 위해 반대 사용 */
        vResolveN = -vResolveN;                  /* A(Box)가 B(Sphere)에게서 멀어지는 방향 */
        fDepth = fSphereRadius - fDist;
    }
    else
    {
        /* 구 중심이 박스 내부/경계에 정확히 있는 경우 */
        const _vector vBoxCenter = Get_ColliderCenter(pColA);

        const _float fDxMin = fabsf(Math::Get_X(vSphereCenter) - vBoxMin.x);
        const _float fDxMax = fabsf(vBoxMax.x - Math::Get_X(vSphereCenter));
        const _float fDyMin = fabsf(Math::Get_Y(vSphereCenter) - vBoxMin.y);
        const _float fDyMax = fabsf(vBoxMax.y - Math::Get_Y(vSphereCenter));
        const _float fDzMin = fabsf(Math::Get_Z(vSphereCenter) - vBoxMin.z);
        const _float fDzMax = fabsf(vBoxMax.z - Math::Get_Z(vSphereCenter));

        _float fBest = fDxMin;
        vResolveN = -Math::Right_Vec();
        vPoint = Math::Set_Vec(vBoxMin.x, Math::Get_Y(vSphereCenter), Math::Get_Z(vSphereCenter), 1.f);

        if (fDxMax < fBest)
        {
            fBest = fDxMax;
            vResolveN = Math::Right_Vec();
            vPoint = Math::Set_Vec(vBoxMax.x, Math::Get_Y(vSphereCenter), Math::Get_Z(vSphereCenter), 1.f);
        }
        if (fDyMin < fBest)
        {
            fBest = fDyMin;
            vResolveN = -Math::Up_Vec();
            vPoint = Math::Set_Vec(Math::Get_X(vSphereCenter), vBoxMin.y, Math::Get_Z(vSphereCenter), 1.f);
        }
        if (fDyMax < fBest)
        {
            fBest = fDyMax;
            vResolveN = Math::Up_Vec();
            vPoint = Math::Set_Vec(Math::Get_X(vSphereCenter), vBoxMax.y, Math::Get_Z(vSphereCenter), 1.f);
        }
        if (fDzMin < fBest)
        {
            fBest = fDzMin;
            vResolveN = -Math::Look_Vec();
            vPoint = Math::Set_Vec(Math::Get_X(vSphereCenter), Math::Get_Y(vSphereCenter), vBoxMin.z, 1.f);
        }
        if (fDzMax < fBest)
        {
            fBest = fDzMax;
            vResolveN = Math::Look_Vec();
            vPoint = Math::Set_Vec(Math::Get_X(vSphereCenter), Math::Get_Y(vSphereCenter), vBoxMax.z, 1.f);
        }

        /* Box를 Sphere에게서 떼는 방향 = Sphere가 Box를 향해 들어온 반대방향 */
        vResolveN = -vResolveN;
        fDepth = fSphereRadius + fBest;
    }

    pOut->pColA = pColA->pCol;
    pOut->pColB = pColB->pCol;
    Math::Store(pOut->vResolveN_A, vResolveN);
    Math::Store(pOut->vPoint, vPoint);
    pOut->fDepth = fDepth;

    return true;
}
_bool CCollision_Detector::Detect_BoxCollision(CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)
{
    if (!pOut || !pColA || !pColB || !pColA->pCol || !pColB->pCol)
        return false;

    if (pColA->pCol->eShape != SHAPE::BOX || pColB->pCol->eShape != SHAPE::BOX)
        return false;

    const _float3& aMin = pColA->aabbWorld.vMin;
    const _float3& aMax = pColA->aabbWorld.vMax;
    const _float3& bMin = pColB->aabbWorld.vMin;
    const _float3& bMax = pColB->aabbWorld.vMax;

    const _float fOverlapX = min(aMax.x, bMax.x) - max(aMin.x, bMin.x);
    if (fOverlapX <= 0.f)
        return false;

    const _float fOverlapY = min(aMax.y, bMax.y) - max(aMin.y, bMin.y);
    if (fOverlapY <= 0.f)
        return false;

    const _float fOverlapZ = min(aMax.z, bMax.z) - max(aMin.z, bMin.z);
    if (fOverlapZ <= 0.f)
        return false;

    const _vector vCenterA = Get_ColliderCenter(pColA);
    const _vector vCenterB = Get_ColliderCenter(pColB);
    const _vector vDelta = vCenterA - vCenterB;

    _vector vResolveN = Math::Right_Vec();
    _float fDepth = fOverlapX;

    const _float fDx = Math::Get_X(vDelta);
    const _float fDy = Math::Get_Y(vDelta);
    const _float fDz = Math::Get_Z(vDelta);

    if (fOverlapY < fDepth)
    {
        fDepth = fOverlapY;
        vResolveN = (fDy >= 0.f) ? Math::Up_Vec() : -Math::Up_Vec();
    }
    else
    {
        vResolveN = (fDx >= 0.f) ? Math::Right_Vec() : -Math::Right_Vec();
    }

    if (fOverlapZ < fDepth)
    {
        fDepth = fOverlapZ;
        vResolveN = (fDz >= 0.f) ? Math::Look_Vec() : -Math::Look_Vec();
    }

    const _vector vPoint = (vCenterA + vCenterB) * 0.5f;

    pOut->pColA = pColA->pCol;
    pOut->pColB = pColB->pCol;
    Math::Store(pOut->vResolveN_A, vResolveN);
    Math::Store(pOut->vPoint, vPoint);
    pOut->fDepth = fDepth;

    return true;
}

void CCollision_Detector::Register_DetectTable()
{
    /* SPHERE -> */
    m_DetectTable[To<size_t>(SHAPE::SPHERE)][To<size_t>(SHAPE::SPHERE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_SphereCollision(pOut, pColA, pColB); };

    m_DetectTable[To<size_t>(SHAPE::SPHERE)][To<size_t>(SHAPE::BOX)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_BoxPlaneCollision(pOut, pColB, pColA); };

    m_DetectTable[To<size_t>(SHAPE::SPHERE)][To<size_t>(SHAPE::PLANE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_SpherePlaneCollision(pOut, pColA, pColB); };

    /* BOX -> */
    m_DetectTable[To<size_t>(SHAPE::BOX)][To<size_t>(SHAPE::SPHERE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_BoxSphereCollision(pOut, pColA, pColB); };

    m_DetectTable[To<size_t>(SHAPE::BOX)][To<size_t>(SHAPE::BOX)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_BoxCollision(pOut, pColA, pColB); };

    m_DetectTable[To<size_t>(SHAPE::BOX)][To<size_t>(SHAPE::PLANE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_BoxPlaneCollision(pOut, pColA, pColB); };

    /* PLANE -> */
    m_DetectTable[To<size_t>(SHAPE::PLANE)][To<size_t>(SHAPE::SPHERE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_SpherePlaneCollision(pOut, pColB, pColA); };

    m_DetectTable[To<size_t>(SHAPE::PLANE)][To<size_t>(SHAPE::BOX)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_BoxPlaneCollision(pOut, pColB, pColA); };

    m_DetectTable[To<size_t>(SHAPE::PLANE)][To<size_t>(SHAPE::PLANE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return false; /* NOT IMPLEMENTED */ };
}

void CCollision_Detector::Fill_ContactInfo(CONTACT_DESC& contact)
{
    contact.tCollisionA.hObject = contact.pColB->hObject;
    contact.tCollisionA.pCounterCollider = contact.pColB;
    contact.tCollisionA.vPoint = contact.vPoint;

    contact.tCollisionB.hObject = contact.pColA->hObject;
    contact.tCollisionB.pCounterCollider = contact.pColA;
    contact.tCollisionB.vPoint = contact.vPoint;
}

std::unique_ptr<CCollision_Detector> CCollision_Detector::Create(CPhysics_Processor* pPhysics)
{
    auto pIntance = std::make_unique<CCollision_Detector>(pPhysics);
    IF_FAIL_RETURN_MSG_BREAK(pIntance->Initialize(), nullptr, "Insatnace create failed");
    return pIntance;
}
