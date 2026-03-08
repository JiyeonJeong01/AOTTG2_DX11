#include "Physics_Processor.h"
#include "Transform_Processor.h"
#include "Component_System.h"
#include "Collision_Detector.h"
#include "Collider_Proxy_Builder.h"
#include "Component_Spec.h"
#include "Rigidbody_Builder.h"
#include "GameObject.h"
#include "Solver.h"

HRESULT CPhysics_Processor::Initialize()
{
    /* 팩토리 등록 */
    {
        SYS_COMPONENT.Register_InitialSpecFactory<CCollider, COLLIDER_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CCollider>();

        SYS_COMPONENT.Register_InitialSpecFactory<CRigidbody, COLLIDER_SPEC>();
        SYS_COMPONENT.Register_BuildSpecFacotry<CRigidbody>();
    }

    m_pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransformProcessor, E_FAIL, "Transform Processor bind failed");

    m_upCollision_Detector = CCollision_Detector::Create(this);
    m_upCollider_Builder = CCollider_Proxy_Builder::Create();
    m_upRigidbody_Builder = CRigidbody_Builder::Create();
    m_upSolver = CSolver::Create();

    return S_OK;
}

void CPhysics_Processor::LateUpdate(_float fDT)
{

}

void CPhysics_Processor::Fixed_Update(_float fDT)
{
    /* 외력 적용하기 */
    Accumulate_Forces();
    Integrate_Forces( fDT);

    /* 적분 적용하기 */
    Integrate_Velocities(fDT);

    /* 충돌 감지하기 */
    vector<CONTACT_DESC> outContacts;
    Process_Collision(outContacts);

    /* 충돌 해결하기 */
    for (_uint i = 0; i < 1; ++i)
        for (auto& contact : outContacts)
            m_upSolver->Solve_Contacts(&contact);
}

void CPhysics_Processor::Render()
{
}

COMPONENT_HANDLE CPhysics_Processor::Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
        return Create_Component_Data_Inner<CCollider>(m_ColliderPool, hObject);
    case COMPONENT_TYPE::RIGIDBODY:
        return Create_Component_Data_Inner<CRigidbody>(m_RigidbodyPool, hObject);
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
    default:
        break;
    }

    _DEBUG_WARN("CUI_Processor::Remove_Component - unsupported component type");
}

void* CPhysics_Processor::Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:  return m_ColliderPool.Get_Data_By_Handle(hComponent);
    case COMPONENT_TYPE::RIGIDBODY:  return m_RigidbodyPool.Get_Data_By_Handle(hComponent);
    default: return nullptr;
    }
}

HRESULT CPhysics_Processor::Initialize_From_Spec_Collider(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{
    COLLIDER_DATA* pData = m_ColliderPool.Get_Data_By_Handle(h);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid Collider handle in Initialize_From_Spec_Collider");
    _DEBUG_ENGINE_ASSERT_MSG(spec != nullptr, "spec is nullptr in Initialize_From_Spec_Collider");

    const COLLIDER_SPEC* pSpec = SCAST(const COLLIDER_SPEC*, spec);

    pData->bEnable = pSpec->bEnable;
    pData->eColType = pSpec->eColType;
    pData->bOnCol = pSpec->bOnCol;
    pData->eShape = pSpec->eShape;
    pData->vOffset = pSpec->vOffset;

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
        pData->plane.fDistance = pSpec->fDistance;
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

std::unique_ptr<COMPONENT_SPEC_BASE> CPhysics_Processor::Build_Spec_Collider(COMPONENT_HANDLE hComponent)
{
    COLLIDER_DATA* pData = m_ColliderPool.Get_Data_By_Handle(hComponent);
    _DEBUG_ENGINE_ASSERT_MSG(pData != nullptr, "Invalid Collider handle in Build_Spec_Collider");

    auto pSpec = std::make_unique<COLLIDER_SPEC>();

    pSpec->bEnable = pData->bEnable;
    pSpec->eColType = pData->eColType;
    pSpec->bOnCol = pData->bOnCol;
    pSpec->eShape = pData->eShape;
    pSpec->vOffset = pData->vOffset;

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
        pSpec->fDistance = pData->plane.fDistance;
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
        pData->vPoint = pTr->vPosition;
        pData->vScale = pTr->vScale;

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
    default: return E_FAIL;
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

            /* TODO : Lock 적용 */
            /* TODO : ApplyPositionLock(*pData); */
            /* TODO : ApplyRotationLock(*pData); */

            /* accum 값은 해당 프레임에만 적용된다. */
            pData->vForceAccum = _float3{ 0.f, 0.f, 0.f };
            pData->vTorqueAccum = _float3{ 0.f, 0.f, 0.f };
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
            if (!pData || !pData->bEnable)
                continue;

            if (pData->eBodyType != BODY_TYPE::DYNAMIC)
                continue;

            CGameObject* pObj = SYS_GAMEOBJECT.Get_Wrapper(pData->hObject);
            if (!pObj)
                continue;

            CTransform transform = pObj->Get_Component<CTransform>();
            if (!transform.Is_Valid())
                continue;

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
    m_AllColliders.clear();

    /* TODO : 가능하면 active count로 reserve 해두기 */

    const auto& ColliderPage = m_ColliderPool.GetPages();
    for (const auto& upPage : ColliderPage)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;

            auto* pData = pPage->Get_Ptr(i);
            if (!pData || !pData->bEnable) continue;

            COLLIDER_PROXY_DATA outData;
            m_upCollider_Builder->Build_Collider_Proxy(pData, outData);
            pData->bDirty = false;

            m_AllColliders.push_back(std::move(outData));
        }
    }

    vector<COLLIDER_PAIR> outPair;
    m_upCollision_Detector->Generate_BroadPhase_Pairs(m_AllColliders, outPair);

    m_upCollision_Detector->Process_NarrowPhase(outPair, outContacts);
}

std::unique_ptr<CPhysics_Processor> CPhysics_Processor::Create()
{
    auto pInstance = std::make_unique<CPhysics_Processor>();

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
