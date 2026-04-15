#include "Physics_Processor.h"

#include "Transform_Processor.h"
#include "Component_System.h"

#include "Collision_Detector.h"
#include "Collider_Proxy_Builder.h"
#include "Component_Spec.h"
#include "Debug_Renderer.h"
#include "Rigidbody_Builder.h"
#include "GameObject.h"
#include "Solver.h"
#include "Uniform_Grid.h"
#include "Editor_System.h"

HRESULT CPhysics_Processor::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_pDevice = pDevice;
    m_pContext = pContext;

    /* 팩토리 등록 */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CCollider, COLLIDER_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CCollider>();

        SYS_COMPONENT.Register_InitialSpecFactory<CRigidbody, RIGIDBODY_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CRigidbody>();

        SYS_COMPONENT.Register_InitialSpecFactory<CSpringJoint, SPRING_JOINT_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CSpringJoint>();
    }

    m_pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransformProcessor, E_FAIL, "Transform Processor bind failed");

    m_upUniform_Grid = CUniform_Grid::Create({ -240.f, -30.f, -240.f }, { 240.f, 30.f, 240.f }, 60.f);
    m_upCollision_Detector = CCollision_Detector::Create(this);
    m_upCollider_Builder = CCollider_Proxy_Builder::Create();
    m_upRigidbody_Builder = CRigidbody_Builder::Create();
    m_upSolver = CSolver::Create();
    m_upDebugRenderer = CDebug_Renderer::Create(m_pDevice, m_pContext);

    return S_OK;
}

void CPhysics_Processor::LateUpdate(_float fDT)
{

}

void CPhysics_Processor::Fixed_Update(_float fDT)
{
    m_ActivatedColliders.clear();

    /* spring joint */
    Process_SpringJoints(fDT);

    Accumulate_Forces();
    Integrate_Forces(fDT);
    Apply_Damping(fDT);

    /* 적분 적용하기 */
    Integrate_Velocities(fDT);

    /* 충돌 감지하기 */
    vector<CONTACT_DESC> outContacts;
    Process_Collision(outContacts);

    /* 충돌 해결하기 */
    for (_uint i = 0; i < 1; ++i)
    {
        for (auto& contact : outContacts)
        {
            if (contact.pColA->bTrigger || contact.pColB->bTrigger)
                continue;
            m_upSolver->Solve_Contacts(&contact);
        }
    }

    /* kinematic 해당 프레임 속도 초기화 */
    Reset_Kinematic_Velocities();
}

void CPhysics_Processor::Render()
{
    if (m_eDebugDraw == DEBUG_DRAW::NONE)
        return;

    const _bool bDrawGrid =
        (m_eDebugDraw == DEBUG_DRAW::ALL_GRID ||
         m_eDebugDraw == DEBUG_DRAW::SELECT_GRID);

    const _bool bDrawNav =
        (m_eDebugDraw == DEBUG_DRAW::ALL_NAV ||
         m_eDebugDraw == DEBUG_DRAW::SELECT_NAV ||
         m_eDebugDraw == DEBUG_DRAW::NAV);

    const _bool bDrawCollider =
        (m_eDebugDraw == DEBUG_DRAW::ALL ||
         m_eDebugDraw == DEBUG_DRAW::SELECT ||
         m_eDebugDraw == DEBUG_DRAW::ALL_GRID ||
         m_eDebugDraw == DEBUG_DRAW::SELECT_GRID ||
         m_eDebugDraw == DEBUG_DRAW::ALL_NAV ||
         m_eDebugDraw == DEBUG_DRAW::SELECT_NAV);

    const _bool bSelectOnly =
        (m_eDebugDraw == DEBUG_DRAW::SELECT ||
         m_eDebugDraw == DEBUG_DRAW::SELECT_GRID ||
         m_eDebugDraw == DEBUG_DRAW::SELECT_NAV);

    m_upDebugRenderer->Begin();

    /* -------- COLLIDER -------- */
    if (bDrawCollider)
    {
        for (const auto& tProxy : m_ActivatedColliders)
        {
            if (tProxy.pCol == nullptr)
                continue;
            if (!tProxy.pCol->bEnable)
                continue;

            if (bSelectOnly && !tProxy.pCol->bDebugDraw)
                continue;

            m_upDebugRenderer->Draw_Collider(tProxy);
        }
    }

    /* -------- UNIFORM GRID -------- */
    if (bDrawGrid && m_upUniform_Grid)
    {
        for (int iZ = 0; iZ < m_upUniform_Grid->Get_DimZ(); ++iZ)
        {
            for (int iY = 0; iY < m_upUniform_Grid->Get_DimY(); ++iY)
            {
                for (int iX = 0; iX < m_upUniform_Grid->Get_DimX(); ++iX)
                {
                    _float3 vCellMin{};
                    _float3 vCellMax{};
                    m_upUniform_Grid->Calc_CellMinMax(iX, iY, iZ, &vCellMin, &vCellMax);

                    AABB tCellAABB{};
                    tCellAABB.vMin = vCellMin;
                    tCellAABB.vMax = vCellMax;

                    m_upDebugRenderer->Draw_AABB(tCellAABB, Colors::Gray);
                }
            }
        }

        const auto& vecQueriedCells = m_upUniform_Grid->Get_DebugFrameQueriedCells();
        for (const auto& tCell : vecQueriedCells)
        {
            _float3 vCellMin{};
            _float3 vCellMax{};
            m_upUniform_Grid->Calc_CellMinMax(tCell.iX, tCell.iY, tCell.iZ, &vCellMin, &vCellMax);

            AABB tCellAABB{};
            tCellAABB.vMin = vCellMin;
            tCellAABB.vMax = vCellMax;

            m_upDebugRenderer->Draw_AABB(tCellAABB, Colors::Green);
        }
    }

    /* -------- NAV -------- */
    if (bDrawNav)
    {
        SYS_EDITOR.Render_NavCells(m_upDebugRenderer.get());
    }

    m_upDebugRenderer->End();
}

