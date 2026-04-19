#include "Component_System.h"
#include "Component_Processor.h"
#include "ComponentGroup_Manager.h"

#include "Transform_Processor.h"
#include "Physics_Processor.h"
#include "Environment_Processor.h"
#include "MeshRenderer_Processor.h"
#include "Animator_Processor.h"
#include "RectTransform_Processor.h"
#include "CanvasRenderer_Processor.h"
#include "Script_Processor.h"
#include "UI_Processor.h"
#include "SpriteEffect_Processor.h"

#include "Render_Struct.h"

IMPLEMENT_SINGLETON(CComponent_System)



CComponent_System::CComponent_System() = default;
CComponent_System::~CComponent_System() = default;


HRESULT CComponent_System::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    static_assert((uint32_t)COMPONENT_TYPE::END <= 32);

    m_pDevice = pDevice;
    m_pContext = pContext;

    m_pComGroupMgr = CComponentGroup_Manager::Create();

    m_pComProcessors.resize(COMPONENT_PROCESSOR_MAX);

    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::TRANSFORM)]
        = CTransform_Processor::Create();
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::PHYSICS)]
        = CPhysics_Processor::Create(m_pDevice, m_pContext);
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::MESH_RENDERER)]
        = CMeshRenderer_Processor::Create(m_pDevice, m_pContext);
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::ANIMATION)]
        = CAnimator_Processor::Create();
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::ENVIRONMENT)]
        = CEnvironment_Processor::Create();
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::RECT_TRANSFORM)]
        = CRectTransform_Processor::Create();
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::CANVAS_RENDERER)]
        = CCanvasRenderer_Processor::Create(m_pDevice, m_pContext, SCAST(CRectTransform_Processor*, m_pComProcessors[COM_TO_PID(COMPONENT_TYPE::RECT_TRANSFORM)].get()));
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::SCRIPT)]
        = CScript_Processor::Create();
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::UI)]
        = CUI_Processor::Create(m_pDevice, m_pContext);
    m_pComProcessors[PID_TO_INT(PROCESSOR_ID::SPRITE_EFFECT)]
        = CSpriteEffect_Processor::Create(m_pDevice, m_pContext);

    for (auto& proc : m_pComProcessors)
    {
        if (proc)
            proc->Late_Initialize();
    }

	return S_OK;
}

void CComponent_System::Update(_float fDT)
{
    static_cast<CScript_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::SCRIPT)].get())->Update(fDT);
    static_cast<CTransform_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::TRANSFORM)].get())->Update(fDT);
    static_cast<CEnvironment_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::ENVIRONMENT)].get())->Update(fDT);
    static_cast<CRectTransform_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::RECT_TRANSFORM)].get())->Update(fDT);
    static_cast<CUI_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::UI)].get())->Update(fDT);
    static_cast<CPhysics_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::PHYSICS)].get())->Update(fDT);
    static_cast<CAnimator_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::ANIMATION)].get())->Update(fDT);
    static_cast<CMeshRenderer_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::MESH_RENDERER)].get())->Update(fDT);
    static_cast<CCanvasRenderer_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::CANVAS_RENDERER)].get())->Update(fDT);
    static_cast<CSpriteEffect_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::SPRITE_EFFECT)].get())->Update(fDT);
}

void CComponent_System::LateUpdate(_float fDT)
{
    static_cast<CScript_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::SCRIPT)].get())->LateUpdate(fDT);
    static_cast<CTransform_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::TRANSFORM)].get())->LateUpdate(fDT);
    static_cast<CEnvironment_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::ENVIRONMENT)].get())->LateUpdate(fDT);
    static_cast<CRectTransform_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::RECT_TRANSFORM)].get())->LateUpdate(fDT);
    static_cast<CUI_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::UI)].get())->LateUpdate(fDT);
    static_cast<CPhysics_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::PHYSICS)].get())->LateUpdate(fDT);
    static_cast<CAnimator_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::ANIMATION)].get())->LateUpdate(fDT);
    static_cast<CMeshRenderer_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::MESH_RENDERER)].get())->LateUpdate(fDT);
    static_cast<CCanvasRenderer_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::CANVAS_RENDERER)].get())->LateUpdate(fDT);
    static_cast<CSpriteEffect_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::SPRITE_EFFECT)].get())->Update(fDT);
}

