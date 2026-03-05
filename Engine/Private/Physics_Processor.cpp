#include "Physics_Processor.h"
#include "Transform_Processor.h"
#include "Component_System.h"

HRESULT CPhysics_Processor::Initialize()
{
    m_pTransformProcessor = SYS_COMPONENT.Bind_Processor<CTransform_Processor>();

    return S_OK;
}

void CPhysics_Processor::LateUpdate(_float fDT)
{
    CComponent_Processor::LateUpdate(fDT);
}

void CPhysics_Processor::Fixed_Update(_float fDT)
{
}

void CPhysics_Processor::Render()
{
}

COMPONENT_HANDLE CPhysics_Processor::Create_Component_Data(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject)
{
    return {};
}

void CPhysics_Processor::Remove_Component(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
}

HRESULT CPhysics_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent,
    const COMPONENT_SPEC_BASE* pSpec)
{
    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE> CPhysics_Processor::Build_Spec(COMPONENT_TYPE eComType,
    COMPONENT_HANDLE hComponent)
{
    return nullptr;
}

void CPhysics_Processor::Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, _bool bEnable)
{
}

void* CPhysics_Processor::Get_DataPtr(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent) noexcept
{
    return nullptr;
}

HRESULT CPhysics_Processor::Initialize_From_Spec_Collider(COMPONENT_HANDLE h, const COMPONENT_SPEC_BASE* spec)
{
    return S_OK;
}