void CPhysics_Processor::Process_SpringJoints(_float fDT)
{
    UNREFERENCED_PARAMETER(fDT);

    const auto& springJointPages = m_SpringJointPool.GetPages();

    for (const auto& upPage : springJointPages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            SPRING_JOINT_DATA* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable || !pData->bUseSpring)
                continue;
            CRigidbody rigidbody = m_RigidbodyPool.Get_Proxy(pData->hRigidbody);
            if (!rigidbody.Is_Valid())
                continue;
            if (false == rigidbody->bEnable|| rigidbody->eBodyType != BODY_TYPE::DYNAMIC)
                continue;

            TRANSFORM_DATA* pTrData = m_pTransformProcessor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, rigidbody->hTransform)._Data();
            if (!pTrData)
                continue;

            const _vector vPos = Math::Load(pTrData->vPosition);
            const _vector vAnchor = Math::Load(pData->vAnchor);

            const _vector vDir = vPos - vAnchor;
            const _float fDist = Math::Get_X(XMVector3Length(vDir));

            if (fDist <= 1e-4f)
                continue;

            _float fTargetLength = pData->fRestLength;

            if (pData->bUseMinLength && fTargetLength < pData->fMinLength)
                fTargetLength = pData->fMinLength;

            if (pData->bUseMaxLength && fTargetLength > pData->fMaxLength)
                fTargetLength = pData->fMaxLength;

            /* 로프처럼 늘어났을 때만 힘을 준다 */
            const _float fX = fDist - fTargetLength;
            if (fX <= 0.f)
                continue;

            const _vector vN = XMVector3Normalize(vDir);
            const _vector vLinearVel = Math::Load(rigidbody->vLinearVel);

            /* 앵커 방향 축에서의 속도 성분 */
            const _float fV = XMVectorGetX(XMVector3Dot(vLinearVel, vN));

            /* F = -k x - c v */
            const _float fForceMag = (-pData->fSpring * fX) - (pData->fDamper * fV);
            _vector vForce = vN * fForceMag;

            _float3 vForce3{};
            Math::Store(vForce3, vForce);
            vForce3.y *= 2.f;

            {
                _float3 vN3{};
                _float3 vVel3{};

                Math::Store(vN3, vN);
                Math::Store(vVel3, vLinearVel);

                const _float fDotVelForce = XMVectorGetX(XMVector3Dot(vLinearVel, vForce));
            }

            rigidbody.Add_Force(vForce3);
        }
    }
}

void CPhysics_Processor::Accumulate_Forces()
{
    const auto& rigidbodyPages = m_RigidbodyPool.GetPages();

    for (const auto& upPage : rigidbodyPages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            RIGIDBODY_DATA* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            if (pData->eBodyType != BODY_TYPE::DYNAMIC)
                continue;

            if (pData->bGravity)
            {
                const _float fMass = (pData->fInvMass > 0.f) ? (1.f / pData->fInvMass) : pData->fMass;
                _vector vForceAccum = Math::Load(pData->vForceAccum);
                _vector vGravity = Math::Load(m_vGravity);
                Math::Store(pData->vForceAccum, vForceAccum + vGravity * fMass);
                __noop;
            }
        }
    }
}