void CComponent_System::FixedUpdate(_float fDT)
{
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::PHYSICS)], , "m_pComProcessor is nullptr");

    To<CPhysics_Processor*>(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::PHYSICS)].get())->Fixed_Update(0.02f);
}

void CComponent_System::Build_RenderQueue(vector<DRAW_CMD>& cmds)
{
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::MESH_RENDERER)], , "m_pComProcessor is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::CANVAS_RENDERER)], , "m_pComProcessor is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::UI)], , "m_pComProcessor is nullptr");

    To<CMeshRenderer_Processor*>(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::MESH_RENDERER)].get())->Build_RenderQueue(cmds);
    To<CCanvasRenderer_Processor*>(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::CANVAS_RENDERER)].get())->Build_RenderQueue(cmds);
    To<CUI_Processor*>(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::UI)].get())->Build_RenderQueue(cmds);
    To<CSpriteEffect_Processor*>(m_pComProcessors[To<_int>(PROCESSOR_ID::SPRITE_EFFECT)].get())->Build_RenderQueue(cmds);

}

void CComponent_System::Render()
{
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::PHYSICS)], , "m_pComProcessor is nullptr");

    To< CPhysics_Processor*>(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::PHYSICS)].get())->Render();
}

void CComponent_System::Update_Debug()
{
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::TRANSFORM)], , "m_pComProcessor is nullptr");
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[PID_TO_INT(PROCESSOR_ID::RECT_TRANSFORM)], , "m_pComProcessor is nullptr");

    SCAST(CTransform_Processor*, m_pComProcessors[PID_TO_INT(PROCESSOR_ID::TRANSFORM)].get())->Update(0.f);
    SCAST(CRectTransform_Processor*, m_pComProcessors[PID_TO_INT(PROCESSOR_ID::RECT_TRANSFORM)].get())->Update(0.f);
}

COMPONENT_HANDLE CComponent_System::Create_Component_By_Type(COMPONENT_TYPE eComType, OBJECT_HANDLE hObject)
{
    const uint32_t iComIdx = COM_TO_INT(eComType);
    const uint32_t iProcIdx = COM_TO_PID(eComType);
    IF_TRUE_RETURN_MSG_BREAK((iComIdx >= COMPONENT_MAX || iProcIdx >= COMPONENT_PROCESSOR_MAX || !m_pComProcessors[iProcIdx]), COMPONENT_HANDLE{},
        "Processor not registered for this component type.");

    return m_pComProcessors[iProcIdx]->Create_Component_Data(eComType, hObject);
}

/* SceneObject나 Prototype에서 역직렬화한 Spec으로 컴포넌트를 생성하기 위해 호출된다. */
HRESULT CComponent_System::Create_Component_From_Spec(CGameObject* pObj, const COMPONENT_SPEC_BASE* pSpec)
{
    IF_NULL_RETURN_MSG_BREAK(pObj, E_FAIL, "Create from spec failed: pObj is nullptr.");
    IF_NULL_RETURN_MSG_BREAK(pSpec, E_FAIL, "Create from spec failed: pSpec is nullptr.");

    const COMPONENT_TYPE eType = pSpec->Get_Type();
    auto fn = m_InitialSpecFactory[COM_TO_INT(eType)];
    IF_NULL_RETURN_MSG_BREAK(fn, E_FAIL, "Factory not registered for this component type.");

    return fn(this, eType, pObj, pSpec);
}

void CComponent_System::Remove_Component_By_Type(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle)
{
    const uint32_t iComIdx = COM_TO_INT(eComType);
    const uint32_t iProcIdx = COM_TO_PID(eComType);
    IF_TRUE_RETURN_MSG_BREAK((iComIdx >= COMPONENT_MAX || iProcIdx >= COMPONENT_PROCESSOR_MAX || !m_pComProcessors[iProcIdx]), ,
        "Processor not registered for this component type.");

    m_pComProcessors[iProcIdx]->Remove_Component(eComType, handle);
}

