#include "InspectorPanel.h"
#include "HierarchyPanel.h"
#include "GameObject.h"
#include "Engine_Log.h"
#include "Asset_Registry.h"

NS_BEGIN(Editor)

CInspectorPanel::CInspectorPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
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

    /* Transform is not implemented yet */
    _float vPos[3] = { 0,0,0 };
    _float vRot[3] = { 0,0,0 };
    _float vScl[3] = { 1,1,1 };

    /* Need to call to read Transform data */
    _bool bChanged = false;

    ImGui::TextUnformatted("Position");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Pos", vPos, 0.1f);

    ImGui::TextUnformatted("Rotation");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Rot", vRot, 0.1f);

    ImGui::TextUnformatted("Scale");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    bChanged |= ImGui::DragFloat3("##Scl", vScl, 0.01f);

    if (bChanged)
    {
        /* Write back to engine Transform */
    }
}

void CInspectorPanel::Draw_Components()
{
    if (!ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    ImGui::TextDisabled("TODO: Draw engine components here.");

    if (ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup("##AddComponentPopup");
    }

    if (ImGui::BeginPopup("##AddComponentPopup"))
    {
        ImGui::TextUnformatted("Not implemented.");
        ImGui::EndPopup();
    }
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

CInspectorPanel* CInspectorPanel::Create(const std::string& strPanelName, CHierarchyPanel* pHierarcy, CProjectPanel* pProject)
{
    CInspectorPanel* pInstance = new CInspectorPanel(strPanelName);
    if (FAILED(pInstance->Initialize(pHierarcy, pProject)))
    {
        Safe_Release(pInstance);
        _DEBUG_ERROR_BREAK("CInspectorPanel Create failed");
    }
    return pInstance;
}

void CInspectorPanel::Free()
{
    CEditorPanel::Free();
}
NS_END
