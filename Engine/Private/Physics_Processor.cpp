#include "Physics_Processor.h"
#include "Transform_Processor.h"
#include "Component_System.h"
#include "Engine_Math.h"
#include "Collision_Detector.h"
#include "Collider_Proxy_Builder.h"

HRESULT CPhysics_Processor::Initialize()
{
    //SYS_COMPONENT.Register_InitialSpecFactory<CTransform, colli>(COMPONENT_TYPE::TRANSFORM);
    //SYS_COMPONENT.Register_BuildSpecFacotry<CTransform>(COMPONENT_TYPE::TRANSFORM);

    m_pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pTransformProcessor, E_FAIL, "Transform Processor bind failed");

    m_upCollision_Detector = CCollision_Detector::Create(this);
    m_upCollider_Builder = CCollider_Proxy_Builder::Create();

    return S_OK;
}

void CPhysics_Processor::LateUpdate(_float fDT)
{

}

void CPhysics_Processor::Fixed_Update(_float fDT)
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

    m_upCollision_Detector->Process_NarrowPhase(outPair);

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
    default:
        return ;
    }
}

HRESULT CPhysics_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pSpec)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
        return Initialize_From_Spec_Collider(hComponent, pSpec);
    default:
        return E_FAIL;
    }
    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CPhysics_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    switch (eComType)
    {
    case COMPONENT_TYPE::COLLIDER:
        {
            /* TODO 콜라이더 스펙 구현 이후 */
        }
    default:
        return nullptr;
    }
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
    default: return nullptr;
    }
}

HRESULT CPhysics_Processor::Initialize_From_Spec_Collider(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{

    return S_OK;
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
    default: return E_FAIL;
    }
}

std::unique_ptr<CPhysics_Processor> CPhysics_Processor::Create()
{
    auto pInstance = std::make_unique<CPhysics_Processor>();

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}