void CPhysics_Processor::Integrate_Forces(_float fDT)
{
    const auto& rigidbodyPages = m_RigidbodyPool.GetPages();

    for (const auto& upPage : rigidbodyPages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            RIGIDBODY_DATA* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            if (pData->eBodyType != BODY_TYPE::DYNAMIC)
                continue;

            if (pData->bDirtyMass || pData->bDirtyInertia || pData->bDirtyWorldInertia)
            {
                if(FAILED(m_upRigidbody_Builder->Rebuild(pData)))
                    continue;   
            }

            /* Linear */
            _vector vForceAccum = Math::Load(pData->vForceAccum);
            if (!Math::Is_Zero(vForceAccum))
            {
                const _vector vLinearVel = Math::Load(pData->vLinearVel);
                Math::Store(pData->vLinearVel, vLinearVel + vForceAccum * pData->fInvMass * fDT);
            }

            /* Angular */
            const _vector vTorqueAccum = Math::Load(pData->vTorqueAccum);
            if (!Math::Is_Zero(vTorqueAccum))
            {
                const _matrix matInvWorldInertia = Math::Load(pData->matWorldInvInertiaTensor);
                const _vector vDeltaW = XMVector3TransformNormal(vTorqueAccum, matInvWorldInertia);

                const _vector vAngularVel = Math::Load(pData->vAngularVel);
                Math::Store(pData->vAngularVel, vAngularVel + vDeltaW * fDT);
            }

            /* Lock 적용 */
            Apply_PositionLock(*pData);
            Apply_RotationLock(*pData);

            /* accum 값은 해당 프레임에만 적용된다. */
            pData->vForceAccum = _float3{ 0.f, 0.f, 0.f };
            pData->vTorqueAccum = _float3{ 0.f, 0.f, 0.f };
        }
    }
}

void CPhysics_Processor::Apply_Damping(_float fDT)
{
    const auto& rigidbodyPages = m_RigidbodyPool.GetPages();
    constexpr _float fDampEps = 1e-4f;

    for (const auto& upPage : rigidbodyPages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            RIGIDBODY_DATA* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            if (pData->eBodyType != BODY_TYPE::DYNAMIC)
                continue;

            /* Linear damping */
            if (pData->fDrag > 0.f)
            {
                const _vector vLinearVel = Math::Load(pData->vLinearVel);
                const _vector vDampedLinearVel = vLinearVel - (vLinearVel * pData->fDrag * fDT);
                if (XMVectorGetX(XMVector3LengthSq(vDampedLinearVel)) < (fDampEps * fDampEps))
                    pData->vLinearVel = Math::Zero3();
                else
                    Math::Store(pData->vLinearVel, vDampedLinearVel);
            }

            /* Linear damping */
            //if (pData->fDrag > 0.f)
            //{
            //    const _vector vLinearVel = Math::Load(pData->vLinearVel);
            //    const _vector vDampedLinearVel = vLinearVel - (vLinearVel * pData->fDrag * fDT);

            //    if (pData->bDebugLog)
            //    {
            //        _float3 vLinearVel3{};
            //        _float3 vDampedLinearVel3{};

            //        Math::Store(vLinearVel3, vLinearVel);
            //        Math::Store(vDampedLinearVel3, vDampedLinearVel);

            //        LOG_INFO("[DAMP] VelBefore : %.2f, %.2f, %.2f",
            //            vLinearVel3.x, vLinearVel3.y, vLinearVel3.z);
            //        LOG_INFO("[DAMP] VelAfter  : %.2f, %.2f, %.2f",
            //            vDampedLinearVel3.x, vDampedLinearVel3.y, vDampedLinearVel3.z);
            //    }

            //    if (XMVectorGetX(XMVector3LengthSq(vDampedLinearVel)) < (fDampEps * fDampEps))
            //        pData->vLinearVel = Math::Zero3();
            //    else
            //        Math::Store(pData->vLinearVel, vDampedLinearVel);
            //}

            /* Angular damping */
            if (pData->fAngularDrag > 0.f)
            {
                const _vector vAngularVel = Math::Load(pData->vAngularVel);
                const _vector vDampedAngularVel = vAngularVel - (vAngularVel * pData->fAngularDrag * fDT);

                if (XMVectorGetX(XMVector3LengthSq(vDampedAngularVel)) < (fDampEps * fDampEps))
                    pData->vAngularVel = Math::Zero3();
                else
                    Math::Store(pData->vAngularVel, vDampedAngularVel);
            }
        }
    }
}


void CPhysics_Processor::Integrate_Velocities(_float fDT)
{
    const auto& rigidbodyPages = m_RigidbodyPool.GetPages();

    for (const auto& upPage : rigidbodyPages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            RIGIDBODY_DATA* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable || pData->eBodyType != BODY_TYPE::DYNAMIC)
                continue;

            CTransform transform = m_pTransformProcessor->Get_Proxy(COMPONENT_TYPE::TRANSFORM, pData->hTransform);
            TRANSFORM_DATA* pTrData = transform._Data();
            if (!pTrData)
                continue;

            /* Linear integration */
            const _vector vLinearVel = Math::Load(pData->vLinearVel);
            const _vector vDeltaPos = vLinearVel * fDT;

            transform.Translate(vDeltaPos, SPACE::WORLD);

            Math::Store(pData->vCOM, Math::Load(pData->vCOM) + vDeltaPos);
            pData->vWorldCOM = pTrData->vPosition;

            /* Angular integration */
            const _vector vAngularVel = Math::Load(pData->vAngularVel);
            if (!Math::Is_Zero(vAngularVel))
            {
                const _vector vAxis = XMVector3Normalize(vAngularVel);
                const _float fAngle = XMVectorGetX(XMVector3Length(vAngularVel)) * fDT;

                transform.Rotate(vAxis, fAngle);
                pData->bDirtyWorldInertia = true;
            }
        }
    }
}

