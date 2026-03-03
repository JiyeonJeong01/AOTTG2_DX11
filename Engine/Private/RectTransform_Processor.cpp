#include "RectTransform_Processor.h"
#include "Component_System.h"
#include "Graphic_Device.h"
#include "CRender_System.h"
#include "Component_Spec.h"

NS_BEGIN(Engine)

std::unique_ptr<CRectTransform_Processor> CRectTransform_Processor::Create()
{
    auto pInstance = std::make_unique<CRectTransform_Processor>();
    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "Create instance failed");
    return pInstance;
}

HRESULT CRectTransform_Processor::Initialize()
{
    SYS_COMPONENT.Register_InitialSpecFactory<CRectTransform, RECTTRANSFORM_SPEC>(COMPONENT_TYPE::RECT_TRANSFORM);
    SYS_COMPONENT.Register_BuildSpecFacotry<CRectTransform>(COMPONENT_TYPE::RECT_TRANSFORM);
    return S_OK;
}

void CRectTransform_Processor::Update(_float fDT)
{
    [[maybe_unused]] _float dt = fDT;

    const auto& ui = SYS_RENDER.Get_UI_Global();

    m_fWidth = ui.vViewport.x;
    m_fHeight= ui.vViewport.y;

    const auto& Pages = m_Pool.GetPages();
    for (const auto& upPage : Pages)
    {
        auto* pPage = upPage.get();
        if (!pPage) continue;

        for (uint32_t i = 0; i < PAGE_SIZE; ++i)
        {
            if (!pPage->Is_Allocated(i))
                continue;
            auto* pData = pPage->Get_Ptr(i);
            if (!pData->bDirty || !pData->bEnable)
                continue;
            Bake_World(pData);

            pData->bDirty = false;
        }
    }
}


void CRectTransform_Processor::LateUpdate(_float)
{
}

HRESULT CRectTransform_Processor::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pSpec)
{
    if (m_fHeight == 0 || m_fWidth == 0)
    {
        const auto& ui = SYS_RENDER.Get_UI_Global();
        m_fWidth = ui.vViewport.x;
        m_fHeight = ui.vViewport.y;

        IF_TRUE_RETURN_MSG_BREAK((m_fHeight == 0 || m_fWidth == 0), E_FAIL, "Initialize_From_Spec failed : Invalid viewport size");
    }

    RECTTRANSFORM_DATA* pData = m_Pool.Get_Data_By_Handle(handle);
    IF_NULL_RETURN_MSG_BREAK(pData, E_FAIL, "Invalid handle in Initialize_From_Spec");

    pData->vPosPx = SCAST(const RECTTRANSFORM_SPEC*, pSpec)->vPosPx;
    pData->vSizePx = SCAST(const RECTTRANSFORM_SPEC*, pSpec)->vSizePx;
    Bake_World(pData);

    return S_OK;
}

std::unique_ptr<COMPONENT_SPEC_BASE>
CRectTransform_Processor::Build_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    IF_TRUE_RETURN_MSG_BREAK(eComType != COMPONENT_TYPE::RECT_TRANSFORM, nullptr, "Wrong component type.");

    RECTTRANSFORM_DATA* pData = m_Pool.Get_Data_By_Handle(hComponent);
    IF_NULL_RETURN_MSG_BREAK(pData, nullptr, "Invalid RectTransform handle in Build_Spec.");

    auto spec = std::make_unique<RECTTRANSFORM_SPEC>();
    spec->vPosPx = pData->vPosPx;
    spec->vSizePx = pData->vSizePx;

    return spec;
}

inline void CRectTransform_Processor::Bake_World(RECTTRANSFORM_DATA* pData)
{
    const _matrix S = XMMatrixScaling(pData->vSizePx.x, -pData->vSizePx.y, 1.f);
    const _matrix T = XMMatrixTranslation(pData->vPosPx.x, pData->vPosPx.y, 0.f);

    XMStoreFloat4x4(&pData->matWorld, S * T);

    pData->bDirty = false;
}

NS_END
