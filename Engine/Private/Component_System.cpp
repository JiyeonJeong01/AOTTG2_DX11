#include "Component_System.h"
#include "Component_Processor.h"
#include "ComponentGroup_Manager.h"

#include "Transform_Processor.h"
#include "MeshRenderer_Processor.h"
#include "RectTransform_Processor.h"
#include "CanvasRenderer_Processor.h"

IMPLEMENT_SINGLETON(CComponent_System)

std::array<PROCESSOR_ID, COMPONENT_MAX>
CComponent_System::m_TypeToProcessorIndex =
{
    PROCESSOR_ID::TRANSFORM,       // TRANSFORM
    PROCESSOR_ID::PHYSICS,         // COLLIDER
    PROCESSOR_ID::PHYSICS,         // RIGIDBODY
    PROCESSOR_ID::SCRIPT,          // SCRIPT
    PROCESSOR_ID::RENDER,          // MESH_RENDERER
    PROCESSOR_ID::ANIMATION,       // ANIMATOR
    PROCESSOR_ID::CAMERA,          // CAMERA
    PROCESSOR_ID::AUDIO,           // AUDIO_LISTENER
    PROCESSOR_ID::AUDIO,           // AUDIO_SOURCE

    PROCESSOR_ID::RECT_TRANSFORM,  // RECT_TRANSFORM
    PROCESSOR_ID::CANVAS,          // CANVAS_RENDERER

    PROCESSOR_ID::UI,              // UI_IMAGE
    PROCESSOR_ID::UI,              // UI_BUTTON
    PROCESSOR_ID::UI               // UI_TEXT
};

CComponent_System::CComponent_System() = default;
CComponent_System::~CComponent_System() = default;


HRESULT CComponent_System::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iWidth, _uint iHeight)
{
    static_assert((uint32_t)COMPONENT_TYPE::END <= 32);

    m_pDevice = pDevice;
    m_pContext = pContext;

    m_pComGroupMgr = CComponentGroup_Manager::Create();

    m_pComProcessors.resize(SCAST(_uint, COMPONENT_TYPE::END));

    m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::TRANSFORM)]
        = CTransform_Processor::Create();
    m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::MESH_RENDERER)]
        = CMeshRenderer_Processor::Create(m_pDevice, m_pContext, SCAST(CTransform_Processor*, m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::TRANSFORM)].get()));
    m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::RECT_TRANSFORM)]
        = CRectTransform_Processor::Create();
    m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::CANVAS_RENDERER)]
        = CCanvasRenderer_Processor::Create(m_pDevice, m_pContext, SCAST(CRectTransform_Processor*, m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::RECT_TRANSFORM)].get()), iWidth, iHeight);


	return S_OK;
}

void CComponent_System::Update(_float fDT)
{
	for (auto& pProcessor : m_pComProcessors)
        if (pProcessor) 
    		pProcessor->Update(fDT);
}

void CComponent_System::LateUpdate(_float fDT)
{
    for (auto& pProcessor : m_pComProcessors)
    {
        if (pProcessor)
            pProcessor->LateUpdate(fDT);
    }
}

void CComponent_System::FixedUpdate(_float fDT)
{
}

void CComponent_System::Render()
{
    SCAST(CMeshRenderer_Processor*, m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::MESH_RENDERER)].get())->Render();
    SCAST(CCanvasRenderer_Processor*, m_pComProcessors[SCAST(_uint, COMPONENT_TYPE::CANVAS_RENDERER)].get())->Render();
}

COMPONENT_HANDLE CComponent_System::Create_Component_By_Type(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject)
{
    const uint32_t iIndex = SCAST(_uint, eComType);
    IF_TRUE_RETURN_MSG_BREAK((iIndex >= SCAST(_uint, COMPONENT_TYPE::END) || !m_pComProcessors[iIndex]), COMPONENT_HANDLE{}, "Processor not registered for this component type.");

    return m_pComProcessors[iIndex]->Create_Component_Data(hObject);
}

void CComponent_System::Create_From_Spec(CGameObject* pObj, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_NULL_RETURN_MSG_BREAK(pObj, , "Create from spec failed: pObj is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pSpec, , "Create from spec failed: pSpec is nullptr.");

    const COMPONENT_TYPE eType = pSpec->Get_Type();
    auto fn = m_factory[SCAST(_uint, eType)];
    IF_NULL_RETURN_MSG_BREAK(fn, , "Factory not registered for this component type.");

    fn(this, eType, pObj, pSpec);
}

void CComponent_System::Remove_Component_By_Type(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle)
{
    const uint32_t iIndex = SCAST(_uint, eComType);
    IF_TRUE_RETURN_MSG_BREAK((iIndex >= SCAST(_uint, COMPONENT_TYPE::END) || !m_pComProcessors[iIndex]), , "Processor not registered for this component type.");

    m_pComProcessors[iIndex]->Remove_Component(handle);
}

void CComponent_System::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, const COMPONENT_SPEC_BASE* pBase)
{
    IF_TRUE_RETURN_MSG_BREAK((SCAST(_uint, eComType) >= SCAST(_uint, COMPONENT_TYPE::END)), , "Invalid component type");
    IF_FAIL_RETURN_MSG_BREAK(m_pComProcessors[SCAST(_uint, eComType)]->Initialize_From_Spec(handle, pBase), , "Failed initialize with spec");
}

uint32_t CComponent_System::Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew)
{
	return m_pComGroupMgr->Promote(hOld, hNew);
}

void CComponent_System::Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew) 
{
	m_pComGroupMgr->Add_To_Group(iGroupID, hNew);
}

const COMPONENT_GROUP& CComponent_System::Get_Group(uint32_t iGroupID)
{
	return m_pComGroupMgr->Get_Group(iGroupID);
}

void CComponent_System::Free_Group(uint32_t iGroupID)
{
	m_pComGroupMgr->Free_Group(iGroupID);
}