void CPhysics_Processor::Process_Collision(vector<CONTACT_DESC>& outContacts)
{
    m_ActivatedColliders.clear();

    /* TODO : 가능하면 active count로 reserve 해두기 */

    const auto& ColliderPage = m_ColliderPool.GetPages();
    for (const auto& upPage : ColliderPage)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            COLLIDER_PROXY_DATA outData{};
            m_upCollider_Builder->Build_Collider_Proxy(pData, outData);
            pData->bDirty = false;

            m_ActivatedColliders.push_back(std::move(outData));
        }
    }

    if (m_upUniform_Grid)
    {
        m_upUniform_Grid->Begin_Debug_Frame();
    }

    vector<COLLIDER_PAIR> outPair;
    m_CurPair.clear();

    m_upCollision_Detector->Generate_BroadPhase_Pairs(m_ActivatedColliders, outPair);
    m_upCollision_Detector->Process_NarrowPhase(outPair, outContacts, m_CurPair);

    Invoke_CollisionEvent();
}

void CPhysics_Processor::Invoke_CollisionEvent()
{
    /* Enter / Stay */
    for (const PAIR_KEY& k : m_CurPair)
    {
        auto* pA = m_ColliderPool.Get_Data_By_Handle(COMPONENT_HANDLE{ k.aKey });
        auto* pB = m_ColliderPool.Get_Data_By_Handle(COMPONENT_HANDLE{ k.bKey });
        if (!pA || !pB)
            continue;

        COLLISION_DESC tA{};
        COLLISION_DESC tB{};

        tA.hObject = pB->hObject;
        tA.pCounterCollider = pB;

        tB.hObject = pA->hObject;
        tB.pCounterCollider = pA;

        const _bool bTriggerPair = pA->bTrigger || pB->bTrigger;
        const _bool bEnter = (m_prevPair.find(k) == m_prevPair.end()); /* 이전에 충돌한 적이 없다면 Enter */

        if (bEnter)
        {
            if (bTriggerPair)
            {
                pA->OnTriggerEnter.Invoke(tA);
                pB->OnTriggerEnter.Invoke(tB);
            }
            else
            {
                pA->OnCollisionEnter.Invoke(tA);
                pB->OnCollisionEnter.Invoke(tB);
            }
        }
        else
        {
            /* 이전에 충돌한 적이 있다면 Stay */
            if (bTriggerPair)
            {
                pA->OnTriggerStay.Invoke(tA);
                pB->OnTriggerStay.Invoke(tB);
            }
            else
            {
                pA->OnCollisionStay.Invoke(tA);
                pB->OnCollisionStay.Invoke(tB);
            }
        }
    }

    /* Exit */
    for (const PAIR_KEY& k : m_prevPair)
    {
        /* 현재 충돌 중이라면 continue */
        if (m_CurPair.find(k) != m_CurPair.end())
            continue;

        auto* pA = m_ColliderPool.Get_Data_By_Handle(COMPONENT_HANDLE{ k.aKey });
        auto* pB = m_ColliderPool.Get_Data_By_Handle(COMPONENT_HANDLE{ k.bKey });
        if (!pA || !pB)
            continue;

        COLLISION_DESC tA{};
        COLLISION_DESC tB{};

        tA.hObject = pB->hObject;
        tA.pCounterCollider = pB;

        tB.hObject = pA->hObject;
        tB.pCounterCollider = pA;

        const _bool bTriggerPair = pA->bTrigger || pB->bTrigger;
        if (bTriggerPair)
        {
            pA->OnTriggerExit.Invoke(tA);
            pB->OnTriggerExit.Invoke(tB);
        }
        else
        {
            pA->OnCollisionExit.Invoke(tA);
            pB->OnCollisionExit.Invoke(tB);
        }
    }

    /* prev <-> cur 교체 */
    m_prevPair.swap(m_CurPair);
    m_CurPair.clear();
}

void CPhysics_Processor::Reset_Kinematic_Velocities()
{
    const auto& rigidbodyPages = m_RigidbodyPool.GetPages();

    for (const auto& upPage : rigidbodyPages)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            RIGIDBODY_DATA* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            if (pData->eBodyType != BODY_TYPE::KINEMATIC)
                continue;

            pData->vLinearVel = Math::Zero3();
            pData->vAngularVel = Math::Zero3();
        }
    }
}

