#include "InspectorPanel.h"
#include "HierarchyPanel.h"
#include "GameObject.h"
#include "Engine_Log.h"
#include "Asset_Registry.h"

#include "Transform.h"
#include "MeshRenderer.h"
#include "Material.h"
#include "Shader.h"

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

    _DEBUG_INFO("Changed target");

    m_bJustStartedNameEdit = false;

    m_selectedAsset = ASSET_SELECTION{};
    m_eMode = (pObj ? InspectMode::GameObject : InspectMode::None);
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

        Draw_Basic_Info();
        ImGui::Separator();

        Draw_Transform();
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
    _bool bActive = m_pTarget->Get_Active();
    if (ImGui::Checkbox("Active", &bActive))
        m_pTarget->Set_Active(bActive);

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
}

void CInspectorPanel::Draw_Transform()
{
    if (!ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    CTransform transform = m_pTarget->Get_Component<CTransform>(COMPONENT_TYPE::TRANSFORM);
    TRANSFORM_DATA* pData = transform._Data();

    /* Transform is not implemented yet */
    _float vPos[3] = { pData->vPosition.x, pData->vPosition.y, pData->vPosition.z };
    _float vScl[3] = { pData->vScale.x, pData->vScale.y, pData->vScale.z };
    _float3 f3Rot = transform.Get_Rotation_Euler();

    _float vRot[3] = { f3Rot.x, f3Rot.y, f3Rot.z };

    /* Need to call to read Transform data */
    _bool bChanged = false;

    ImGui::TextUnformatted("Position ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Pos", vPos, 0.1f);

    ImGui::TextUnformatted("Rotation");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Rot", vRot, 0.1f);

    ImGui::TextUnformatted("Scale     ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Scl", vScl, 0.01f);

    if (bChanged)
    {
        pData->vPosition = { vPos[0], vPos[1], vPos[2] };
        pData->vScale = { vScl[0], vScl[1], vScl[2] };
        _float3 newEulerDeg{ vRot[0], vRot[1], vRot[2] };
        transform.Set_Rotation_Euler(newEulerDeg);

        pData->bDirty = true;
    }
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
}

void CInspectorPanel::Draw_CurrentComponents()
{
    Component::COMPONENT_MASK mask = m_pTarget->Get_ComponentMask();

    for (uint32_t i = 0; i < SCAST(uint32_t, COMPONENT_MAX); ++i)
    {
        if ((mask & Component::Component_Bit((COMPONENT_TYPE)i)) == 0)
            continue;

        COMPONENT_TYPE eType = (COMPONENT_TYPE)i;
        Draw_ComponentByType(eType);
    }

}

void CInspectorPanel::Draw_ComponentByType(COMPONENT_TYPE eComType)
{
    if (eComType == COMPONENT_TYPE::MESH_RENDERER)
    {
        CMeshRenderer mr = m_pTarget->Get_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
        if (!mr.Is_Valid())
            return;

        MESH_RENDERER_DATA* pData = mr._Data();
        if (!pData)
            return;

        if (!ImGui::TreeNodeEx("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
            return;

        bool bChanged = false;

        // --- Mesh ---
        ImGui::TextUnformatted("Mesh");
        ImGui::SameLine();
        ImGui::Text("%u", pData->hMesh);

        // --- Material ---
        ImGui::TextUnformatted("Material");
        ImGui::SameLine();
        ImGui::Text("%u", pData->hMaterial);

        // --- Pass ---
        //uint16_t pass = pData->;
        //if (ImGui::DragScalar("Pass", ImGuiDataType_U16, &pass, 1.f))
        //{
        //    pData->passIndex = pass;
        //    bChanged = true;
        //}

        // --- Layer ---
        int layer = (int)pData->layer;
        if (ImGui::DragInt("Layer", &layer, 1, 0, 10))
        {
            pData->layer = (RENDER_LAYER)layer;
            bChanged = true;
        }

        // --- Flags ---
        uint32_t flags = pData->flags;
        if (ImGui::InputScalar("Flags", ImGuiDataType_U32, &flags))
        {
            pData->flags = flags;
            bChanged = true;
        }

        //if (bChanged)
        //{
        //    pData->bDirty = true;
        //}

        ImGui::TreePop();
    }
}

void CInspectorPanel::Draw_AddComponentPopup()
{
    if (!ImGui::BeginPopup("##AddComponentPopup"))
        return;

    if (ImGui::MenuItem("Transform"))
    {
        m_pTarget->Add_Component<CTransform>(COMPONENT_TYPE::TRANSFORM);
        ImGui::CloseCurrentPopup();
    }
    if (ImGui::MenuItem("MeshRenderer"))
    {
        m_pTarget->Add_Component<CMeshRenderer>(COMPONENT_TYPE::MESH_RENDERER);
        ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
}

void CInspectorPanel::Draw_Asset()
{
    // 리소스 로드 X: 메타만 표시
    const std::string name = m_selectedAsset.path.filename().string();
    const std::string path = m_selectedAsset.path.string();

    ImGui::TextUnformatted("Asset");
    ImGui::Separator();

    ImGui::Text("Name: %s", name.c_str());
    ImGui::Text("Type: %s", CAsset_Registry::AssetType_ToStr(m_selectedAsset.type)); // 공용 함수 만들면 좋음
    ImGui::Text("Path: %s", path.c_str());
    ImGui::Text("Directory: %s", m_selectedAsset.isDirectory ? "true" : "false");
}

void CInspectorPanel::Draw_None()
{
    ImGui::TextUnformatted("No selection.");
}

std::unique_ptr<CInspectorPanel> CInspectorPanel::Create(const std::string& strPanelName, CHierarchyPanel* pHierarcy, CProjectPanel* pProject)
{
    auto pInstance = std::make_unique<CInspectorPanel>(strPanelName);
    if (FAILED(pInstance->Initialize(pHierarcy, pProject)))
    {
        _DEBUG_ERROR_BREAK("CInspectorPanel Create failed");
        return nullptr;
    }
    return pInstance;
}

NS_END
