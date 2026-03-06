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
    return false;
}

void CCollision_Detector::Register_DetectTable()
{
    /* SPHERE -> */
    m_DetectTable[To<size_t>(SHAPE::SPHERE)][To<size_t>(SHAPE::SPHERE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_SphereCollision(pOut, pColA, pColB); };

    m_DetectTable[To<size_t>(SHAPE::SPHERE)][To<size_t>(SHAPE::BOX)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return false; /* NOT IMPLEMENTED */ };

    m_DetectTable[To<size_t>(SHAPE::SPHERE)][To<size_t>(SHAPE::PLANE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return Detect_SpherePlaneCollision(pOut, pColA, pColB); };

    /* BOX -> */
    m_DetectTable[To<size_t>(SHAPE::BOX)][To<size_t>(SHAPE::SPHERE)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return false; /* NOT IMPLEMENTED */ };

    m_DetectTable[To<size_t>(SHAPE::BOX)][To<size_t>(SHAPE::BOX)] =
        [this](CONTACT_DESC* pOut, COLLIDER_PROXY_DATA* pColA, COLLIDER_PROXY_DATA* pColB)->_bool { return false; /* NOT IMPLEMENTED */ };

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
