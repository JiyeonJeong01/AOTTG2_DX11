#pragma region HEADER
#include "InspectorPanel.h"
#include "HierarchyPanel.h"
#include "GameObject.h"
#include "Engine_Log.h"
#include "Asset_Registry.h"
#include "LayerHelper.h"

#include "Component_System.h"
#include "Resource_System.h"

#include "Transform.h"
#include "Rigidbody.h"
#include "Collider.h"
#include "SpringJoint.h"
#include "MeshRenderer.h"
#include "Script.h"
#include "RectTransform.h"
#include "Camera.h"
#include "Animator.h"

#include "UIImage.h"
#include "UIButton.h"
#include "CanvasRenderer.h"

#include "Editor_Util.h"
#include "Script_Processor.h"
#include "Animator_Processor.h"

#include "Material.h"
#include "UI_Processor.h"
#include "MeshRenderer_Processor.h"

#include "Client_Define.h"
#include "magic_enum.hpp"

#pragma endregion

NS_BEGIN(Editor)
    CInspectorPanel::CInspectorPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CInspectorPanel::~CInspectorPanel()
{
}

HRESULT CInspectorPanel::Initialize(CHierarchyPanel* pPanel, CProjectPanel* pProject)
{
    pPanel->m_OnPrimarySelectionChanged.Add_Listener(&CInspectorPanel::Set_Target, this);
    pProject->m_OnSelectionChanged.Add_Listener(&CInspectorPanel::Set_Selected_Asset, this);

    m_pScript_Processor = SYS_COMPONENT.Bind_Processor<CScript_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pScript_Processor, E_FAIL, "Transform processor bind failed");

    m_pMeshRenderer_Processor = SYS_COMPONENT.Bind_Processor<CMeshRenderer_Processor>();
    IF_NULL_RETURN_MSG_BREAK(m_pMeshRenderer_Processor, E_FAIL, "MeshRenderer_Processor processor bind failed");

    return S_OK;
}

void CInspectorPanel::Set_Target(Engine::CGameObject* pObj)
{
    if (m_pTarget == pObj)
        return;

    m_pTarget = pObj;

    /* Refresh name buffer */
    m_nameBuffer.clear();
    if (m_pTarget)
        m_nameBuffer.assign(m_pTarget->Get_Label());

    m_bJustStartedNameEdit = false;

    m_selectedAsset = ASSET_SELECTION{};
    m_eMode = (pObj ? InspectMode::GameObject : InspectMode::None);

    /* 편집 관련 초기화  */
    m_bAttachInputActive = false;
    m_bEditUseAttach = false;
    m_strEditAttachBoneName = "";
    m_bEditModeInitialized = false;
    m_eEditMode = MESH_MODE::NONE;
}

void CInspectorPanel::Set_Selected_Asset(const ASSET_SELECTION& sel)
{
    m_selectedAsset = sel;
    m_pTarget = nullptr;
    m_eMode = (sel.Is_Valid() ? InspectMode::Asset : InspectMode::None);
}

void CInspectorPanel::Clear_Target()
{
}

void CInspectorPanel::Validate_Target()
{
    if (m_pTarget == nullptr)
        return;

    /* TODO : Validiate  */
}

void CInspectorPanel::Render()
{
    if (!m_bOpen)
        return;

    Validate_Target();

    if (!ImGui::Begin(m_strPanelName.c_str(), (bool*)&m_bOpen))
    {
        ImGui::End();
        return;
    }

    if (m_eMode == InspectMode::GameObject)
    {
        if (m_pTarget == nullptr)
        {
            ImGui::TextUnformatted("No selection.");
            ImGui::End();
            return;
        }
        Draw_Header();
        ImGui::Separator();

        Draw_ObjectLayer();
        ImGui::Separator();

        Draw_Basic_Info();
        ImGui::Separator();

        Draw_RequiredComponent();
        ImGui::Separator();

        Draw_Components();
    }
    else if (m_eMode == InspectMode::Asset)
    {
        Draw_Asset();
    }
    else
    {
        Draw_None();
    }

    ImGui::End();
}

void CInspectorPanel::Draw_Header()
{
    /* Optional : icon or breadcrumbs */
    ImGui::TextUnformatted(m_pTarget->Get_Label().data());
    ImGui::SameLine();
    ImGui::TextDisabled("(%p)", (void*)m_pTarget);

    ImGui::SameLine();
    ImGui::Checkbox("Auto Focus", &m_bAutoFocusOnSelection);
}

void CInspectorPanel::Draw_Basic_Info()
{
    /* Active Toggle */
    _bool bActive = m_pTarget->Get_Enabled();
    if (ImGui::Checkbox("Active", &bActive))
        m_pTarget->Set_Enable(bActive);

    /* Name Edit */
    ImGui::TextUnformatted("Name");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);

    if (m_bAutoFocusOnSelection && m_bJustStartedNameEdit)
    {
        ImGui::SetKeyboardFocusHere();
    }

    ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_AutoSelectAll |
        ImGuiInputTextFlags_EnterReturnsTrue;

    const bool bEnter = ImGui::InputText("##InspectorName", &m_nameBuffer, flags);

    /* Confirm the name when enter is pressed */
    if (bEnter)
    {
        if (m_nameBuffer.empty())
            m_nameBuffer = "GameObject";

        m_pTarget->Set_Label(m_nameBuffer.c_str());
        m_bJustStartedNameEdit = false;
        return;
    }

    /* Skip the first frame to avoid race */
    if (m_bJustStartedNameEdit)
    {
        m_bJustStartedNameEdit = false;
        return;
    }

    /* Conform the name when focus is lost */
    if (ImGui::IsItemDeactivated())
    {
        if (m_nameBuffer.empty())
            m_nameBuffer = "GameObject";

        m_pTarget->Set_Label(m_nameBuffer.c_str());
        return;
    }

    ImGui::Spacing();

    if (ImGui::TreeNodeEx("Object Mask", ImGuiTreeNodeFlags_DefaultOpen))
    {
        struct MASK_ITEM
        {
            const char* szName;
            uint32_t    iMask;
        };

        std::string_view svSeverity = magic_enum::enum_name(Client::PLAYER);

        

        const MASK_ITEM arrMaskItems[] =
        {
            { "PLAYER",   Client::PLAYER },
            { "TITAN",    Client::TITAN },
            { "NPC",      Client::NPC },
            { "ALLY",     Client::ALLY },
            { "WALKABLE", Client::WALKABLE },
        };

        for (const auto& tItem : arrMaskItems)
        {
            bool bChecked = m_pTarget->Has_Mask(tItem.iMask);

            if (ImGui::Checkbox(tItem.szName, &bChecked))
            {
                if (bChecked)
                    m_pTarget->Add_Mask(tItem.iMask);
                else
                    m_pTarget->Remove_Mask(tItem.iMask);
            }
        }

        ImGui::TreePop();
    }
}