void CPhysics_Processor::Apply_RotationLock(RIGIDBODY_DATA& data)
{
    if (data.tRotationLock.bX) data.vAngularVel.x = 0.f;
    if (data.tRotationLock.bY) data.vAngularVel.y = 0.f;
    if (data.tRotationLock.bZ) data.vAngularVel.z = 0.f;
}

void CPhysics_Processor::Apply_PositionLock(RIGIDBODY_DATA& data)
{
    if (data.tPositionLock.bX) data.vLinearVel.x = 0.f;
    if (data.tPositionLock.bY) data.vLinearVel.y = 0.f;
    if (data.tPositionLock.bZ) data.vLinearVel.z = 0.f;
}

_bool CPhysics_Processor::Detect_Raycast(RAY& tRay, RAYCAST_HITS& outHits)
{
    return m_upCollision_Detector->Detect_Raycast(tRay, m_ActivatedColliders, outHits);
}

const COLLIDER_PROXY_DATA* CPhysics_Processor::Find_ActivatedCollider_ByHandle(COMPONENT_HANDLE hCollider) const
{
    for (const auto& tProxy : m_ActivatedColliders)
    {
        if (!tProxy.pCol)
            continue;

        if (tProxy.pCol->hSelf == hCollider)
            return &tProxy;
    }

    return nullptr;
}

void CPhysics_Processor::Rebuild_Static_Grid()
{
    if (!m_upUniform_Grid)
        return;

    m_upUniform_Grid->Clear();

    const auto& ColliderPage = m_ColliderPool.GetPages();
    for (const auto& upPage : ColliderPage)
    {
        auto* pPage = upPage.get();
        if (!pPage)
            continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable)
                continue;

            if (!pData->bStatic)
                continue;

            COLLIDER_PROXY_DATA tProxy{};
            m_upCollider_Builder->Build_Collider_Proxy(pData, tProxy);

            m_upUniform_Grid->Insert_Static(pData->hSelf, tProxy.aabbWorld);
        }
    }
}

void CPhysics_Processor::Set_DrawMode(DEBUG_DRAW eDraw)
{
    m_eDebugDraw = eDraw;
}

COMPONENT_HANDLE CPhysics_Processor::Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
        return Create_Component_Data_Inner<CCollider>(m_ColliderPool, hObject);
    case COMPONENT_TYPE::RIGIDBODY:
        return Create_Component_Data_Inner<CRigidbody>(m_RigidbodyPool, hObject);
    case COMPONENT_TYPE::SPRING_JOINT:
        return Create_Component_Data_Inner<CSpringJoint>(m_SpringJointPool, hObject);
    default:
        return COMPONENT_HANDLE{};
    }
}

void CPhysics_Processor::Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
        return Remove_Component_Inner<CCollider>(m_ColliderPool, hComponent);
    case COMPONENT_TYPE::RIGIDBODY:
        return Remove_Component_Inner<CRigidbody>(m_RigidbodyPool, hComponent);
    case COMPONENT_TYPE::SPRING_JOINT:
        return Remove_Component_Inner<CSpringJoint>(m_SpringJointPool, hComponent);
    }
}

HRESULT CPhysics_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
        return Initialize_From_Spec_Collider(hComponent, pSpec);
    case COMPONENT_TYPE::RIGIDBODY:
        return Initialize_From_Spec_Rigidbody(hComponent, pSpec);
    case COMPONENT_TYPE::SPRING_JOINT:
        return Initialize_From_Spec_SpringJoint(hComponent, pSpec);
    default:
        return E_FAIL;
    }
}

std::unique_ptr<COMPONENT_SPEC_BASE> CPhysics_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
        return Build_Spec_Collider(hComponent);
    case COMPONENT_TYPE::RIGIDBODY:
        return Build_Spec_Rigidbody(hComponent);
    case COMPONENT_TYPE::SPRING_JOINT:
        return Build_Spec_SpringJoint(hComponent);
    }
    return nullptr;
}

void CPhysics_Processor::Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
    {
        COLLIDER_DATA* pData = m_ColliderPool.Get_Data_By_Handle(hComponent);
        pData->bEnable = bEnable;
        return;
    }
    case COMPONENT_TYPE::RIGIDBODY:
    {
        RIGIDBODY_DATA* pData = m_RigidbodyPool.Get_Data_By_Handle(hComponent);
        pData->bEnable = bEnable;
        return;
    }
    case COMPONENT_TYPE::SPRING_JOINT:
    {
        SPRING_JOINT_DATA* pData = m_SpringJointPool.Get_Data_By_Handle(hComponent);
        pData->bEnable = bEnable;
        return;
    }
    default:
        break;
    }

    _DEBUG_WARN("CPhysics_Processor::Remove_Component - unsupported component type");
}

