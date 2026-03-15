#include "ResourcePanel.h"

#include "Editor_Util.h"
#include "Resource_System.h"
#include "Asset_Registry.h"

NS_BEGIN(Editor)

CResourcePanel::CResourcePanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CResourcePanel::~CResourcePanel()
{
}

HRESULT CResourcePanel::Initialize()
{
    return S_OK;
}

void CResourcePanel::Update()
{
}

void CResourcePanel::Render()
{
    if (!ImGui::Begin(m_strPanelName.c_str()))
    {
        ImGui::End();
        return;
    }

    Draw_Search_Bar();
    ImGui::Separator();
    Draw_Result();

    ImGui::End();
}

void CResourcePanel::Draw_Search_Bar()
{
    ImGui::TextUnformatted("Type");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(140.f);
    ImGui::Combo("##ResourceType", &m_iTypeIndex, s_arrTypeNames, IM_ARRAYSIZE(s_arrTypeNames));

    ImGui::TextUnformatted("Handle");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(180.f);
    ImGui::InputScalar("##ResourceHandle", ImGuiDataType_U32, &m_iHandle);

    ImGui::TextUnformatted("Name Filter");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputText("##ResourceNameFilter", &m_searchName);
}

void CResourcePanel::Draw_Result()
{
    const Engine::ASSET_TYPE eType = Get_Selected_AssetType();
    const Engine::ASSET_GUID& tGUID = SYS_RESOURCE.Find_GUID_By_Handle(eType, m_iHandle);

    ImGui::Text("Selected Type : %s", Get_Selected_AssetType_Name());
    ImGui::Text("Handle        : %u", m_iHandle);

    if (!tGUID.Is_Valid())
    {
        ImGui::Spacing();
        ImGui::TextUnformatted("Invalid handle or resource not loaded.");
        return;
    }

    const std::string strGUID = tGUID.To_String_Utf8();
    const std::string& strName = SYS_RESOURCE.Find_Name_By_GUID(tGUID);
    const Engine::ASSET_RECORD* pRecord = SYS_ASSET.Find(tGUID);

    if (!m_searchName.empty() && !Editor_Util::Str_IContains(strName, m_searchName))
    {
        ImGui::Spacing();
        ImGui::TextUnformatted("Filtered out by name.");
        return;
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Resolved Asset");
    ImGui::Separator();

    ImGui::Text("GUID : %s", strGUID.c_str());
    ImGui::Text("Name : %s", strName.c_str());

    if (pRecord)
    {
        ImGui::Text("Type : %d", (_int)pRecord->eType);
        ImGui::TextWrapped("Path : %s", pRecord->path.string().c_str());
    }
    else
    {
        ImGui::TextUnformatted("Registry Record : <not found>");
    }
}

Engine::ASSET_TYPE CResourcePanel::Get_Selected_AssetType() const
{
    switch (m_iTypeIndex)
    {
    case 0: return Engine::ASSET_TYPE::MESH;
    case 1: return Engine::ASSET_TYPE::MODEL;
    case 2: return Engine::ASSET_TYPE::MATERIAL;
    case 3: return Engine::ASSET_TYPE::SHADER;
    case 4: return Engine::ASSET_TYPE::TEXTURE;
    default: return Engine::ASSET_TYPE::MESH;
    }
}

const char* CResourcePanel::Get_Selected_AssetType_Name() const
{
    if (m_iTypeIndex < 0 || m_iTypeIndex >= IM_ARRAYSIZE(s_arrTypeNames))
        return "<invalid>";

    return s_arrTypeNames[m_iTypeIndex];
}

std::unique_ptr<CResourcePanel> CResourcePanel::Create(const std::string& strPanelName)
{
    auto pInstance = std::make_unique<CResourcePanel>(strPanelName);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "CResourcePanel Create failed");
    return pInstance;
}

NS_END
