#include "Transform_Processor.h"
#include "Component_System.h"

#include "Component_Spec.h"
#include "Engine_MathDX.h"

HRESULT CTransform_Processor::Initialize()
{
    SYS_COMPONENT->Register_Factory<CTransform, TRANSFORM_SPEC>(COMPONENT_TYPE::TRANSFORM);

    return S_OK;
}

void CTransform_Processor::LateUpdate(_float fDT)
{
}

HRESULT CTransform_Processor::Initialize_From_Spec(COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    TRANSFORM_DATA* pData = m_Pool.Get_Data_By_Handle(handle);

    pData->vPosition = SCAST(const TRANSFORM_SPEC*, pSpec)->vPosition;
    pData->vRotationQuat = SCAST(const TRANSFORM_SPEC*, pSpec)->vRotationQuat;
    pData->vScale = SCAST(const TRANSFORM_SPEC*, pSpec)->vScale;

    Bake_World(pData);

    pData->bDirty = false;

    return S_OK;
}

void CTransform_Processor::Update(_float fDT)
{
    auto Pages = m_Pool.GetPages();
    for (auto* pPage : Pages)
    {
        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Active(i))
                continue;
            auto* pData = pPage->Get_Ptr(i);
            if (!pData->bDirty)
                continue;
            Bake_World(pData);
        }
    }

}

void CTransform_Processor::Bake_World(TRANSFORM_DATA* pData)
{
    const _matrix S = XMMatrixScaling(pData->vScale.x, pData->vScale.y, pData->vScale.z);
    const _matrix R = XMMatrixRotationQuaternion(MathDX::Load(pData->vRotationQuat));
    const _matrix T = XMMatrixTranslationFromVector(MathDX::Load(pData->vPosition));

    const _matrix W = S * R * T; /* row-vector convention */
    MathDX::Store(pData->matWorld, W);

    pData->bDirty = false;
}

CTransform_Processor* CTransform_Processor::Create()
{
    CTransform_Processor* pInstance = new CTransform_Processor;
    if (FAILED(pInstance->Initialize()))
    {
        _DEBUG_ERROR_BREAK("Create instance failed");
        Safe_Release(pInstance);
    }
    return pInstance;
}

void CTransform_Processor::Free()
{
    /* TODO : implement free logic */
}