void* CPhysics_Processor::Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:  return m_ColliderPool.Get_Data_By_Handle(hComponent);
    case COMPONENT_TYPE::RIGIDBODY:  return m_RigidbodyPool.Get_Data_By_Handle(hComponent);
    case COMPONENT_TYPE::SPRING_JOINT:  return m_SpringJointPool.Get_Data_By_Handle(hComponent);
    default: return nullptr;
    }
}

HRESULT CPhysics_Processor::Initialize_From_Spec_Collider(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{
    COLLIDER_DATA* pData = m_ColliderPool.Get_Data_By_Handle(h);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid Collider handle in Initialize_From_Spec_Collider");
    _DEBUG_ENGINE_ASSERT_MSG(spec != nullptr, "spec is nullptr in Initialize_From_Spec_Collider");

    const COLLIDER_SPEC* pSpec = SCAST(const COLLIDER_SPEC*, spec);
    pData->hSelf = h;
    pData->bEnable = pSpec->bEnable;
    pData->bOnCol = pSpec->bOnCol;
    pData->bDebugDraw = pSpec->bDebugDraw;
    pData->bTrigger = pSpec->bTrigger;
    pData->eShape = pSpec->eShape;
    pData->vOffset = pSpec->vOffset;
    pData->vRotationOffset = pSpec->vRotationOffset;
    pData->bStatic = pSpec->bStatic;
    pData->iMask = pSpec->iMask;
    pData->iDiscardMask = pSpec->iDiscardMask;

    switch (pSpec->eShape)
    {
    case SHAPE::BOX:
        pData->box.vHalfExtentsLocal = pSpec->vHalfExtentsLocal;
        break;

    case SHAPE::SPHERE:
        pData->sphere.fRadiusLocal = pSpec->fRadiusLocal;
        break;

    case SHAPE::PLANE:
        pData->plane.vNormalLocal = pSpec->vNormalLocal;
        pData->plane.bInfinite = pSpec->bInfinite;
        pData->plane.vDimension = pSpec->vDimension;
        break;

    default:
        _DEBUG_ENGINE_ASSERT_MSG(false, "Invalid collider shape in Initialize_From_Spec_Collider");
        return E_FAIL;
    }

    pData->bDirty = true;

    return S_OK;
}

HRESULT CPhysics_Processor::Initialize_From_Spec_Rigidbody(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{
    RIGIDBODY_DATA* pData = m_RigidbodyPool.Get_Data_By_Handle(h);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid Rigidbody handle in Initialize_From_Spec_Rigidbody");
    _DEBUG_ENGINE_ASSERT_MSG(spec != nullptr, "spec is nullptr in Initialize_From_Spec_Rigidbody");

    const RIGIDBODY_SPEC* pSpec = SCAST(const RIGIDBODY_SPEC*, spec);

    pData->bEnable = pSpec->bEnable;
    pData->bGravity = pSpec->bGravity;

    pData->eShape = pSpec->eShape;
    pData->eBodyType = pSpec->eBodyType;

    pData->fMass = pSpec->fMass;
    pData->fDrag = pSpec->fDrag;
    pData->fAngularDrag = pSpec->fAngularDrag;
    pData->fRestitution = pSpec->fRestitution;
    pData->fFriction = pSpec->fFriction;

    pData->tRotationLock = pSpec->tRotationLock;
    pData->tPositionLock = pSpec->tPositionLock;

    pData->bDirtyMass = true;
    pData->bDirtyInertia = true;
    pData->bDirtyWorldInertia = true;
    pData->bInitialized = false;

    return S_OK;
}

HRESULT CPhysics_Processor::Initialize_From_Spec_SpringJoint(COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* spec)
{
    const SPRING_JOINT_SPEC* pSpec = To<const SPRING_JOINT_SPEC*>(spec);
    IF_NULL_RETURN_MSG_BREAK(pSpec, E_FAIL, "pSpec is nullptr");

    SPRING_JOINT_DATA* pData = m_SpringJointPool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "pData is nullptr");

    /* 저장한 값 채우기 */
    pData->bEnable = pSpec->bEnable;
    pData->bUseSpring = pSpec->bUseSpring;
    pData->vAnchor = pSpec->vAnchor;
    pData->fSpring = pSpec->fSpring;
    pData->fDamper = pSpec->fDamper;
    pData->fRestLength = pSpec->fRestLength;
    pData->fMinLength = pSpec->fMinLength;
    pData->fMaxLength = pSpec->fMaxLength;
    pData->bUseMinLength = pSpec->bUseMinLength;
    pData->bUseMaxLength = pSpec->bUseMaxLength;

    /* 런타임 연결값은 spec에서 복원하지 않는다 */
    pData->hRigidbody = INVALID_HANDLE;

    CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
    if (pObj)
    {
        CRigidbody rigidbody = pObj->Get_Component<CRigidbody>();
        if (rigidbody.Is_Valid())
            pData->hRigidbody = rigidbody.Get_Handle();
    }

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CPhysics_Processor::Build_Spec_Collider(COMPONENT_HANDLE hComponent)
{
    COLLIDER_DATA* pData = m_ColliderPool.Get_Data_By_Handle(hComponent);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid Collider handle in Build_Spec_Collider");

    auto pSpec = std::make_unique<COLLIDER_SPEC>();

    pSpec->bEnable = pData->bEnable;
    pSpec->bOnCol = pData->bOnCol;
    pSpec->bDebugDraw = pData->bDebugDraw;
    pSpec->bTrigger = pData->bTrigger;
    pSpec->eShape = pData->eShape;
    pSpec->vOffset = pData->vOffset;
    pSpec->vRotationOffset = pData->vRotationOffset;
    pSpec->bStatic = pData->bStatic;
    pSpec->iMask = pData->iMask;
    pSpec->iDiscardMask = pData->iDiscardMask;

    switch (pData->eShape)
    {
    case SHAPE::BOX:
        pSpec->vHalfExtentsLocal = pData->box.vHalfExtentsLocal;
        return pSpec;

    case SHAPE::SPHERE:
        pSpec->fRadiusLocal = pData->sphere.fRadiusLocal;
        return pSpec;

    case SHAPE::PLANE:
        pSpec->vNormalLocal = pData->plane.vNormalLocal;
        pSpec->bInfinite = pData->plane.bInfinite;
        pSpec->vDimension = pData->plane.vDimension;
        return pSpec;
    }

    return nullptr;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CPhysics_Processor::Build_Spec_Rigidbody(COMPONENT_HANDLE hComponent)
{
    RIGIDBODY_DATA* pData = m_RigidbodyPool.Get_Data_By_Handle(hComponent);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid Rigidbody handle in Build_Spec_Rigidbody");
    auto pSpec = std::make_unique<RIGIDBODY_SPEC>();

    pSpec->bEnable = pData->bEnable;
    pSpec->bGravity = pData->bGravity;

    pSpec->eShape = pData->eShape;
    pSpec->eBodyType = pData->eBodyType;

    pSpec->fMass = pData->fMass;
    pSpec->fDrag = pData->fDrag;
    pSpec->fAngularDrag = pData->fAngularDrag;
    pSpec->fRestitution = pData->fRestitution;
    pSpec->fFriction = pData->fFriction;

    pSpec->tRotationLock = pData->tRotationLock;
    pSpec->tPositionLock = pData->tPositionLock;

    return pSpec;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CPhysics_Processor::Build_Spec_SpringJoint(COMPONENT_HANDLE hComponent)
{
    SPRING_JOINT_DATA* pData = m_SpringJointPool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "pData is nullptr");

    auto pSpec = std::make_unique<SPRING_JOINT_SPEC>();

    pSpec->bEnable = pData->bEnable;
    pSpec->bUseSpring = pData->bUseSpring;

    pSpec->vAnchor = pData->vAnchor;

    pSpec->fSpring = pData->fSpring;
    pSpec->fDamper = pData->fDamper;
    pSpec->fRestLength = pData->fRestLength;

    pSpec->fMinLength = pData->fMinLength;
    pSpec->fMaxLength = pData->fMaxLength;

    pSpec->bUseMinLength = pData->bUseMinLength;
    pSpec->bUseMaxLength = pData->bUseMaxLength;

    return pSpec;
}

HRESULT CPhysics_Processor::Initialize_Component_Data(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
    {
        COLLIDER_DATA* pData = m_ColliderPool.Get_Data_By_Handle(hComponent);
        IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "Can't find data.");

        /* Transform 핸들을 캐싱한다. */
        CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Can't find pObj.");

        CTransform transform = pObj->Get_Component<CTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!transform.Is_Valid(), E_FAIL, "Transform is invalid.");

        TRANSFORM_DATA* pTr = transform._Data();
        IF_NULL_RETURN_MSG_BREAK(pTr, E_FAIL, "Transform data is nullptr");

        /* 값 채우기 */
        pData->hTransform = transform.Get_Handle();
        pData->hRigidbody = {}; /* TODO : 로직 생각해보기 */
        pData->bDebugDraw = false;

        pData->bDirty = true;
        return S_OK;
    }
    case COMPONENT_TYPE::RIGIDBODY:
    {
        RIGIDBODY_DATA* pData = m_RigidbodyPool.Get_Data_By_Handle(hComponent);
        IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "Can't find rigidbody data.");

        CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Can't find pObj.");

        CTransform transform = pObj->Get_Component<CTransform>();
        IF_TRUE_RETURN_MSG_BREAK(!transform.Is_Valid(), E_FAIL, "Transform is invalid.");

        TRANSFORM_DATA* pTr = transform._Data();
        IF_NULL_RETURN_MSG_BREAK(pTr, E_FAIL, "Transform data is nullptr.");

        /* Rigidbody는 반드시 Collider가 있어야 한다. */
        CCollider collider = pObj->Get_Component<CCollider>();
        if (!collider.Is_Valid())
        {
            collider = pObj->Add_Component<CCollider>();
            /* NOTE : SHAPE는 즉시 설정해줘야 하는데, 설정하지 않았다면 기본값 BOX 로 들어가게 된다. 변경 불가 */
            collider.Set_Shape(SHAPE::BOX);
            IF_TRUE_RETURN_MSG_BREAK(!collider.Is_Valid(), E_FAIL, "Failed to add collider for rigidbody.");
        }

        COLLIDER_DATA* pColData = collider._Data();
        IF_NULL_RETURN_MSG_BREAK(pColData, E_FAIL, "Collider data is nullptr.");

        /* Rigidbody <-> Collider 연결 */
        pData->hCollider = collider.Get_Handle();
        pData->hTransform = pColData->hTransform;
        pColData->hRigidbody = hComponent;

        /* Rigidbody는 Collider의 shape를 따른다. */
        pData->eShape = pColData->eShape;

        /* 현재 기준 COM은 Transform Position과 동일하게 둔다. (추후 변경 가능) */
        pData->vCOM = pTr->vPosition;
        pData->vWorldCOM = pTr->vPosition;

        /* 계산값/임시값 초기화 */
        pData->fInvMass = 0.f;

        pData->vDimension = _float3{ 0.f, 0.f, 0.f };
        pData->vDimensionCenter = _float3{ 0.f, 0.f, 0.f };

        pData->vForceAccum = _float3{ 0.f, 0.f, 0.f }; /* 매 프레임 초기화 */
        pData->vTorqueAccum = _float3{ 0.f, 0.f, 0.f };

        pData->matInertiaTensor = Math::Identity();
        pData->matInvInertiaTensor = Math::Identity();
        pData->matWorldInertiaTensor = Math::Identity();
        pData->matWorldInvInertiaTensor = Math::Identity();

        /* 최초 계산이 필요하도록 dirty 설정 */
        pData->bDirtyMass = true;
        pData->bDirtyInertia = true;
        pData->bDirtyWorldInertia = true;
        pData->bInitialized = true;

        /* Collider도 다시 계산되도록 표시 */
        pColData->bDirty = true;

        IF_FAIL_RETURN_MSG_BREAK(m_upRigidbody_Builder->Rebuild(pData), E_FAIL, "Failed to rebuild rigidbody.");

        return S_OK;
    }
    case COMPONENT_TYPE::SPRING_JOINT:
    {
        SPRING_JOINT_DATA* pData = m_SpringJointPool.Get_Data_By_Handle(hComponent);
        IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "Can't find spring joint data.");

        CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
        IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Can't find pObj.");

        CRigidbody rigidbody = pObj->Get_Component<CRigidbody>();
        if (!rigidbody.Is_Valid())
        {
            rigidbody = pObj->Add_Component<CRigidbody>();
            IF_TRUE_RETURN_MSG_BREAK(!rigidbody.Is_Valid(), E_FAIL, "Failed to add rigidbody for spring joint.");
        }

        RIGIDBODY_DATA* pRbData = rigidbody._Data();
        IF_NULL_RETURN_MSG_BREAK(pRbData, E_FAIL, "Rigidbody data is nullptr.");

        /* SpringJoint <-> Rigidbody 연결 */
        pData->hRigidbody = rigidbody.Get_Handle();

        /* 보정 */
        if (pData->fSpring < 0.f) pData->fSpring = 0.f;
        if (pData->fDamper < 0.f) pData->fDamper = 0.f;
        if (pData->fRestLength < 0.f) pData->fRestLength = 0.f;
        if (pData->fMinLength < 0.f) pData->fMinLength = 0.f;
        if (pData->fMaxLength < 0.f) pData->fMaxLength = 0.f;

        if (pData->bUseMinLength && pData->fMinLength > pData->fRestLength && pData->fRestLength > 0.f)
            pData->fMinLength = pData->fRestLength;

        if (pData->bUseMaxLength && pData->fMaxLength < pData->fRestLength)
            pData->fMaxLength = pData->fRestLength;

        return S_OK;
    }
    default: return E_FAIL;
    }
}

std::unique_ptr<CPhysics_Processor> CPhysics_Processor::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    auto pInstance = std::make_unique<CPhysics_Processor>();

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(pDevice, pContext), nullptr, "Create instance failed");
    return pInstance;
}