void CComponent_System::Set_Enable(COMPONENT_TYPE eComType, COMPONENT_HANDLE handle, _bool bEnable)
{
    const uint32_t iComIdx = COM_TO_INT(eComType);
    const uint32_t iProcIdx = COM_TO_PID(eComType);

    IF_TRUE_RETURN_MSG_BREAK((iComIdx >= COMPONENT_MAX || iProcIdx >= COMPONENT_PROCESSOR_MAX || !m_pComProcessors[iProcIdx]), ,
        "Processor not registered for this component type.");

    m_pComProcessors[iProcIdx]->Set_Enable(eComType, handle, bEnable);
}

void CComponent_System::Get_Component_Handle_By_Type(COMPONENT_TYPE eComType, OBJECT_HANDLE hObj, vector<COMPONENT_HANDLE>& outHandles)
{
    outHandles.clear();

    const GAMEOBJECT_DATA& tData = SYS_GAMEOBJECT.Access_Data_Raw(hObj);

    const uint32_t iSlotData = tData.iComponentSlots[COM_TO_INT(eComType)];
    if (iSlotData == Component::INVALID_COMPONENT_SLOT)
    {
        return;
    }
    if ((iSlotData & Component::GROUP_FLAG) == 0)
    {
        COMPONENT_HANDLE hComponent;
        hComponent.iHandle = iSlotData;
        outHandles.push_back(hComponent);
        return ;
    }
    const uint32_t iGroupID = iSlotData & Component::DATA_MASK;
    const auto& tGroup = Get_Group(iGroupID);

    outHandles.reserve(tGroup.tExtras.size() + 1);
    outHandles.push_back(tGroup.tPrimary);

    for (auto& hCom : tGroup.tExtras)
        outHandles.push_back(hCom);
}

HRESULT CComponent_System::Initialize_From_Spec(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent, const COMPONENT_SPEC_BASE* pBase)
{
    const uint32_t iComIdx = COM_TO_INT(eComType);
    const uint32_t iProcIdx = COM_TO_PID(eComType);

    IF_TRUE_RETURN_MSG_BREAK((iComIdx >= COMPONENT_MAX), E_FAIL, "Invalid component type");
    IF_NULL_RETURN_MSG_BREAK(m_pComProcessors[iProcIdx], E_FAIL, "m_pComProcessor is nullptr");
    IF_FAIL_RETURN_MSG_BREAK(m_pComProcessors[iProcIdx]->Initialize_From_Spec(eComType, hComponent, pBase), E_FAIL,
        "Failed initialize with spec");

    return S_OK;
}

/* 1. 게임 오브젝트 클론을 위한 스펙 생성 및 적용
 * 2. Prototype, SceneObject 직렬화를 위한 스펙 빌드
 *
 */
std::unique_ptr<COMPONENT_SPEC_BASE> CComponent_System::Build_Spec_By_Type(COMPONENT_TYPE eComType, COMPONENT_HANDLE hComponent)
{
    /* --------------------------------------------------------------------------------------------------------------
     * WARNING: Do NOT register this function in Register_BuildSpecFactory.  Doing so will cause infinite recursion.
     * --------------------------------------------------------------------------------------------------------------*/
    const uint32_t iComIdx = COM_TO_INT(eComType);

    IF_TRUE_RETURN_MSG_BREAK((iComIdx >= COMPONENT_MAX), nullptr, "Invalid component type");

    auto fn = m_BuildSpecFactory[iComIdx];
    IF_NULL_RETURN_MSG_BREAK(fn, nullptr, "Factory not registered for this component type.");

    auto upSpec = fn(this, eComType, hComponent);
    IF_NULL_RETURN_MSG_BREAK(upSpec, nullptr, "Factory not registered for this component type.");
    return upSpec;
}

uint32_t CComponent_System::Promote(COMPONENT_HANDLE hOld, COMPONENT_HANDLE hNew)
{
	return m_pComGroupMgr->Promote(hOld, hNew);
}

void CComponent_System::Add_To_Group(uint32_t iGroupID, COMPONENT_HANDLE hNew) 
{
	m_pComGroupMgr->Add_To_Group(iGroupID, hNew);
}

COMPONENT_GROUP& CComponent_System::Get_Group(uint32_t iGroupID)
{
	return m_pComGroupMgr->Get_Group(iGroupID);
}

void CComponent_System::Free_Group(uint32_t iGroupID)
{
	m_pComGroupMgr->Free_Group(iGroupID);
}