void CInspectorPanel::Draw_ObjectLayer()
{
    if (!m_pTarget) return;

    auto& sys = SYS_GAMEOBJECT;
    auto& layers = sys.Layers();

    const Layer::LAYER_ID curLayer = m_pTarget->Get_Layer();

    std::string curName = "None";
    layers.Find_Name(curLayer, curName);

    ImGui::TextUnformatted("Layer");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(180.f);

    bool openEditPopup = false;

    if (ImGui::BeginCombo("##LayerCombo", curName.c_str()))
    {
        for (Layer::LAYER_ID i = 0; i < Layer::MAX_LAYERS; ++i)
        {
            if (!layers.Is_Used(i)) continue;

            std::string name;
            if (!layers.Find_Name(i, name)) continue;

            const bool selected = (i == curLayer);
            if (ImGui::Selectable(name.c_str(), selected))
                sys.Set_Layer(m_pTarget, i);

            if (selected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::Separator();

        if (ImGui::Selectable("Edit Layers..."))
            openEditPopup = true;

        ImGui::EndCombo();
    }

    if (openEditPopup)
    {
        ImVec2 itemMin = ImGui::GetItemRectMin();
        ImVec2 itemSize = ImGui::GetItemRectSize();

        ImGui::SetNextWindowPos(
            ImVec2(itemMin.x, itemMin.y + itemSize.y),
            ImGuiCond_Appearing
        );

        ImGui::SetNextWindowSize(ImVec2(460.f, 420.f), ImGuiCond_Appearing);

        ImGui::OpenPopup("LayerEditorPopup");
    }

    Draw_LayerEditorPopup();
}

void CInspectorPanel::Draw_LayerEditorPopup()
{
    if (!ImGui::BeginPopup("LayerEditorPopup", ImGuiWindowFlags_AlwaysAutoResize))
        return;

    auto& layers = SYS_GAMEOBJECT.Layers();

    ImGui::TextUnformatted("Layers");
    ImGui::SameLine();

    float buttonWidth = 60.f;
    float padding = ImGui::GetStyle().ItemSpacing.x;
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - buttonWidth - padding - 10.f);

    if (ImGui::Button("X", ImVec2(buttonWidth, 0)))
        ImGui::CloseCurrentPopup();

    ImGui::Separator();

    const float listH = 260.f;

    if (ImGui::BeginChild("##LayerList", ImVec2(420.f, listH), true))
    {
        for (Layer::LAYER_ID i = 0; i < Layer::MAX_LAYERS; ++i)
        {
            if (!layers.Is_Used(i))
                continue;

            std::string name;
            layers.Find_Name(i, name);

            const bool reserved = (i == Layer::DEFAULT_LAYER || i == Layer::UI_LAYER);

            ImGui::PushID((int)i);

            ImGui::Text("[%02u]", (unsigned)i);
            ImGui::SameLine();

            if (reserved)
            {
                ImGui::BeginDisabled(true);
                char buf[64]{};
                strncpy_s(buf, name.c_str(), _TRUNCATE);
                ImGui::SetNextItemWidth(260.f);
                ImGui::InputText("##LayerName", buf, IM_ARRAYSIZE(buf));
                ImGui::EndDisabled();

                ImGui::SameLine();
                ImGui::TextDisabled("System");
            }
            else
            {
                static char editBuf[Layer::MAX_LAYERS][64] = {};
                if (editBuf[i][0] == '\0')
                    strncpy_s(editBuf[i], name.c_str(), _TRUNCATE);

                ImGui::SetNextItemWidth(260.f);
                if (ImGui::InputText("##LayerName", editBuf[i], IM_ARRAYSIZE(editBuf[i]),
                    ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    std::string newName = editBuf[i];
                    if (!newName.empty() && newName != name)
                        layers.Rename_Layer(i, newName);
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete"))
                {
                    layers.Unregister_Layer(i);
                    editBuf[i][0] = '\0';
                }
            }

            ImGui::PopID();
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    // ====== 하단: Add Layer (ID 콤보 + 힌트 텍스트 + +버튼) ======
    static Layer::LAYER_ID s_selectedID = 0;
    static char s_newName[64] = "";

    std::vector<Layer::LAYER_ID> unused;
    layers.Gather_UnusedLayers(unused);

    // 1. ID 선택 (폭 50px로 축소)
    ImGui::SetNextItemWidth(50.f);
    const char* preview = (unused.empty() ? "--" : std::to_string((unsigned)s_selectedID).c_str());
    if (ImGui::BeginCombo("##UnusedID", preview))
    {
        for (auto id : unused)
        {
            if (ImGui::Selectable(std::to_string((unsigned)id).c_str(), id == s_selectedID))
                s_selectedID = id;
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();

    // 2. 이름 입력
    ImGui::SetNextItemWidth(295.f);
    ImGui::InputTextWithHint("##NewLayerName", "New Layer Name...", s_newName, IM_ARRAYSIZE(s_newName));

    ImGui::SameLine();

    // 3. + 버튼
    const bool canAdd = (!unused.empty() && s_newName[0] != '\0');
    ImGui::BeginDisabled(!canAdd);
    if (ImGui::Button("Add", ImVec2(120.f, 0)))
    {
        layers.Register_Layer(s_selectedID, s_newName);
        s_newName[0] = '\0'; // 입력창 비우기

        std::vector<Layer::LAYER_ID> nextUnused;
        layers.Gather_UnusedLayers(nextUnused);
        if (!nextUnused.empty()) s_selectedID = nextUnused.front();
    }
    ImGui::EndDisabled();

    ImGui::EndPopup();
}

void CInspectorPanel::Draw_RequiredComponent()
{
    if (!m_pTarget)
        return;
    Component::COMPONENT_MASK mask = m_pTarget->Get_ComponentMask();

    if ((m_pTarget->Get_ComponentMask() & Component::To_Bit(COMPONENT_TYPE::TRANSFORM)) != 0)
        Draw_Transform();
    else if ((m_pTarget->Get_ComponentMask() & Component::To_Bit(COMPONENT_TYPE::RECT_TRANSFORM)) != 0)
        Draw_RectTransform();
}

void CInspectorPanel::Draw_Components()
{
    if (!ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    Draw_CurrentComponents();

    ImVec2 size(ImGui::GetContentRegionAvail().x, 50.f);
    if (ImGui::Button("Add Component", size))
    {
        ImGui::OpenPopup("##AddComponentPopup");
    }

    Draw_AddComponentPopup();
    Draw_CreateScriptPopup();
}

void CInspectorPanel::Draw_CurrentComponents()
{
    Component::COMPONENT_MASK mask = m_pTarget->Get_ComponentMask();

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        if ((mask & Component::To_Bit((COMPONENT_TYPE)i)) == 0)
            continue;

        COMPONENT_TYPE eType = (COMPONENT_TYPE)i;
        Draw_ComponentByType(eType);
    }
}

void CInspectorPanel::Draw_AddComponentPopup()
{
    _bool bIsUI = m_pTarget->Get_Handle().Is_UI();

    if (!ImGui::BeginPopup("##AddComponentPopup"))
        return;
    if (false == bIsUI) /* ----------------------- Not UI--------------------------*/
    {
        if (ImGui::BeginMenu("Collider"))
        {
            if (ImGui::MenuItem("Box"))
            {
                CCollider col = m_pTarget->Add_Component<CCollider>();
                if (col.Is_Valid())
                    col.Set_Shape(SHAPE::BOX);

                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Sphere"))
            {
                CCollider col = m_pTarget->Add_Component<CCollider>();
                if (col.Is_Valid())
                    col.Set_Shape(SHAPE::SPHERE);

                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Plane"))
            {
                CCollider col = m_pTarget->Add_Component<CCollider>();
                if (col.Is_Valid())
                    col.Set_Shape(SHAPE::PLANE);

                ImGui::CloseCurrentPopup();
            }

            ImGui::EndMenu();
        }

        if (ImGui::MenuItem("Rigidbody"))
        {
            CCollider col = m_pTarget->Get_Component<CCollider>();
            if (!col.Is_Valid())
            {
                ImGui::OpenPopup("RigidbodyNeedColliderPopup");
            }
            else
            {
                CRigidbody rb = m_pTarget->Add_Component<CRigidbody>();
                if (rb.Is_Valid())
                {
                    RIGIDBODY_DATA* pBody = rb._Data();
                    COLLIDER_DATA* pCol = col._Data();

                    if (pBody && pCol)
                    {
                        pBody->bEnable = true;
                        pBody->bGravity = true;

                        pBody->eShape = pCol->eShape;
                        pBody->eBodyType = BODY_TYPE::DYNAMIC;
                        pBody->hCollider = col.Get_Handle();

                        pBody->bDirtyMass = true;
                        pBody->bDirtyInertia = true;
                        pBody->bDirtyWorldInertia = true;

                        pCol->hRigidbody = rb.Get_Handle();
                    }
                }

                ImGui::CloseCurrentPopup();
            }
        }

        if (ImGui::MenuItem("SpringJoint"))
        {
            CRigidbody col = m_pTarget->Get_Component<CRigidbody>();
            if (!col.Is_Valid())
            {
                LOG_WARN("SpringJoint must have CRigidbody.");
            }
            else
            {
                CSpringJoint sj = m_pTarget->Add_Component<CSpringJoint>();
                ImGui::CloseCurrentPopup();
            }
        }

        if (ImGui::BeginPopupModal("RigidbodyNeedColliderPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted("Rigidbody requires a Collider component first.");
            ImGui::Spacing();

            if (ImGui::Button("OK", ImVec2(120.f, 0.f)))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::MenuItem("MeshRenderer"))
        {
            m_pTarget->Add_Component<CMeshRenderer>();
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::MenuItem("Camera"))
        {
            m_pTarget->Add_Component<CCamera>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Animator"))
        {
            m_pTarget->Add_Component<CAnimator>();
            ImGui::CloseCurrentPopup();
        }
    }
    else   /* ------------------------------ UI ----------------------------------*/
    {
        if (ImGui::MenuItem("CanvasRenderer"))
        {
            m_pTarget->Add_Component<CCanvasRenderer>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Image"))
        {
            m_pTarget->Add_Component<CUIImage>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Button"))
        {
            m_pTarget->Add_Component<CUIButton>();
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::MenuItem("Text"))
        {
            m_pTarget->Add_Component<CUIText>();
            ImGui::CloseCurrentPopup();
        }
    }

    /* ------------------------------ Common ----------------------------------*/
    if (ImGui::MenuItem("Script"))
    {
        m_pTarget->Add_Component<CScript>();
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void CInspectorPanel::Draw_CreateScriptPopup()
{

}

void CInspectorPanel::Draw_Asset()
{
    const std::string name = m_selectedAsset.path.filename().string();
    const std::string path = m_selectedAsset.path.string();

    ImGui::TextUnformatted("Asset");
    ImGui::Separator();

    ImGui::Text("Name: %s", name.c_str());
    ImGui::Text("Type: %s", CAsset_Registry::AssetType_ToStr(m_selectedAsset.type));
    ImGui::Text("Path: %s", path.c_str());
    ImGui::Text("Directory: %s", m_selectedAsset.isDirectory ? "true" : "false");
}

void CInspectorPanel::Draw_None()
{
    ImGui::TextUnformatted("No selection.");
}


void CInspectorPanel::Draw_ComponentByType(COMPONENT_TYPE eComType)
{
    switch(eComType)
    {
    case COMPONENT_TYPE::COLLIDER :
        Draw_Collider();
        break;
    case COMPONENT_TYPE::RIGIDBODY :
        Draw_Rigidbody();
        break;
    case COMPONENT_TYPE::SPRING_JOINT :
        Draw_SpringJoint();
        break;
    case COMPONENT_TYPE::MESH_RENDERER :
        Draw_MeshRenderer();
        break;
    case COMPONENT_TYPE::CAMERA :
        Draw_Camera();
        break;
    case COMPONENT_TYPE::ANIMATOR :
        Draw_Animator();
        break;
    case COMPONENT_TYPE::CANVAS_RENDERER:
        Draw_CanvasRenderer();
        break;
    case COMPONENT_TYPE::SCRIPT:
        Draw_Script();
        break;
    case COMPONENT_TYPE::UI_IMAGE:
        Draw_UIImage();
        break;
    case COMPONENT_TYPE::UI_BUTTON:
        Draw_UIButton();
        break;
    case COMPONENT_TYPE::UI_TEXT:
        Draw_UIText();
        break;
    }
}

void CInspectorPanel::Draw_Transform()
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    CTransform transform = m_pTarget->Get_Component<CTransform>();
    TRANSFORM_DATA* pData = transform._Data();
    if (!pData)
        return;

    /* 타겟이 바뀐 경우에만 1회 동기화 */
    if (m_pCachedTransformTarget != m_pTarget)
    {
        m_vCachedRotationEuler = transform.Get_Rotation_Euler();
        m_pCachedTransformTarget = m_pTarget;
    }

    _float vPos[3] = { pData->vPosition.x, pData->vPosition.y, pData->vPosition.z };
    _float vScl[3] = { pData->vScale.x, pData->vScale.y, pData->vScale.z };
    _float vRot[3] = { m_vCachedRotationEuler.x, m_vCachedRotationEuler.y, m_vCachedRotationEuler.z };

    _bool bPosChanged = false;
    _bool bRotChanged = false;
    _bool bSclChanged = false;

    ImGui::TextUnformatted("Position ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bPosChanged = ImGui::DragFloat3("##Pos", vPos, 0.1f);

    ImGui::TextUnformatted("Rotation");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bRotChanged = ImGui::DragFloat3("##Rot", vRot, 0.1f);

    ImGui::TextUnformatted("Scale     ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bSclChanged = ImGui::DragFloat3("##Scl", vScl, 0.01f);

    if (bPosChanged)
    {
        pData->vPosition = { vPos[0], vPos[1], vPos[2] };
        pData->bDirty = true;
    }

    if (bSclChanged)
    {
        pData->vScale = { vScl[0], vScl[1], vScl[2] };
        pData->bDirty = true;
    }

    if (bRotChanged) /* NOTE !! 멤버 변수를 두어 UI <-> 실제 회전값 변경 시 오류 해결 */
    {
        m_vCachedRotationEuler = { vRot[0], vRot[1], vRot[2] };
        transform.Set_Rotation_Euler(m_vCachedRotationEuler);
        pData->bDirty = true;
    }
}

void CInspectorPanel::Draw_RectTransform()
{
    if (!ImGui::CollapsingHeader("RectTransform", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    CRectTransform rt = m_pTarget->Get_Component<CRectTransform>();
    if (!rt.Is_Valid())
        return;

    RECTTRANSFORM_DATA* pData = rt._Data();
    if (!pData)
        return;


    _int vPos[2] = { SCAST(_int, pData->vPosPx.x),  SCAST(_int, pData->vPosPx.y) };
    _int vSize[2] = { SCAST(_int,pData->vSizePx.x), SCAST(_int,pData->vSizePx.y) };

    _bool bChanged = false;

    ImGui::TextUnformatted("Position");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragInt2("##UIPos", vPos, 1);

    ImGui::TextUnformatted("Size      ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragInt2("##UISize", vSize, 1);

    if (vSize[0] < 0) vSize[0] = 0;
    if (vSize[1] < 0) vSize[1] = 0;

    if (bChanged)
    {
        pData->vPosPx = { SCAST(_float, vPos[0]),  SCAST(_float, vPos[1]) };
        pData->vSizePx = { SCAST(_float, vSize[0]), SCAST(_float, vSize[1]) };
        pData->bDirty = true;
    }
}

void CInspectorPanel::Draw_MeshRenderer()
{
    CMeshRenderer mr = m_pTarget->Get_Component<CMeshRenderer>();
    if (!mr.Is_Valid())
        return;

    MESH_RENDERER_DATA* pData = mr._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("MeshRenderer_Header");
    const ImGuiID idCheck = window->GetID("MeshRenderer_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    _bool enabled = pData->bEnable;
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        SYS_COMPONENT.Set_Enable(COMPONENT_TYPE::MESH_RENDERER, mr.Get_Handle(), enabled);
        pData->bEnable = enabled;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("MeshRenderer", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    const bool bIsModel = SYS_RESOURCE.Is_ModelHandle(pData->hMesh);

    // --- Mesh ---
    ImGui::TextUnformatted(bIsModel ? "Model handle" : "Mesh handle");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hMesh);
    {
        uint32_t hMesh = pData->hMesh;
        if (ImGui::InputScalar("Mesh", ImGuiDataType_U32, &hMesh))
        {
            pData->hMesh = hMesh;
            bChanged = true;
        }

        Editor_Util::Draw_DropTarget_GUID("Mesh", "ASSET_GUID",
            [&](const ASSET_GUID& dropped)
            {
                auto* pAsset = SYS_ASSET.Find(dropped);
                if (!pAsset)
                    return;

                ASSET_TYPE eType = pAsset->eType;

                uint32_t newHandle = INVALID_HANDLE_UINT;
                if (eType == ASSET_TYPE::MODEL)
                    newHandle = SYS_RESOURCE.Load_Model(dropped);
                else if (eType == ASSET_TYPE::MESH)
                    newHandle = SYS_RESOURCE.Load_Mesh(dropped);
                else
                    return;

                if (newHandle != INVALID_HANDLE_UINT && newHandle != pData->hMesh)
                {
                    pData->hMesh = newHandle;
                    bChanged = true;

                    _DEBUG_INFO("MeshRenderer hMesh set=%u isModel=%d",
                        pData->hMesh, SYS_RESOURCE.Is_ModelHandle(pData->hMesh) ? 1 : 0);
                }
            },
            "Drop Mesh or Model here");
    }

    // --- Material ---
    ImGui::TextUnformatted(bIsModel ? "Fallback Material handle" : "Material handle");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hMaterial);
    {
        uint32_t hMaterial = pData->hMaterial;

        if (ImGui::InputScalar("Material", ImGuiDataType_U32, &hMaterial))
        {
            pData->hMaterial = hMaterial;
            bChanged = true;
        }

        Editor_Util::Draw_DropTarget_GUID_Typed(
            "Material",
            "ASSET_GUID",
            ASSET_TYPE::MATERIAL,
            [&](const ASSET_GUID& dropped)
            {
                const uint32_t newHandle = SYS_RESOURCE.Load_Material(dropped);
                if (newHandle != INVALID_HANDLE_UINT && newHandle != pData->hMaterial)
                {
                    pData->hMaterial = newHandle;
                    bChanged = true;
                }
            },
            bIsModel ? "Drop Fallback Material here" : "Drop Material here"
        );
    }

    MODEL_ENTRY* pModel = nullptr;

    if (bIsModel)
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        pModel = SYS_RESOURCE.Get_Model(pData->hMesh);
        if (!pModel)
        {
            ImGui::TextUnformatted("Model Parts: <invalid model>");
        }
        else
        {
            ImGui::Text("Model Parts (%zu)", pModel->parts.size());

            for (_uint i = 0; i < (_uint)pModel->parts.size(); ++i)
            {
                const auto& part = pModel->parts[i];

                ImGui::PushID((int)i);

                std::string strLabel = "Part " + std::to_string(i);
                const bool bPartOpen = ImGui::TreeNodeEx(
                    strLabel.c_str(),
                    ImGuiTreeNodeFlags_DefaultOpen |
                    ImGuiTreeNodeFlags_SpanAvailWidth);

                if (bPartOpen)
                {
                    ImGui::Text("Mesh Handle: %u", part.hMesh);
                    ImGui::Text("Material Handle: %u", part.hMaterial);

                    const uint32_t hResolvedMaterial =
                        (part.hMaterial != INVALID_HANDLE_UINT) ? part.hMaterial : pData->hMaterial;

                    ImGui::Text("Resolved Material: %u", hResolvedMaterial);

                    if (part.hMaterial == INVALID_HANDLE_UINT)
                        ImGui::TextUnformatted("Using MeshRenderer fallback material.");

                    ImGui::TreePop();
                }

                ImGui::PopID();
            }
        }
    }

    if (m_pTarget->Get_Parent() != nullptr)
    {
        const char* items[] = { "None", "Parts", "Attach" };

        if (!m_bEditModeInitialized)
        {
            m_eEditMode = pData->eMode;
            m_strEditAttachBoneName = pData->strAttachBoneName;
            m_bEditModeInitialized = true;
        }

        int current = static_cast<int>(m_eEditMode);
        ImGui::Combo("##MeshMode", &current, items, IM_ARRAYSIZE(items));
        m_eEditMode = static_cast<MESH_MODE>(current);

        if (m_eEditMode == MESH_MODE::ATTACH)
        {
            CGameObject* pParent = m_pTarget->Get_Parent();
            if (pParent != nullptr)
            {
                CMeshRenderer parentMR = pParent->Get_Component<CMeshRenderer>();
                auto* pParentMRData = parentMR._Data();

                if (pParentMRData != nullptr)
                {
                    MODEL_ENTRY* pParentModel = nullptr;
                    if (SYS_RESOURCE.Is_ModelHandle(pParentMRData->hMesh))
                        pParentModel = SYS_RESOURCE.Get_Model(pParentMRData->hMesh);

                    if (pParentModel != nullptr && pParentModel->Has_Skeleton())
                    {
                        std::vector<const char*> vecBoneItems;
                        vecBoneItems.reserve(pParentModel->tSkeleton.bones.size());

                        int iCurrentBone = -1;

                        for (_uint i = 0; i < (_uint)pParentModel->tSkeleton.bones.size(); ++i)
                        {
                            const auto& bone = pParentModel->tSkeleton.bones[i];
                            vecBoneItems.push_back(bone.strName.c_str());

                            if (bone.strName == m_strEditAttachBoneName)
                                iCurrentBone = static_cast<int>(i);
                        }

                        ImGui::SetNextItemWidth(200.f);

                        const char* pPreview = (iCurrentBone >= 0 && iCurrentBone < (int)vecBoneItems.size())
                            ? vecBoneItems[iCurrentBone]
                            : "Select Bone";

                        if (ImGui::BeginCombo("##AttachBoneCombo", pPreview))
                        {
                            for (int i = 0; i < (int)vecBoneItems.size(); ++i)
                            {
                                const bool bSelected = (iCurrentBone == i);

                                if (ImGui::Selectable(vecBoneItems[i], bSelected))
                                {
                                    m_strEditAttachBoneName = vecBoneItems[i];
                                    iCurrentBone = i;
                                }

                                if (bSelected)
                                    ImGui::SetItemDefaultFocus();
                            }

                            ImGui::EndCombo();
                        }
                    }
                    else
                    {
                        ImGui::TextDisabled("Parent model has no skeleton");
                    }
                }
            }
        }


        ImGui::SameLine();

        if (ImGui::Button("Save"))
        {
            auto* pMRData = mr._Data();
            if (!pMRData)
                return;

            pData->eMode = m_eEditMode;

            switch (m_eEditMode)
            {
            case MESH_MODE::NONE:
            {
                pData->strAttachBoneName.clear();

                m_pMeshRenderer_Processor->Clear_AttachReference(pMRData);
                m_pMeshRenderer_Processor->Clear_SkinningReference(pMRData);

                pData->eMode = MESH_MODE::NONE;
            }
            break;

            case MESH_MODE::PARTS:
            {
                m_pMeshRenderer_Processor->Resolve_SkinningReference(mr.Get_Handle());
            }
            break;

            case MESH_MODE::ATTACH:
            {
                pData->strAttachBoneName = m_strEditAttachBoneName;

                if (!pData->strAttachBoneName.empty())
                    m_pMeshRenderer_Processor->Resolve_AttachReference(mr.Get_Handle());
            }
            break;
            }

            m_bAttachInputActive = false;
        }
    }
    else /* 내가 모델 / 부모 인 경우 스켈레톤의 뼈 목록 보여주기 */
    {
        if (bIsModel)
        {
            if (!pModel)
                pModel = SYS_RESOURCE.Get_Model(pData->hMesh);
            if (pModel != nullptr && pModel->Has_Skeleton())
            {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                std::string strBoneHeader = "Skeleton Bones (" + std::to_string(pModel->tSkeleton.bones.size()) + ")";
                if (ImGui::TreeNode(strBoneHeader.c_str()))
                {
                    for (_uint i = 0; i < (_uint)pModel->tSkeleton.bones.size(); ++i)
                    {
                        const auto& bone = pModel->tSkeleton.bones[i];

                        ImGui::PushID((int)i);

                        std::string strBoneLabel = bone.strName + "##BoneName";
                        if (ImGui::TreeNode(strBoneLabel.c_str()))
                        {
                            ImGui::Text("Index: %u", i);
                            ImGui::Text("Parent Index: %d", bone.iParentBoneIndex);
                            ImGui::TreePop();
                        }

                        ImGui::PopID();
                    }

                    ImGui::TreePop();
                }
            }
        }
    }

    // --- Layer ---
    int layer = static_cast<int>(pData->layer);
    if (ImGui::Combo("Layer", &layer, m_layerNames, IM_ARRAYSIZE(m_layerNames)))
    {
        pData->layer = static_cast<RENDER_LAYER>(layer);
        bChanged = true;
    }

    // --- Flags ---
    uint32_t flagsValue = pData->flags;
    if (ImGui::InputScalar("Flags", ImGuiDataType_U32, &flagsValue))
    {
        pData->flags = flagsValue;
        bChanged = true;
    }

    ImGui::TreePop();
}

void CInspectorPanel::Draw_CanvasRenderer()
{
    CCanvasRenderer cr = m_pTarget->Get_Component<CCanvasRenderer>();
    if (!cr.Is_Valid())
        return;

    CANVAS_RENDERER_DATA* pData = cr._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("CanvasRenderer_Header");
    const ImGuiID idCheck = window->GetID("CanvasRenderer_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    // 헤더 체크박스
    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("CanvasRenderer", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    // 비활성화면 아래 UI 회색 + 입력 막기
    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    // Material
    ImGui::TextUnformatted("Material");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hMaterial);

    // Texture
    ImGui::TextUnformatted("Texture");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hTexture);

    // Layer
    int layer = (int)pData->layer;
    if (ImGui::DragInt("Layer", &layer, 1, 0, 10))
    {
        pData->layer = (RENDER_LAYER)layer;
        bChanged = true;
    }

    // Flags
    uint32_t flagsValue = pData->flags;
    if (ImGui::InputScalar("Flags", ImGuiDataType_U32, &flagsValue))
    {
        pData->flags = flagsValue;
        bChanged = true;
    }

    // SortZ
    float sortZ = pData->sortZ;
    if (ImGui::DragFloat("SortZ", &sortZ, 0.01f, 0.f, 1.f, "%.3f"))
    {
        if (sortZ < 0.f) sortZ = 0.f;
        if (sortZ > 1.f) sortZ = 1.f;
        pData->sortZ = sortZ;
        bChanged = true;
    }

    // Color
    _float4 col = pData->vColor;
    float c[4] = { col.x, col.y, col.z, col.w };
    if (ImGui::ColorEdit4("Color", c))
    {
        pData->vColor = { c[0], c[1], c[2], c[3] };
        bChanged = true;
    }

    // UV
    float uv[4] = { pData->rcUV.fLeft, pData->rcUV.fTop, pData->rcUV.fRight, pData->rcUV.fBottom };
    if (ImGui::DragFloat4("UV (L,T,R,B)", uv, 0.001f, 0.f, 1.f, "%.3f"))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (uv[i] < 0.f) uv[i] = 0.f;
            if (uv[i] > 1.f) uv[i] = 1.f;
        }
        pData->rcUV = { uv[0], uv[1], uv[2], uv[3] };
        bChanged = true;
    }

    // Clip Rect
    bool clipEnabled = ((pData->flags & CF_CLIP_RECT) != 0);
    if (ImGui::Checkbox("Enable ClipRect", &clipEnabled))
    {
        if (clipEnabled) pData->flags |= CF_CLIP_RECT;
        else             pData->flags &= ~CF_CLIP_RECT;
        bChanged = true;
    }

    float clip[4] = { pData->rcClip.fLeft, pData->rcClip.fTop, pData->rcClip.fRight, pData->rcClip.fBottom };
    if (ImGui::DragFloat4("ClipRect (L,T,R,B)", clip, 1.f, 0.f, 0.f, "%.0f"))
    {
        pData->rcClip = { clip[0], clip[1], clip[2], clip[3] };
        bChanged = true;
    }

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    // if (bChanged)
    //     pData->bDirty = true;

    ImGui::TreePop();
}

void CInspectorPanel::Draw_Collider()
{
    CCollider col = m_pTarget->Get_Component<CCollider>();
    if (!col.Is_Valid())
        return;

    COLLIDER_DATA* pData = col._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("Collider_Header");
    const ImGuiID idCheck = window->GetID("Collider_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
        pData->bDirty = true;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("Collider", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    bool bTrigger = (pData->bTrigger);
    if (ImGui::Checkbox("bTrigger", &bTrigger))
    {
        pData->bTrigger = bTrigger ? 1 : 0;
        pData->bDirty = true;
    }

    ImGui::TextUnformatted("Shape");
    ImGui::SameLine();

    const char* shapeText = "Unknown";
    switch (pData->eShape)
    {
    case SHAPE::BOX:    shapeText = "Box"; break;
    case SHAPE::SPHERE: shapeText = "Sphere"; break;
    case SHAPE::PLANE:  shapeText = "Plane"; break;
    default: break;
    }
    ImGui::TextUnformatted(shapeText);

    float vOffset[3] = { pData->vOffset.x, pData->vOffset.y, pData->vOffset.z };
    if (ImGui::DragFloat3("Offset", vOffset, 0.01f, 0.f, 0.f, "%.3f"))
    {
        pData->vOffset = { vOffset[0], vOffset[1], vOffset[2] };
        bChanged = true;
    }

    if (ImGui::DragFloat3("Rotation Offset", &pData->vRotationOffset.x, 0.1f))
    {
        pData->bDirty = true;
        bChanged = true;
    }

    ImGui::TextUnformatted("On Collision");
    ImGui::SameLine();
    ImGui::TextUnformatted(pData->bOnCol ? "True" : "False");

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    switch (pData->eShape)
    {
    case SHAPE::BOX:
    {
        float vHalfExtents[3] =
        {
            pData->box.vHalfExtentsLocal.x,
            pData->box.vHalfExtentsLocal.y,
            pData->box.vHalfExtentsLocal.z
        };

        if (ImGui::DragFloat3("Half Extents", vHalfExtents, 0.01f, 0.001f, 0.f, "%.3f"))
        {
            if (vHalfExtents[0] < 0.001f) vHalfExtents[0] = 0.001f;
            if (vHalfExtents[1] < 0.001f) vHalfExtents[1] = 0.001f;
            if (vHalfExtents[2] < 0.001f) vHalfExtents[2] = 0.001f;

            pData->box.vHalfExtentsLocal = { vHalfExtents[0], vHalfExtents[1], vHalfExtents[2] };
            bChanged = true;
        }
        break;
    }

    case SHAPE::SPHERE:
    {
        float fRadius = pData->sphere.fRadiusLocal;
        if (ImGui::DragFloat("Radius", &fRadius, 0.01f, 0.001f, 0.f, "%.3f"))
        {
            if (fRadius < 0.001f)
                fRadius = 0.001f;

            pData->sphere.fRadiusLocal = fRadius;
            bChanged = true;
        }
        break;
    }

    case SHAPE::PLANE:
    {
        bool bInfinite = (pData->plane.bInfinite != 0);
        if (ImGui::Checkbox("Infinite", &bInfinite))
        {
            pData->plane.bInfinite = bInfinite ? 1 : 0;
            bChanged = true;
        }

        if (!bInfinite)
        {
            float vDimension[2] =
            {
                pData->plane.vDimension.x,
                pData->plane.vDimension.y
            };

            if (ImGui::DragFloat2("Dimension", vDimension, 0.01f, 0.001f, 0.f, "%.3f"))
            {
                if (vDimension[0] < 0.001f) vDimension[0] = 0.001f;
                if (vDimension[1] < 0.001f) vDimension[1] = 0.001f;

                pData->plane.vDimension = { vDimension[0], vDimension[1] };
                bChanged = true;
            }
        }
        break;
    }

    default:
        break;
    }

    if (bChanged)
        pData->bDirty = true;

    ImGui::TreePop();
}

void CInspectorPanel::Draw_Rigidbody()
{
    CRigidbody rb = m_pTarget->Get_Component<CRigidbody>();
    if (!rb.Is_Valid())
        return;

    RIGIDBODY_DATA* pData = rb._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("Rigidbody_Header");
    const ImGuiID idCheck = window->GetID("Rigidbody_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("Rigidbody", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    ImGui::TextUnformatted("Body Type");
    ImGui::SameLine();

    const char* bodyTypeItems[] =
    {
        "Dynamic",
        "Kinematic"
    };

    int currentBodyType = 0;
    switch (pData->eBodyType)
    {
    case BODY_TYPE::DYNAMIC:   currentBodyType = 0; break;
    case BODY_TYPE::KINEMATIC: currentBodyType = 1; break;
    default:                   currentBodyType = 0; break;
    }

    ImGui::SetNextItemWidth(140.f);
    if (ImGui::Combo("##BodyType", &currentBodyType, bodyTypeItems, IM_ARRAYSIZE(bodyTypeItems)))
    {
        switch (currentBodyType)
        {
        case 0:
            pData->eBodyType = BODY_TYPE::DYNAMIC;
            break;

        case 1:
            pData->eBodyType = BODY_TYPE::KINEMATIC;
            break;
        }

        pData->bDirtyMass = true;
    }

    ImGui::TextUnformatted("Shape");
    ImGui::SameLine();

    const char* shapeText = "Unknown";
    switch (pData->eShape)
    {
    case SHAPE::BOX:    shapeText = "Box"; break;
    case SHAPE::SPHERE: shapeText = "Sphere"; break;
    case SHAPE::PLANE:  shapeText = "Plane"; break;
    default: break;
    }
    ImGui::TextUnformatted(shapeText);

    ImGui::SeparatorText("Constraints");
    ImGui::TextUnformatted("Position Lock");

    bool lockPosX = (pData->tPositionLock.bX != 0);
    if (ImGui::Checkbox("X##Pos", &lockPosX))
    {
        pData->tPositionLock.bX = lockPosX ? 1 : 0;
        bChanged = true;
    }
    ImGui::SameLine();

    bool lockPosY = (pData->tPositionLock.bY != 0);
    if (ImGui::Checkbox("Y##Pos", &lockPosY))
    {
        pData->tPositionLock.bY = lockPosY ? 1 : 0;
        bChanged = true;
    }
    ImGui::SameLine();

    bool lockPosZ = (pData->tPositionLock.bZ != 0);
    if (ImGui::Checkbox("Z##Pos", &lockPosZ))
    {
        pData->tPositionLock.bZ = lockPosZ ? 1 : 0;
        bChanged = true;
    }

    ImGui::TextUnformatted("Rotation Lock");

    bool lockRotX = (pData->tRotationLock.bX != 0);
    if (ImGui::Checkbox("X##Rot", &lockRotX))
    {
        pData->tRotationLock.bX = lockRotX ? 1 : 0;
        bChanged = true;
    }
    ImGui::SameLine();

    bool lockRotY = (pData->tRotationLock.bY != 0);
    if (ImGui::Checkbox("Y##Rot", &lockRotY))
    {
        pData->tRotationLock.bY = lockRotY ? 1 : 0;
        bChanged = true;
    }
    ImGui::SameLine();

    bool lockRotZ = (pData->tRotationLock.bZ != 0);
    if (ImGui::Checkbox("Z##Rot", &lockRotZ))
    {
        pData->tRotationLock.bZ = lockRotZ ? 1 : 0;
        bChanged = true;
    }

    ImGui::Separator();

    bool gravity = (pData->bGravity != 0);
    if (ImGui::Checkbox("Use Gravity", &gravity))
    {
        pData->bGravity = gravity ? 1 : 0;
        bChanged = true;
    }

    float fMass = pData->fMass;
    if (ImGui::DragFloat("Mass", &fMass, 0.01f, 0.001f, 100000.f, "%.3f"))
    {
        if (fMass < 0.001f)
            fMass = 0.001f;

        pData->fMass = fMass;
        pData->bDirtyMass = true;
        pData->bDirtyInertia = true;
        pData->bDirtyWorldInertia = true;
        bChanged = true;
    }

    float fDrag = pData->fDrag;
    if (ImGui::DragFloat("Drag", &fDrag, 0.01f, 0.f, 1000.f, "%.3f"))
    {
        if (fDrag < 0.f)
            fDrag = 0.f;

        pData->fDrag = fDrag;
        bChanged = true;
    }

    float fAngularDrag = pData->fAngularDrag;
    if (ImGui::DragFloat("Angular Drag", &fAngularDrag, 0.01f, 0.f, 1000.f, "%.3f"))
    {
        if (fAngularDrag < 0.f)
            fAngularDrag = 0.f;

        pData->fAngularDrag = fAngularDrag;
        bChanged = true;
    }

    float fRestitution = pData->fRestitution;
    if (ImGui::DragFloat("Restitution", &fRestitution, 0.01f, 0.f, 1.f, "%.3f"))
    {
        if (fRestitution < 0.f) fRestitution = 0.f;
        if (fRestitution > 1.f) fRestitution = 1.f;

        pData->fRestitution = fRestitution;
        bChanged = true;
    }

    float fFriction = pData->fFriction;
    if (ImGui::DragFloat("Friction", &fFriction, 0.01f, 0.f, 10.f, "%.3f"))
    {
        if (fFriction < 0.f)
            fFriction = 0.f;

        pData->fFriction = fFriction;
        bChanged = true;
    }

    ImGui::Separator();

    ImGui::Text("Linear Vel   : (%.2f, %.2f, %.2f)",
        pData->vLinearVel.x,
        pData->vLinearVel.y,
        pData->vLinearVel.z);

    ImGui::Text("Angular Vel  : (%.2f, %.2f, %.2f)",
        pData->vAngularVel.x,
        pData->vAngularVel.y,
        pData->vAngularVel.z);

    ImGui::Text("Force Accum  : (%.2f, %.2f, %.2f)",
        pData->vForceAccum.x,
        pData->vForceAccum.y,
        pData->vForceAccum.z);

    ImGui::Text("Torque Accum : (%.2f, %.2f, %.2f)",
        pData->vTorqueAccum.x,
        pData->vTorqueAccum.y,
        pData->vTorqueAccum.z);

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    if (bChanged)
    {
        pData->bDirtyMass = true;
        pData->bDirtyInertia = true;
        pData->bDirtyWorldInertia = true;
    }

    ImGui::TreePop();
}

void CInspectorPanel::Draw_SpringJoint()
{
    CSpringJoint springJoint = m_pTarget->Get_Component<CSpringJoint>();
    if (!springJoint.Is_Valid())
        return;

    SPRING_JOINT_DATA* pData = springJoint._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("SpringJoint_Header");
    const ImGuiID idCheck = window->GetID("SpringJoint_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("SpringJoint", flags);

    ImGui::PopID();

    if (!open)
        return;

    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    bool bChanged = false;

    bool bUseSpring = (pData->bUseSpring != 0);
    if (ImGui::Checkbox("Use Spring", &bUseSpring))
    {
        pData->bUseSpring = bUseSpring ? 1 : 0;
        bChanged = true;
    }

    _float3 vAnchor = pData->vAnchor;
    if (ImGui::DragFloat3("Anchor", &vAnchor.x, 0.1f, -100000.f, 100000.f, "%.2f"))
    {
        pData->vAnchor = vAnchor;
        bChanged = true;
    }

    float fSpring = pData->fSpring;
    if (ImGui::DragFloat("Spring", &fSpring, 0.1f, 0.f, 100000.f, "%.3f"))
    {
        if (fSpring < 0.f)
            fSpring = 0.f;

        pData->fSpring = fSpring;
        bChanged = true;
    }

    float fDamper = pData->fDamper;
    if (ImGui::DragFloat("Damper", &fDamper, 0.1f, 0.f, 100000.f, "%.3f"))
    {
        if (fDamper < 0.f)
            fDamper = 0.f;

        pData->fDamper = fDamper;
        bChanged = true;
    }

    float fRestLength = pData->fRestLength;
    if (ImGui::DragFloat("Rest Length", &fRestLength, 0.01f, 0.f, 100000.f, "%.3f"))
    {
        if (fRestLength < 0.f)
            fRestLength = 0.f;

        pData->fRestLength = fRestLength;
        bChanged = true;
    }

    bool bUseMinLength = (pData->bUseMinLength != 0);
    if (ImGui::Checkbox("Use Min Length", &bUseMinLength))
    {
        pData->bUseMinLength = bUseMinLength ? 1 : 0;
        bChanged = true;
    }

    if (pData->bUseMinLength)
    {
        float fMinLength = pData->fMinLength;
        if (ImGui::DragFloat("Min Length", &fMinLength, 0.01f, 0.f, 100000.f, "%.3f"))
        {
            if (fMinLength < 0.f)
                fMinLength = 0.f;

            pData->fMinLength = fMinLength;
            bChanged = true;
        }
    }

    bool bUseMaxLength = (pData->bUseMaxLength != 0);
    if (ImGui::Checkbox("Use Max Length", &bUseMaxLength))
    {
        pData->bUseMaxLength = bUseMaxLength ? 1 : 0;
        bChanged = true;
    }

    if (pData->bUseMaxLength)
    {
        float fMaxLength = pData->fMaxLength;
        if (ImGui::DragFloat("Max Length", &fMaxLength, 0.01f, 0.f, 100000.f, "%.3f"))
        {
            if (fMaxLength < 0.f)
                fMaxLength = 0.f;

            pData->fMaxLength = fMaxLength;
            bChanged = true;
        }
    }

    ImGui::Separator();

    const bool bConnected = (pData->hRigidbody != INVALID_HANDLE);
    ImGui::Text("Rigidbody : %s", bConnected ? "Connected" : "None");

    ImGui::Text("Anchor    : (%.2f, %.2f, %.2f)",
        pData->vAnchor.x,
        pData->vAnchor.y,
        pData->vAnchor.z);

    if (bConnected)
    {
        CRigidbody rb = m_pTarget->Get_Component<CRigidbody>();
        if (rb.Is_Valid())
        {
            RIGIDBODY_DATA* pRbData = rb._Data();
            if (pRbData)
            {
                ImGui::Text("Linear Vel: (%.2f, %.2f, %.2f)",
                    pRbData->vLinearVel.x,
                    pRbData->vLinearVel.y,
                    pRbData->vLinearVel.z);
            }
        }
    }

    if (bChanged)
    {
        if (pData->fSpring < 0.f)     pData->fSpring = 0.f;
        if (pData->fDamper < 0.f)     pData->fDamper = 0.f;
        if (pData->fRestLength < 0.f) pData->fRestLength = 0.f;
        if (pData->fMinLength < 0.f)  pData->fMinLength = 0.f;
        if (pData->fMaxLength < 0.f)  pData->fMaxLength = 0.f;

        if (pData->bUseMinLength && pData->fMinLength > pData->fRestLength && pData->fRestLength > 0.f)
            pData->fMinLength = pData->fRestLength;

        if (pData->bUseMaxLength && pData->fMaxLength < pData->fRestLength)
            pData->fMaxLength = pData->fRestLength;
    }

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    ImGui::TreePop();
}

void CInspectorPanel::Draw_Animator()
{
    CAnimator animator = m_pTarget->Get_Component<CAnimator>();
    if (!animator.Is_Valid())
        return;

    ANIMATOR_DATA* pData = animator._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("Animator_Header");
    const ImGuiID idCheck = window->GetID("Animator_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("Animator", flags);

    ImGui::PopID();

    if (!open)
        return;

    ImGui::SameLine(ImGui::GetCursorPosX() + 170.0f);

    if (ImGui::Button("Blend"))
    {
        ImGui::OpenPopup("Animator Blend Editor");
    }

    ImGui::SameLine(0.0f, 10.0f);

    Draw_AnimatorBlendingView();

    if (ImGui::Button("Loop"))
    {
        ImGui::OpenPopup("Animator Loop Editor");
    }

    Draw_AnimatorLoopView();

    const bool bDisabledScope = (pData->bEnable == 0);
    if (bDisabledScope)
        ImGui::BeginDisabled();

    bool bChanged = false;

    bool bPlaying = (pData->bPlaying != 0);
    if (ImGui::Checkbox("Playing", &bPlaying))
    {
        pData->bPlaying = bPlaying ? 1 : 0;
        bChanged = true;
    }

    float fPlaySpeed = pData->fPlaySpeed;
    if (ImGui::DragFloat("Play Speed", &fPlaySpeed, 0.01f, 0.f, 10.f, "%.3f"))
    {
        if (fPlaySpeed < 0.f)
            fPlaySpeed = 0.f;

        pData->fPlaySpeed = fPlaySpeed;
        bChanged = true;
    }

    MODEL_ENTRY* pModel = nullptr;

    if (m_pTarget)
    {
        CMeshRenderer mr = m_pTarget->Get_Component<CMeshRenderer>();
        if (mr.Is_Valid())
        {
            pModel = SYS_RESOURCE.Get_Model(mr->hMesh);
        }
    }

    ImGui::Separator();

    if (!pModel)
    {
        ImGui::TextDisabled("No model.");
    }
    else if (!pModel->Has_Animation())
    {
        ImGui::TextDisabled("No animation clips.");
    }
    else
    {
        if (pData->iAnimationClip == INVALID_HANDLE_UINT ||
            pData->iAnimationClip >= pModel->vecAnimClips.size())
        {
            pData->iAnimationClip = 0;
        }

        const ANIMATION_CLIP_ENTRY& curClip = pModel->vecAnimClips[pData->iAnimationClip];

        ImGui::Text("Animation Clip Count : %d", SCAST(int, pModel->vecAnimClips.size()));
        ImGui::Text("Current Clip : %s", curClip.strName.c_str());
        ImGui::Text("Duration : %.3f", curClip.fDuration);
        ImGui::Text("Tick Per Second : %.3f", curClip.fTickPerSecond);

        int iCurrentClip = SCAST(int, pData->iAnimationClip);
        if (ImGui::BeginCombo("##Animation Clip", curClip.strName.c_str()))
        {
            for (int i = 0; i < SCAST(int, pModel->vecAnimClips.size()); ++i)
            {
                const bool bSelected = (iCurrentClip == i);
                const char* pClipName = pModel->vecAnimClips[i].strName.empty()
                    ? "<Unnamed Clip>"
                    : pModel->vecAnimClips[i].strName.c_str();

                if (ImGui::Selectable(pClipName, bSelected))
                {
                    animator.Set_NextAnimationClip(pClipName);
                }

                if (bSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::SameLine();

        if (ImGui::Button("Copy"))
        {
            std::string strClipNames;

            for (int i = 0; i < SCAST(int, pModel->vecAnimClips.size()); ++i)
            {
                const char* pClipName = pModel->vecAnimClips[i].strName.empty()
                    ? "<Unnamed Clip>"
                    : pModel->vecAnimClips[i].strName.c_str();

                strClipNames += pClipName;

                if (i + 1 < SCAST(int, pModel->vecAnimClips.size()))
                    strClipNames += "\n";
            }

            ImGui::SetClipboardText(strClipNames.c_str());
        }

        float fTrackPosition = pData->fTrackPosition;
        const float fMaxTrack = curClip.fDuration > 0.f ? curClip.fDuration : 0.f;
        if (ImGui::DragFloat("Track Position", &fTrackPosition, 0.01f, 0.f, fMaxTrack, "%.3f"))
        {
            if (fTrackPosition < 0.f)
                fTrackPosition = 0.f;

            if (curClip.fDuration > 0.f && fTrackPosition > curClip.fDuration)
                fTrackPosition = curClip.fDuration;

            pData->fTrackPosition = fTrackPosition;
            bChanged = true;
        }
    }

    if (bChanged)
    {
        if (pData->fPlaySpeed < 0.f)
            pData->fPlaySpeed = 0.f;

        if (pModel && pModel->Has_Animation())
        {
            if (pData->iAnimationClip == INVALID_HANDLE_UINT ||
                pData->iAnimationClip >= pModel->vecAnimClips.size())
            {
                pData->iAnimationClip = 0;
            }

            const ANIMATION_CLIP_ENTRY& clip = pModel->vecAnimClips[pData->iAnimationClip];

            if (pData->fTrackPosition < 0.f)
                pData->fTrackPosition = 0.f;

            if (clip.fDuration > 0.f && pData->fTrackPosition > clip.fDuration)
                pData->fTrackPosition = clip.fDuration;
        }
        else
        {
            pData->iAnimationClip = INVALID_HANDLE_UINT;
            pData->fTrackPosition = 0.f;
            pData->currentKeyFrameIndices.clear();
        }
    }

    if (bDisabledScope)
        ImGui::EndDisabled();

    ImGui::TreePop();
}

void CInspectorPanel::Draw_Camera()
{
    CCamera camera = m_pTarget->Get_Component<CCamera>();
    if (!camera.Is_Valid())
        return;

    CAMERA_DATA* pData = camera._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImGuiID idHeader = window->GetID("Camera_Header");
    const ImGuiID idCheck = window->GetID("Camera_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
        pData->dirty = true;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("Camera", flags);

    ImGui::PopID();

    if (!open)  return;

    if (pData->bEnable == 0) ImGui::BeginDisabled();

    bool bChanged = false;

    bool bOrthographic = (pData->bOrthographic != 0);
    if (ImGui::Checkbox("Orthographic", &bOrthographic))
    {
        pData->bOrthographic = bOrthographic ? 1 : 0;
        bChanged = true;
    }

    float fAspect = pData->fAspect;
    if (ImGui::DragFloat("Aspect", &fAspect, 0.001f, 0.001f, 100.f, "%.3f"))
    {
        if (fAspect < 0.001f)
            fAspect = 0.001f;

        pData->fAspect = fAspect;
        bChanged = true;
    }

    float fNear = pData->fNear;
    if (ImGui::DragFloat("Near", &fNear, 0.01f, 0.001f, 100000.f, "%.3f"))
    {
        if (fNear < 0.001f)
            fNear = 0.001f;

        pData->fNear = fNear;
        bChanged = true;
    }

    float fFar = pData->fFar;
    if (ImGui::DragFloat("Far", &fFar, 0.1f, 0.001f, 1000000.f, "%.3f"))
    {
        if (fFar < 0.001f)
            fFar = 0.001f;

        pData->fFar = fFar;
        bChanged = true;
    }

    if (pData->bOrthographic)
    {
        float fOrthoSize = pData->fOrthoSize;
        if (ImGui::DragFloat("Ortho Size", &fOrthoSize, 0.01f, 0.001f, 100000.f, "%.3f"))
        {
            if (fOrthoSize < 0.001f)
                fOrthoSize = 0.001f;

            pData->fOrthoSize = fOrthoSize;
            bChanged = true;
        }
    }
    else
    {
        float fFovy = pData->fFovy;
        if (ImGui::DragFloat("FOV Y", &fFovy, 0.1f, 1.f, 179.f, "%.3f"))
        {
            if (fFovy < 1.f)
                fFovy = 1.f;
            if (fFovy > 179.f)
                fFovy = 179.f;

            pData->fFovy = fFovy;
            bChanged = true;
        }
    }

    ImGui::Separator();

    uint32_t layerMask = pData->layerMask;
    int layerMaskInt = SCAST(int, layerMask);
    if (ImGui::InputInt("Layer Mask", &layerMaskInt))
    {
        pData->layerMask = SCAST(uint32_t, layerMaskInt);
        bChanged = true;
    }

    int iPriority = (int)pData->iPriority;
    if (ImGui::InputInt("Render Priority", &iPriority))
    {
        pData->iPriority = (uint8_t)iPriority;
        bChanged = true;
    }

    if (bChanged)
    {
        if (pData->fAspect < 0.001f)
            pData->fAspect = 0.001f;

        if (pData->fNear < 0.001f)
            pData->fNear = 0.001f;

        if (pData->fFar < 0.001f)
            pData->fFar = 0.001f;

        if (pData->fFar < pData->fNear)
            pData->fFar = pData->fNear + 0.001f;

        if (pData->fOrthoSize < 0.001f)
            pData->fOrthoSize = 0.001f;

        if (pData->fFovy < 1.f)
            pData->fFovy = 1.f;
        if (pData->fFovy > 179.f)
            pData->fFovy = 179.f;

        pData->dirty = true;
    }

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    ImGui::TreePop();
}

void CInspectorPanel::Draw_Script()
{
    if (!m_pTarget)
        return;

    auto vecScripts = m_pTarget->Get_Components<CScript>();
    if (vecScripts.empty())
        return;

    for (size_t i = 0; i < vecScripts.size(); ++i)
    {
        if (i != 0)
            ImGui::Separator();

        Draw_AllScripts(vecScripts[i].Get_Handle());
    }
}

void CInspectorPanel::Draw_AllScripts(COMPONENT_HANDLE hComponent)
{
    if (!m_pTarget)
        return;

    // 1) 이 오브젝트가 가진 모든 Script 프록시 중에서, 요청된 핸들을 찾는다.
    auto vecScripts = m_pTarget->Get_Components<CScript>();

    CScript sc;
    SCRIPT_DATA* pData = nullptr;

    for (auto& it : vecScripts)
    {
        if (it.Get_Handle().iHandle == hComponent.iHandle)
        {
            sc = it;
            pData = sc._Data();
            break;
        }
    }

    if (!sc.Is_Valid())
        return;
    if (!pData)
        return;

    // ------------------------------------------------------------
    // Header : [Enable Checkbox] + [Collapsible "Script"]
    // ------------------------------------------------------------
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    // 각 스크립트 인스턴스별로 ImGui ID 충돌 방지
    ImGui::PushID(SCAST(int, hComponent.iHandle));

    const ImGuiID idHeader = window->GetID("Script_Header");
    const ImGuiID idCheck = window->GetID("Script_Enable");

    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    // 헤더 체크박스가 실제 enable을 담당
    _bool enabled = pData->bEnable;
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        m_pScript_Processor->Set_Enable(COMPONENT_TYPE::SCRIPT, sc.Get_Handle(), enabled);
        pData->bEnable = enabled;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("Script", flags);

    ImGui::PopID();

    if (!open)
    {
        ImGui::PopID();
        return;
    }

    bool bChanged = false;

    /* TypeId, 런타임 인덱스 */
    ImGui::Text("TypeID: %u", (uint32_t)pData->iTypeID);

    // ------------------------------------------------------------
    // 캐시
    // ------------------------------------------------------------
    Engine::ASSET_GUID guid{};
    m_pScript_Processor->Try_Get_Guid_By_TypeID(pData->iTypeID, guid);

    const bool bHasBind = guid.Is_Valid();

    std::filesystem::path assetPath;
    const Engine::ASSET_RECORD* pRecord = nullptr;

    if (bHasBind)
    {
        assetPath = SYS_ASSET.Get_Asset_Path(guid);
        if (!assetPath.empty())
            pRecord = SYS_ASSET.Find(guid);
    }

    std::string boundClassName;
    std::string boundHeaderFileName;

    auto read_text_file_utf8 = [&](const std::filesystem::path& p, std::string& outText) -> bool
        {
            outText.clear();
            std::ifstream ifs(p, std::ios_base::binary);
            if (!ifs.is_open())
                return false;

            ifs.seekg(0, std::ios_base::end);
            const std::streamsize sz = ifs.tellg();
            if (sz <= 0)
                return false;
            ifs.seekg(0, std::ios_base::beg);

            outText.resize((size_t)sz);
            ifs.read(outText.data(), sz);
            return true;
        };

    auto try_parse_script_json = [&]()
        {
            if (!bHasBind) return;
            if (assetPath.empty()) return;
            if (!std::filesystem::exists(assetPath)) return;

            std::string txt;
            if (!read_text_file_utf8(assetPath, txt))
                return;

            try
            {
                json j = json::parse(txt);
                if (j.contains("ClassName") && j["ClassName"].is_string())
                    boundClassName = j["ClassName"].get<std::string>();
                if (j.contains("Header") && j["Header"].is_string())
                    boundHeaderFileName = j["Header"].get<std::string>();
            }
            catch (...)
            {
            }
        };

    /* 한 번만 파싱 */
    try_parse_script_json();

    /* 스크립트 바인딩 상태 */
    ImGui::TextUnformatted("Class");
    ImGui::SameLine();
    if (!bHasBind)
        ImGui::TextUnformatted("EMPTY");
    else
        ImGui::TextUnformatted(!boundClassName.empty() ? boundClassName.c_str() : "<unknown class>");

    // 파일 경로 표시 + "..." 버튼(열기/탐색기)
    if (bHasBind)
    {
        if (!assetPath.empty())
        {
            ImGui::SameLine();
            if (ImGui::SmallButton(".script"))
            {
                Editor_Util::RevealFile_In_Explorer(assetPath);
            }

            /* 바인딩 상태 표시 */
            ImGui::SameLine();
            if (ImGui::SmallButton("Open"))
            {
                /* 우선순위: Header(JSON의 Header) -> stem 기반 header/cpp -> 부모 폴더 */
                std::filesystem::path headerPath;
                std::filesystem::path cppPath;

                /* .script의 파일명(stem) */
                const std::string stem = assetPath.stem().string();

                const std::filesystem::path headerRoot = std::filesystem::path(Engine::ProjectConfig::CLIENT) / Engine::ProjectConfig::HEADER;
                const std::filesystem::path implRoot = std::filesystem::path(Engine::ProjectConfig::CLIENT) / Engine::ProjectConfig::IMPL;

                if (!boundHeaderFileName.empty())
                    headerPath = headerRoot / boundHeaderFileName;
                else
                    headerPath = headerRoot / (stem + ".h");

                cppPath = implRoot / (stem + ".cpp");

                auto try_open_or_reveal = [&](const std::filesystem::path& p) -> bool
                    {
                        if (p.empty() || !std::filesystem::exists(p))
                            return false;

                        Editor_Util::OpenFile_In_OS(p);
                        return true;
                    };

                bool bOpened = false;
                if (try_open_or_reveal(cppPath)) bOpened = true;
                else if (try_open_or_reveal(headerPath)) bOpened = true;

                if (!bOpened)
                {
                    /* 둘 다 없으면 .script 부모 폴더라도 열기 */
                    if (!assetPath.empty())
                        Editor_Util::RevealFile_In_Explorer(assetPath);
                    else
                        Editor_Util::RevealFile_In_Explorer(assetPath.parent_path());
                }
            }
        }
        else
        {
            ImGui::TextUnformatted("Asset Path: <not found>");
        }
    }

    // ------------------------------------------------------------
    // 드래그드롭 바인딩
    // ------------------------------------------------------------
    ImGui::Separator();
    ImGui::TextUnformatted("Bind Script");
    ImGui::SameLine();
    ImGui::TextDisabled(bHasBind ? "(Already bound)" : "(Drag script asset here)");

    ImVec2 dropSize(ImGui::GetContentRegionAvail().x, 28.f);

    /* 이미 바인딩됐으면 드롭 타겟 자체를 비활성화 */
    if (bHasBind)
        ImGui::BeginDisabled();

    const char* dropLabel = bHasBind
        ? (!boundClassName.empty() ? boundClassName.c_str() : "BOUND")
        : "EMPTY";

    ImGui::Button(dropLabel, dropSize);

    if (!bHasBind)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_GUID"))
            {
                const Engine::ASSET_GUID* pGUID = (const Engine::ASSET_GUID*)p->Data;
                if (pGUID && pGUID->Is_Valid())
                {
                    auto pRec = SYS_ASSET.Find(*pGUID);
                    if (pRec && pRec->eType == Engine::ASSET_TYPE::SCRIPT)
                    {
                        // 여기서만 바인딩
                        m_pScript_Processor->Rebind_ScriptGuid(sc.Get_Handle(), *pGUID);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }
    }

    if (bHasBind)
        ImGui::EndDisabled();
    if (bHasBind)
    {
        Engine::IScript* pScript = m_pScript_Processor->Get_Script_Instance(sc.Get_Handle());
        if (pScript)
        {
            Draw_ScriptFields(pScript);
        }
        else
        {
            ImGui::TextDisabled("Script instance is not ready.");
        }
    }

    ImGui::TreePop();
    ImGui::PopID(); // PushID(hComponent)
}

void CInspectorPanel::Draw_ScriptFields(Engine::IScript* pScript)
{
    if (nullptr == pScript)
        return;

    const SCRIPT_REFLECTION_INFO* pInfo = pScript->Get_Reflection_Info();
    if (nullptr == pInfo)
        return;

    if (false == ImGui::CollapsingHeader("Exposed Fields", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    for (const SCRIPT_FIELD_DESC& tDesc : pInfo->vecFields)
    {
        void* pField = reinterpret_cast<void*>(reinterpret_cast<char*>(pScript) + tDesc.iOffset);
        if (nullptr == pField)
            continue;

        ImGui::PushID(tDesc.strName.c_str());

        switch (tDesc.eType)
        {
        case SCRIPT_FIELD_TYPE::INT:
        {
            int* pValue = reinterpret_cast<int*>(pField);
            ImGui::InputInt(tDesc.strName.c_str(), pValue);
            break;
        }

        case SCRIPT_FIELD_TYPE::FLOAT:
        {
            float* pValue = reinterpret_cast<float*>(pField);
            ImGui::InputFloat(tDesc.strName.c_str(), pValue);
            break;
        }

        case SCRIPT_FIELD_TYPE::FLOAT2:
        {
            _float2* pValue = reinterpret_cast<_float2*>(pField);
            ImGui::InputFloat2(tDesc.strName.c_str(), &pValue->x);
            break;
        }

        case SCRIPT_FIELD_TYPE::FLOAT3:
        {
            _float3* pValue = reinterpret_cast<_float3*>(pField);
            ImGui::InputFloat3(tDesc.strName.c_str(), &pValue->x);
            break;
        }

        case SCRIPT_FIELD_TYPE::FLOAT4:
        {
            _float4* pValue = reinterpret_cast<_float4*>(pField);
            ImGui::InputFloat4(tDesc.strName.c_str(), &pValue->x);
            break;
        }
        case SCRIPT_FIELD_TYPE::DEBUG_CHAR:
        {
            const _char* pValue = reinterpret_cast<const _char*>(pField);
            if (pValue) {
                ImGui::LabelText(tDesc.strName.c_str(), "%s", pValue);
            }            break;
        }
        case SCRIPT_FIELD_TYPE::OBJECT_REF:
        {
            SCRIPT_OBJECT_REF* pValue = reinterpret_cast<SCRIPT_OBJECT_REF*>(pField);

            std::string strButtonText = tDesc.strName;
            strButtonText += " : ";

            if (pValue->hObject.Is_Valid())
            {
                CGameObject* pObject = SYS_GAMEOBJECT.Get_Wrapper(pValue->hObject);
                if (pObject)
                    strButtonText += pObject->Get_Label();
                else
                    strButtonText += "<Invalid Handle>";
            }
            else if (pValue->tUUID != INSTANCE_UUID{})
            {
                strButtonText += "<Missing Object>";
            }
            else
            {
                strButtonText += "<None>";
            }

            ImGui::Button(strButtonText.c_str(), ImVec2(-1.f, 0.f));

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("HIERARCHY_GO_PTR"))
                {
                    Engine::CGameObject* pDropped = *(Engine::CGameObject**)p->Data;
                    if (pDropped)
                    {
                        pValue->hObject = pDropped->Get_Handle();
                        pValue->tUUID = pDropped->Get_UUID();
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (ImGui::BeginPopupContextItem("ObjectRefContext"))
            {
                if (ImGui::MenuItem("Clear"))
                {
                    pValue->Clear();
                }
                ImGui::EndPopup();
            }

            break;
        }

        default:
            break;
        }

        ImGui::PopID();
    }
}

void CInspectorPanel::Draw_UIImage()
{
    CUIImage img = m_pTarget->Get_Component<CUIImage>();
    if (!img.Is_Valid())
        return;

    UI_IMAGE_DATA* pData = img._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiWindow* w = ImGui::GetCurrentWindow();
    const ImGuiID idHeader = w->GetID("UIImage_Header");
    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
        pData->dirty = true;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("UIImage", flags);

    ImGui::PopID();

    if (!open)
        return;

    bool bChanged = false;

    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    // 연결 상태 확인용: 오브젝트 핸들 / CanvasRenderer 핸들
    ImGui::TextUnformatted("Owner Object");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hObject.raw);

    ImGui::TextUnformatted("CanvasRenderer");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hCanvasRenderer);

    // dirty 플래그 확인
    bool dirty = (pData->dirty != 0);
    if (ImGui::Checkbox("Dirty", &dirty))
    {
        pData->dirty = dirty ? 1 : 0;
        bChanged = true;
    }

    // VisualPriority
    int pr = (int)pData->visualPriority;
    if (ImGui::DragInt("VisualPriority", &pr, 1.f, 0, 255))
    {
        if (pr < 0) pr = 0;
        if (pr > 255) pr = 255;
        pData->visualPriority = (uint8_t)pr;
        bChanged = true;
    }

    // Texture
    {
        uint32_t hTex = pData->hTexture;

        /* 기존: 숫자 입력(유지) */
        if (ImGui::InputScalar("Texture", ImGuiDataType_U32, &hTex))
        {
            pData->hTexture = hTex;
            bChanged = true;
        }

        /*  드래그&드롭으로 텍스처 GUID 받아서 핸들로 로드 후 세팅 */
        Editor_Util::Draw_DropTarget_GUID_Typed(
            "Texture",          // label (현재는 내부에서 안 쓰지만 그대로)
            "ASSET_GUID",       // payloadName (프로젝트 패널에서 넣는 문자열)
            ASSET_TYPE::TEXTURE,
            [&](const ASSET_GUID& dropped)
            {
                const uint32_t newHandle = SYS_RESOURCE.Load_Texture(dropped);
                if (newHandle != INVALID_HANDLE_UINT && newHandle != pData->hTexture)
                {
                    pData->hTexture = newHandle;
                    bChanged = true;
                }
            },
            "Drop Texture here"
        );
    }

    // Color
    _float4 col = pData->color;
    float c[4] = { col.x, col.y, col.z, col.w };
    if (ImGui::ColorEdit4("Color", c))
    {
        pData->color = { c[0], c[1], c[2], c[3] };
        bChanged = true;
    }

    // UV
    float uv[4] = { pData->rcUV.fLeft, pData->rcUV.fTop, pData->rcUV.fRight, pData->rcUV.fBottom };
    if (ImGui::DragFloat4("UV (L,T,R,B)", uv, 0.001f, 0.f, 1.f, "%.3f"))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (uv[i] < 0.f) uv[i] = 0.f;
            if (uv[i] > 1.f) uv[i] = 1.f;
        }
        pData->rcUV = { uv[0], uv[1], uv[2], uv[3] };
        bChanged = true;
    }

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    if (bChanged)
        pData->dirty = true;

    ImGui::TreePop();
}

void CInspectorPanel::Draw_UIButton()
{
    CUIButton btn = m_pTarget->Get_Component<CUIButton>();
    if (!btn.Is_Valid())
        return;

    UI_BUTTON_DATA* pData = btn._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiWindow* w = ImGui::GetCurrentWindow();
    const ImGuiID idHeader = w->GetID("UIButton_Header");
    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("UIButton", flags);

    ImGui::PopID();

    if (!open)
        return;

    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    ImGui::TextUnformatted("Owner Object");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hObject.raw);

    ImGui::TextUnformatted("RectTransform");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hRectTransform);

    ImGui::TextUnformatted("TargetCanvas");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hTargetCanvas);

    {
        bool interactable = (pData->bInteractable != 0);
        if (ImGui::Checkbox("Interactable", &interactable))
        {
            pData->bInteractable = interactable ? 1 : 0;
        }
    }

    {
        int state = static_cast<int>(pData->eState);
        ImGui::InputInt("State", &state, 0, 0, ImGuiInputTextFlags_ReadOnly);
    }

    {
        int pr = static_cast<int>(pData->visualPriority);
        if (ImGui::DragInt("VisualPriority", &pr, 1.f, 0, 255))
        {
            if (pr < 0) pr = 0;
            if (pr > 255) pr = 255;
            pData->visualPriority = static_cast<uint8_t>(pr);
        }
    }

    ImGui::SeparatorText("Color");

    {
        float c[4] = { pData->normal.x, pData->normal.y, pData->normal.z, pData->normal.w };
        if (ImGui::ColorEdit4("Normal Color", c))
        {
            pData->normal = { c[0], c[1], c[2], c[3] };
        }
    }

    {
        float c[4] = { pData->hover.x, pData->hover.y, pData->hover.z, pData->hover.w };
        if (ImGui::ColorEdit4("Hover Color", c))
        {
            pData->hover = { c[0], c[1], c[2], c[3] };
        }
    }

    {
        float c[4] = { pData->pressed.x, pData->pressed.y, pData->pressed.z, pData->pressed.w };
        if (ImGui::ColorEdit4("Pressed Color", c))
        {
            pData->pressed = { c[0], c[1], c[2], c[3] };
        }
    }

    {
        float c[4] = { pData->disabled.x, pData->disabled.y, pData->disabled.z, pData->disabled.w };
        if (ImGui::ColorEdit4("Disabled Color", c))
        {
            pData->disabled = { c[0], c[1], c[2], c[3] };
        }
    }

    ImGui::SeparatorText("Texture / UV");

    {
        uint32_t hTex = pData->normalTex;

        if (ImGui::InputScalar("Normal Texture", ImGuiDataType_U32, &hTex))
        {
            pData->normalTex = hTex;
        }

        Editor_Util::Draw_DropTarget_GUID_Typed(
            "Normal Texture",
            "ASSET_GUID",
            ASSET_TYPE::TEXTURE,
            [&](const ASSET_GUID& dropped)
            {
                const uint32_t newHandle = SYS_RESOURCE.Load_Texture(dropped);
                if (newHandle != INVALID_HANDLE_UINT && newHandle != pData->normalTex)
                {
                    pData->normalTex = newHandle;
                }
            },
            "Drop Normal Texture here"
        );

        float uv[4] = {
            pData->normalUV.fLeft,
            pData->normalUV.fTop,
            pData->normalUV.fRight,
            pData->normalUV.fBottom
        };

        if (ImGui::DragFloat4("Normal UV (L,T,R,B)", uv, 0.001f, 0.f, 1.f, "%.3f"))
        {
            for (int i = 0; i < 4; ++i)
            {
                if (uv[i] < 0.f) uv[i] = 0.f;
                if (uv[i] > 1.f) uv[i] = 1.f;
            }

            pData->normalUV = { uv[0], uv[1], uv[2], uv[3] };
        }
    }

    {
        uint32_t hTex = pData->hoverTex;

        if (ImGui::InputScalar("Hover Texture", ImGuiDataType_U32, &hTex))
        {
            pData->hoverTex = hTex;
        }

        Editor_Util::Draw_DropTarget_GUID_Typed(
            "Hover Texture",
            "ASSET_GUID",
            ASSET_TYPE::TEXTURE,
            [&](const ASSET_GUID& dropped)
            {
                const uint32_t newHandle = SYS_RESOURCE.Load_Texture(dropped);
                if (newHandle != INVALID_HANDLE_UINT && newHandle != pData->hoverTex)
                {
                    pData->hoverTex = newHandle;
                }
            },
            "Drop Hover Texture here"
        );

        float uv[4] = {
            pData->hoverUV.fLeft,
            pData->hoverUV.fTop,
            pData->hoverUV.fRight,
            pData->hoverUV.fBottom
        };

        if (ImGui::DragFloat4("Hover UV (L,T,R,B)", uv, 0.001f, 0.f, 1.f, "%.3f"))
        {
            for (int i = 0; i < 4; ++i)
            {
                if (uv[i] < 0.f) uv[i] = 0.f;
                if (uv[i] > 1.f) uv[i] = 1.f;
            }

            pData->hoverUV = { uv[0], uv[1], uv[2], uv[3] };
        }
    }

    {
        uint32_t hTex = pData->pressedTex;

        if (ImGui::InputScalar("Pressed Texture", ImGuiDataType_U32, &hTex))
        {
            pData->pressedTex = hTex;
        }

        Editor_Util::Draw_DropTarget_GUID_Typed(
            "Pressed Texture",
            "ASSET_GUID",
            ASSET_TYPE::TEXTURE,
            [&](const ASSET_GUID& dropped)
            {
                const uint32_t newHandle = SYS_RESOURCE.Load_Texture(dropped);
                if (newHandle != INVALID_HANDLE_UINT && newHandle != pData->pressedTex)
                {
                    pData->pressedTex = newHandle;
                }
            },
            "Drop Pressed Texture here"
        );

        float uv[4] = {
            pData->pressedUV.fLeft,
            pData->pressedUV.fTop,
            pData->pressedUV.fRight,
            pData->pressedUV.fBottom
        };

        if (ImGui::DragFloat4("Pressed UV (L,T,R,B)", uv, 0.001f, 0.f, 1.f, "%.3f"))
        {
            for (int i = 0; i < 4; ++i)
            {
                if (uv[i] < 0.f) uv[i] = 0.f;
                if (uv[i] > 1.f) uv[i] = 1.f;
            }

            pData->pressedUV = { uv[0], uv[1], uv[2], uv[3] };
        }
    }

    ImGui::SeparatorText("Event");

    ImGui::Text("OnHover Listeners: %zu", pData->OnHover.Get_ListenerCount());
    ImGui::Text("OnClick Listeners: %zu", pData->OnClick.Get_ListenerCount());

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    ImGui::TreePop();
}

void CInspectorPanel::Draw_UIText()
{
    CUIText txt = m_pTarget->Get_Component<CUIText>();
    if (!txt.Is_Valid())
        return;

    UI_TEXT_DATA* pData = txt._Data();
    if (!pData)
        return;

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiWindow* w = ImGui::GetCurrentWindow();
    const ImGuiID idHeader = w->GetID("UIText_Header");
    ImGui::PushID(idHeader);

    ImGui::AlignTextToFramePadding();

    bool enabled = (pData->bEnable != 0);
    if (ImGui::Checkbox("##Enable", &enabled))
    {
        pData->bEnable = enabled ? 1 : 0;
        pData->dirty = true;
    }

    ImGui::SameLine();

    const ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_DefaultOpen |
        ImGuiTreeNodeFlags_Framed |
        ImGuiTreeNodeFlags_SpanAvailWidth |
        ImGuiTreeNodeFlags_AllowOverlap;

    const bool open = ImGui::TreeNodeEx("UIText", flags);

    ImGui::PopID();

    if (!open)
        return;

    if (pData->bEnable == 0)
        ImGui::BeginDisabled();

    ImGui::TextUnformatted("Owner Object");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hObject.raw);

    ImGui::TextUnformatted("RectTransform");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hRectTransform);

    ImGui::TextUnformatted("CanvasRenderer");
    ImGui::SameLine();
    ImGui::Text("%u", pData->hCanvasRenderer);

    ImGui::SeparatorText("Font");

    {
        uint32_t hFont = pData->hFont;
        if (ImGui::InputScalar("Font", ImGuiDataType_U32, &hFont))
        {
            pData->hFont = hFont;
            pData->dirty = true;
        }

        Editor_Util::Draw_DropTarget_GUID_Typed(
            "Font",
            "ASSET_GUID",
            ASSET_TYPE::FONT,
            [&](const ASSET_GUID& dropped)
            {
                const uint32_t newHandle = SYS_RESOURCE.Load_Font(dropped);
                if (newHandle != INVALID_HANDLE_UINT && newHandle != pData->hFont)
                {
                    pData->hFont = newHandle;
                    pData->dirty = true;
                }
            },
            "Drop Font here"
        );
    }

    ImGui::SeparatorText("Text");

    {
#ifdef UNICODE
        std::string strUtf8;
        if (!pData->strText.empty())
        {
            const int iRequired = WideCharToMultiByte(CP_UTF8, 0, pData->strText.c_str(), -1, nullptr, 0, nullptr, nullptr);
            if (iRequired > 0)
            {
                strUtf8.resize(iRequired - 1);
                WideCharToMultiByte(CP_UTF8, 0, pData->strText.c_str(), -1, strUtf8.data(), iRequired, nullptr, nullptr);
            }
        }

        char buffer[1024] = {};
        if (!strUtf8.empty())
            strcpy_s(buffer, strUtf8.c_str());

        if (ImGui::InputTextMultiline("Text", buffer, IM_ARRAYSIZE(buffer)))
        {
            const int iRequired = MultiByteToWideChar(CP_UTF8, 0, buffer, -1, nullptr, 0);
            if (iRequired > 0)
            {
                std::wstring result;
                result.resize(iRequired - 1);
                MultiByteToWideChar(CP_UTF8, 0, buffer, -1, result.data(), iRequired);
                pData->strText = result;
                pData->dirty = true;
            }
        }
#else
        char buffer[1024] = {};
        if (!pData->strText.empty())
            strcpy_s(buffer, pData->strText.c_str());

        if (ImGui::InputTextMultiline("Text", buffer, IM_ARRAYSIZE(buffer)))
        {
            pData->strText = buffer;
            pData->dirty = true;
        }
#endif
    }

    ImGui::SeparatorText("Appearance");

    {
        float c[4] = { pData->color.x, pData->color.y, pData->color.z, pData->color.w };
        if (ImGui::ColorEdit4("Color", c))
        {
            pData->color = { c[0], c[1], c[2], c[3] };
            pData->dirty = true;
        }
    }

    {
        float fScale = pData->fScale;
        if (ImGui::DragFloat("Scale", &fScale, 0.01f, 0.f, 100.f, "%.2f"))
        {
            if (fScale < 0.f)
                fScale = 0.f;

            pData->fScale = fScale;
            pData->dirty = true;
        }
    }

    {
        int pr = static_cast<int>(pData->visualPriority);
        if (ImGui::DragInt("VisualPriority", &pr, 1.f, 0, 255))
        {
            if (pr < 0) pr = 0;
            if (pr > 255) pr = 255;
            pData->visualPriority = static_cast<uint8_t>(pr);
            pData->dirty = true;
        }
    }

    ImGui::SeparatorText("Render");

    {
        uint32_t flagsValue = pData->flags;
        if (ImGui::InputScalar("Flags", ImGuiDataType_U32, &flagsValue))
        {
            pData->flags = flagsValue;
            pData->dirty = true;
        }
    }

    {
        float sortZ = pData->sortZ;
        if (ImGui::DragFloat("SortZ", &sortZ, 0.001f, 0.f, 1.f, "%.3f"))
        {
            if (sortZ < 0.f) sortZ = 0.f;
            if (sortZ > 1.f) sortZ = 1.f;
            pData->sortZ = sortZ;
            pData->dirty = true;
        }
    }

    {
        float clip[4] =
        {
            pData->rcClip.fLeft,
            pData->rcClip.fTop,
            pData->rcClip.fRight,
            pData->rcClip.fBottom
        };

        if (ImGui::DragFloat4("ClipRect (L,T,R,B)", clip, 1.f))
        {
            pData->rcClip = { clip[0], clip[1], clip[2], clip[3] };

            if (pData->rcClip.fLeft > pData->rcClip.fRight)
                std::swap(pData->rcClip.fLeft, pData->rcClip.fRight);

            if (pData->rcClip.fTop > pData->rcClip.fBottom)
                std::swap(pData->rcClip.fTop, pData->rcClip.fBottom);

            pData->dirty = true;
        }
    }

    {
        bool dirty = (pData->dirty != 0);
        if (ImGui::Checkbox("Dirty", &dirty))
        {
            pData->dirty = dirty ? 1 : 0;
        }
    }

    if (pData->bEnable == 0)
        ImGui::EndDisabled();

    ImGui::TreePop();
}

void CInspectorPanel::Draw_AnimatorBlendingView()
{
    CAnimator animator = m_pTarget->Get_Component<CAnimator>();
    if (!animator.Is_Valid())
        return;

    ANIMATOR_DATA* pData = animator._Data();
    if (!pData)
        return;

    CMeshRenderer meshRenderer = m_pTarget->Get_Component<CMeshRenderer>();
    if (!meshRenderer.Is_Valid())
        return;

    MESH_RENDERER_DATA* pMrData = meshRenderer._Data();
    if (!pMrData)
        return;

    if (pMrData->hMesh == INVALID_HANDLE_UINT)
        return;

    if (!SYS_RESOURCE.Is_ModelHandle(pMrData->hMesh))
        return;

    MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(pMrData->hMesh);
    if (!pModel)
        return;

    const auto& vecAnimClips = pModel->vecAnimClips;

    if (ImGui::BeginPopup("Animator Blend Editor", ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (vecAnimClips.empty())
        {
            ImGui::TextDisabled("No animation clips.");
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
            return;
        }

        static int32_t s_iFromClip = 0;
        static int32_t s_iToClip = 0;
        static _float s_fBlendDuration = 0.2f;

        if (s_iFromClip >= static_cast<int32_t>(vecAnimClips.size()))
            s_iFromClip = 0;
        if (s_iToClip >= static_cast<int32_t>(vecAnimClips.size()))
            s_iToClip = 0;

        ImGui::Text("Blend transition setting");
        ImGui::Separator();

        if (ImGui::BeginCombo("From Clip", vecAnimClips[s_iFromClip].strName.c_str()))
        {
            for (int32_t i = 0; i < static_cast<int32_t>(vecAnimClips.size()); ++i)
            {
                const bool bSelected = (s_iFromClip == i);
                if (ImGui::Selectable(vecAnimClips[i].strName.c_str(), bSelected))
                    s_iFromClip = i;

                if (bSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (ImGui::BeginCombo("To Clip", vecAnimClips[s_iToClip].strName.c_str()))
        {
            for (int32_t i = 0; i < static_cast<int32_t>(vecAnimClips.size()); ++i)
            {
                const bool bSelected = (s_iToClip == i);
                if (ImGui::Selectable(vecAnimClips[i].strName.c_str(), bSelected))
                    s_iToClip = i;

                if (bSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::DragFloat("Blend Duration", &s_fBlendDuration, 0.005f, 0.f, 5.f, "%.3f sec");

        const bool bCanSave =
            s_iFromClip >= 0 &&
            s_iToClip >= 0 &&
            s_iFromClip < static_cast<int32_t>(vecAnimClips.size()) &&
            s_iToClip < static_cast<int32_t>(vecAnimClips.size()) &&
            s_iFromClip != s_iToClip;

        ImGui::BeginDisabled(!bCanSave);
        if (ImGui::Button("Save"))
        {
            const uint64_t iBlendKey = pData->pAnimator_Processor->Make_AnimationClipBlendKey(
                static_cast<uint32_t>(s_iFromClip),
                static_cast<uint32_t>(s_iToClip));

            _float fBlendDuration = std::fmaxf(0.f, s_fBlendDuration);

            if (fBlendDuration > pModel->vecAnimClips[s_iToClip].fDuration)
                fBlendDuration = pModel->vecAnimClips[s_iToClip].fDuration;

            pData->BlendMap[iBlendKey] = fBlendDuration;
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Saved transitions");

        std::vector<uint64_t> vecRemoveKeys;

        for (const auto& pair : pData->BlendMap)
        {
            const uint64_t iKey = pair.first;
            const _float fDuration = pair.second;

            const uint32_t iFromClip = static_cast<uint32_t>(iKey >> 32);
            const uint32_t iToClip = static_cast<uint32_t>(iKey & 0xffffffffu);

            const char* pFromName = (iFromClip < vecAnimClips.size()) ? vecAnimClips[iFromClip].strName.c_str() : "<Invalid>";
            const char* pToName = (iToClip < vecAnimClips.size()) ? vecAnimClips[iToClip].strName.c_str() : "<Invalid>";

            ImGui::PushID(static_cast<int>(iFromClip ^ (iToClip << 16)));

            ImGui::Text("%s -> %s : %.3f sec", pFromName, pToName, fDuration);
            ImGui::SameLine();

            if (ImGui::SmallButton("Delete"))
                vecRemoveKeys.push_back(iKey);

            ImGui::PopID();
        }

        for (uint64_t iKey : vecRemoveKeys)
            pData->BlendMap.erase(iKey);

        ImGui::EndPopup();
    }
}

void CInspectorPanel::Draw_AnimatorLoopView()
{
    CAnimator animator = m_pTarget->Get_Component<CAnimator>();
    if (!animator.Is_Valid())
        return;

    ANIMATOR_DATA* pData = animator._Data();
    if (!pData)
        return;

    CMeshRenderer meshRenderer = m_pTarget->Get_Component<CMeshRenderer>();
    if (!meshRenderer.Is_Valid())
        return;

    MESH_RENDERER_DATA* pMrData = meshRenderer._Data();
    if (!pMrData)
        return;

    if (pMrData->hMesh == INVALID_HANDLE_UINT)
        return;

    if (!SYS_RESOURCE.Is_ModelHandle(pMrData->hMesh))
        return;

    MODEL_ENTRY* pModel = SYS_RESOURCE.Get_Model(pMrData->hMesh);
    if (!pModel)
        return;

    const auto& vecAnimClips = pModel->vecAnimClips;

    if (ImGui::BeginPopup("Animator Loop Editor", ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (vecAnimClips.empty())
        {
            ImGui::TextDisabled("No animation clips.");
            if (ImGui::Button("Close"))
                ImGui::CloseCurrentPopup();

            ImGui::EndPopup();
            return;
        }

        ImGui::Text("Loop setting per clip");
        ImGui::Separator();

        for (uint32_t i = 0; i < static_cast<uint32_t>(vecAnimClips.size()); ++i)
        {
            const std::string& strClipName = vecAnimClips[i].strName;

            _bool bLoop = false;
            auto it = pData->LoopMap.find(i);
            if (it != pData->LoopMap.end())
                bLoop = it->second;

            ImGui::PushID(static_cast<int>(i));

            bool bLoopImGui = (bLoop != false);
            if (ImGui::Checkbox(strClipName.c_str(), &bLoopImGui))
            {
                pData->LoopMap[i] = bLoopImGui;
            }

            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::Separator();

        if (ImGui::Button("All Loop On"))
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(vecAnimClips.size()); ++i)
                pData->LoopMap[i] = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("All Loop Off"))
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(vecAnimClips.size()); ++i)
                pData->LoopMap[i] = false;
        }

        ImGui::SameLine();

        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }
}

std::unique_ptr<CInspectorPanel> CInspectorPanel::Create(const std::string& strPanelName, CHierarchyPanel* pHierarcy, CProjectPanel* pProject)
{
    auto pInstance = std::make_unique<CInspectorPanel>(strPanelName);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(pHierarcy, pProject), nullptr, "CInspectorPanel Create failed");
    return pInstance;
}

NS_END
