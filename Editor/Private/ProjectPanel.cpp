#include "ProjectPanel.h"

#include "Engine_Log.h"
#include "Editor_Util.h"
#include "Asset_Registry.h"
#include "Core_System.h"

NS_BEGIN(Editor)

CProjectPanel::CProjectPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CProjectPanel::~CProjectPanel()
{
}

ASSET_SELECTION CProjectPanel::Build_Selection(const std::filesystem::path& p) const
{
    ASSET_SELECTION sel{};
    sel.path = p;

    if (p.empty())
        return sel;

    std::error_code ec;
    sel.isDirectory = std::filesystem::is_directory(p, ec);
    sel.type = Engine::CAsset_Registry::Detect_Type(p, sel.isDirectory);
    return sel;
}

void CProjectPanel::Notify_Selection_Changed()
{
    ASSET_SELECTION sel = Build_Selection(m_selectedPath);

    m_OnSelectionChanged.Invoke(sel);
}

HRESULT CProjectPanel::Initialize()
{
    m_assetsRoot = Engine::ProjectConfig::PATH + Engine::ProjectConfig::ROOT;

    Ensure_Path_Exists(m_assetsRoot);

    m_currentFolder = m_assetsRoot;

    m_bTreeDirty = true;
    m_bListDirty = true;

    return S_OK;
}

void CProjectPanel::Update()
{
    if (m_bTreeDirty)
        Refresh_Folder_Tree();

    if (m_bListDirty)
        Refresh_File_List();
}

void CProjectPanel::Render()
{
    if (!ImGui::Begin(m_strPanelName.c_str()))
    {
        ImGui::End();
        return;
    }

    Draw_Toolbar();
    Draw_Search_Bar();
    Draw_Main_Split();
    Draw_Context_Menu();

    ImGui::End();
}

/* =======================================================================*/
/* ============================== Selection ==============================*/
/* =======================================================================*/
const std::filesystem::path& CProjectPanel::Get_Selected_Path() const
{
    return m_selectedPath;
}

_bool CProjectPanel::Has_Selection() const
{
    return !m_selectedPath.empty();
}

void CProjectPanel::Set_Selection(const std::filesystem::path& path)
{
    m_selectedPath = path;
}

void CProjectPanel::Clear_Selection()
{
    m_selectedPath.clear();
}

/* =======================================================================*/
/* ============================== Rename =================================*/
/* =======================================================================*/
void CProjectPanel::Begin_Rename(const std::filesystem::path& targetPath)
{
    if (targetPath.empty())
        return;

    m_renameTargetPath = targetPath;
    m_renameBuffer = Editor_Util::To_UTF8(targetPath.filename());
    m_bJustStartedRename = true;
}

void CProjectPanel::Cancel_Rename()
{
    m_renameTargetPath.clear();
    m_renameBuffer.clear();
    m_bJustStartedRename = false;
}

_bool CProjectPanel::Is_Renaming() const
{
    return !m_renameTargetPath.empty();
}

/* =======================================================================*/
/* ============================== UI State ===============================*/
/* =======================================================================*/
void CProjectPanel::Draw_Toolbar()
{
    /* Minimal toolbar, expand later */
    ImGui::BeginChild("##ProjectToolbar", ImVec2(0.f, 32.f), false, ImGuiWindowFlags_NoScrollbar);

    if (ImGui::Button("Refresh"))
    {
        m_bTreeDirty = true;
        m_bListDirty = true;
    }

    ImGui::SameLine();

    ImGui::TextUnformatted("Current:");
    ImGui::SameLine();

    std::string cur = Editor_Util::To_UTF8(m_currentFolder);
    ImGui::TextUnformatted(cur.c_str());

    ImGui::EndChild();
}

void CProjectPanel::Draw_Search_Bar()
{
    ImGui::BeginChild("##ProjectSearch", ImVec2(0.f, 34.f), false, ImGuiWindowFlags_NoScrollbar);

    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##ProjectSearchInput", "Search...", &m_search);

    ImGui::EndChild();
}

void CProjectPanel::Draw_Main_Split()
{
    /* left tree + right list */
    ImGui::BeginChild("##ProjectMain", ImVec2(0.f, 0.f), false);

    const _float fLeftWidth = 280.f;

    ImGui::BeginChild("##ProjectFolderTree", ImVec2(fLeftWidth, 0.f), true);
    Draw_Folder_Tree();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("##ProjectFileList", ImVec2(0.f, 0.f), true);
    Draw_File_List();
    ImGui::EndChild();

    ImGui::EndChild();
}

/* =======================================================================*/
/* ========================= Left : FOLDER tree ==========================*/
/* =======================================================================*/
void CProjectPanel::Draw_Folder_Tree()
{
    if (!std::filesystem::exists(m_assetsRoot))
    {
        ImGui::TextUnformatted("Assets root not found.");
        return;
    }

    Draw_Folder_Node_Recursive(m_rootNode, 0);
}

void CProjectPanel::Draw_Folder_Node_Recursive(const FOLDER_NODE& tNode, _int iDepth)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;

    const _bool bRoot = (tNode.path == m_assetsRoot);
    const _bool bHasChildren = !tNode.children.empty();

    if (!bHasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (tNode.path == m_currentFolder)
        flags |= ImGuiTreeNodeFlags_Selected;

    _bool bOpen = false;

    /* TreeNodeEX returns the node is currently open. This controls to draw child elements. */
    if (bRoot)
    {
        /* Root: always open by default */
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
        bOpen = ImGui::TreeNodeEx(Engine::ProjectConfig::ROOT.c_str(), flags);
    }
    else
    {
        /* Restore and maintain the previous open state */
        _bool bToOpen = Is_Folder_Open(tNode.path);
        if (bToOpen)
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);

        bOpen = ImGui::TreeNodeEx(tNode.name.c_str(), flags);
    }

    /* Click -> select folder */
    /* Identify selection vs toggling */
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    {
        /* If the click didn't toggle the open state, treat it as a folder selection. */
        m_currentFolder = tNode.path;
        m_bListDirty = true;

        Set_Selection(tNode.path);
        Notify_Selection_Changed();
    }

    if (!bRoot && ImGui::IsItemToggledOpen())
    {
        Set_Folder_Open(tNode.path, bOpen);
    }

    /* If bOpen is true, the node is expanded; draw children recursively */
    if (bOpen)
    {
        for (const auto& c : tNode.children)
            Draw_Folder_Node_Recursive(c, iDepth + 1);

        ImGui::TreePop();
    }
}

/* =======================================================================*/
/* ========================= Right : File list ===========================*/
/* =======================================================================*/
void CProjectPanel::Draw_File_List()
{
    if (!std::filesystem::exists(m_currentFolder))
    {
        ImGui::TextUnformatted("FOLDER not found.");
        return;
    }

    /* Blank-space context (file list only) */
    if (ImGui::BeginPopupContextWindow("##FileListBlankContext",
        ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
    {
        if (ImGui::MenuItem("Create Folder"))
        {
            std::filesystem::path created;
            if (Create_Folder(m_currentFolder, created))
            {
                m_bListDirty = true;
                m_bTreeDirty = true;

                m_pendingRenamePath = created;

                Set_Selection(created);
                Notify_Selection_Changed();
            }
        }

        if (ImGui::MenuItem("Refresh"))
        {
            m_bTreeDirty = true;
            m_bListDirty = true;
        }

        if (ImGui::MenuItem("Show Current in Explorer"))
            Show_In_Explorer(m_currentFolder);

        ImGui::EndPopup();
    }

    /* Display file(asset) list */
    for (const auto& item : m_Assets)
    {
        if (!m_search.empty() && !Is_Visible_By_Filter(item.name, m_search))
            continue;

        Draw_File_Asset_Row(item);
    }

    /* Rename after create folder (outside popup) */
    if (!m_pendingRenamePath.empty())
    {
        Begin_Rename(m_pendingRenamePath);
        m_pendingRenamePath.clear();
    }
}

void CProjectPanel::Draw_File_Asset_Row(const LIST_ASSET& tAsset)
{
    /* Hide meta file */
    if (Engine::Is_MetaFile(tAsset.path))
        return;

    ImGui::PushID(Editor_Util::To_UTF8(tAsset.path).c_str());

    const _bool bSelected = (!m_selectedPath.empty() &&
        std::filesystem::exists(m_selectedPath) && 
        std::filesystem::exists(tAsset.path) &&    
        std::filesystem::equivalent(m_selectedPath, tAsset.path));

    /* row label */
    const _char* szAssetType = Engine::CAsset_Registry::AssetType_ToStr(tAsset.type);

    /* Display asset type */
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::Text("[%s]", szAssetType);
    ImGui::PopStyleColor();

    ImGui::SameLine(90.f);


    /* Rename inline */
    if (Is_Renaming() && m_renameTargetPath == tAsset.path)
    {
        Draw_Rename_Field(tAsset);
        ImGui::EndGroup();
        ImGui::PopID();
        return;
    }

    /* Click row */
    if (ImGui::Selectable(tAsset.name.c_str(), bSelected,
        ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_SpanAllColumns))
    {
        const _bool bChanged =
            (m_selectedPath.empty() || !std::filesystem::equivalent(m_selectedPath, tAsset.path));

        Set_Selection(tAsset.path);

        if (bChanged)
            Notify_Selection_Changed();

        if (tAsset.isDirectory && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            m_currentFolder = tAsset.path;
            m_bListDirty = true;
        }

        if (!tAsset.isDirectory &&
            ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            Try_Open_Scene_On_DoubleClick(tAsset);
        }
    }

    if (!tAsset.isDirectory)
    {
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            Engine::ASSET_GUID g{};
            if (SYS_ASSET.Try_Get_GUID(tAsset.path, g))
            {
                ImGui::SetDragDropPayload("ASSET_GUID", &g, sizeof(g));
                ImGui::Text("Asset: %s", tAsset.name.c_str());
            }
            else
            {
                ImGui::Text("No GUID");
            }

            ImGui::EndDragDropSource();
        }
    }

    ImGui::EndGroup();

    /* context */
    std::string popupId = "##Context" + Editor_Util::To_UTF8(tAsset.path);
    if (ImGui::BeginPopupContextItem(popupId.c_str(), ImGuiPopupFlags_MouseButtonRight))
    {
        m_contextTargetPath = tAsset.path;

        if (ImGui::MenuItem("Rename", "F2"))
            Begin_Rename(m_contextTargetPath);

        if (ImGui::MenuItem("Delete", "Del"))
        {
            Delete_Path(m_contextTargetPath);
            m_bTreeDirty = true;
            m_bListDirty = true;
            if (!m_selectedPath.empty() && (m_selectedPath == m_contextTargetPath))
                Clear_Selection();
        }

        if (ImGui::MenuItem("Show in Explorer"))
            Show_In_Explorer(m_contextTargetPath);

        ImGui::EndPopup();
    }
    ImGui::PopID();
}

/* =======================================================================*/
/* ======================= Context menu(blank space) =====================*/
/* =======================================================================*/
void CProjectPanel::Draw_Context_Menu()
{
    /* blank space context */
    if (ImGui::BeginPopupContextWindow("##ProjectBlankContext", ImGuiPopupFlags_MouseButtonRight))
    {
        m_contextTargetPath.clear();

        if (ImGui::MenuItem("Refresh"))
        {
            m_bTreeDirty = true;
            m_bListDirty = true;
        }

        if (ImGui::MenuItem("Show Current in Explorer"))
        {
            Show_In_Explorer(m_currentFolder);
        }

        ImGui::EndPopup();
    }

    /* shortcuts */
    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
    {
        if (ImGui::IsKeyPressed(ImGuiKey_F2))
        {
            if (Has_Selection())
                Begin_Rename(m_selectedPath);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            if (Has_Selection())
            {
                Delete_Path(m_selectedPath);
                Clear_Selection();
                m_bTreeDirty = true;
                m_bListDirty = true;
            }
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            if (Is_Renaming())
                Cancel_Rename();
        }
    }
}

_bool CProjectPanel::Create_Folder(const std::filesystem::path& parentFolder, std::filesystem::path& outCreatedPath)
{
    outCreatedPath.clear();

    if (parentFolder.empty() || !std::filesystem::exists(parentFolder))
        return false;

    std::error_code ec;

    const std::string folderName = Make_Unique_Folder_Name_Impl(parentFolder, "New Folder");
    std::filesystem::path newPath = parentFolder / folderName;

    std::filesystem::create_directory(newPath, ec);
    if (ec)
    {
        _DEBUG_ERROR("Create Folder Failed: %s", ec.message().c_str());
        return false;
    }

    outCreatedPath = newPath;
    return true;
}

/* =======================================================================*/
/* ============================== Refresh ================================*/
/* =======================================================================*/
void CProjectPanel::Refresh_Folder_Tree()
{
    m_rootNode = FOLDER_NODE{};
    m_rootNode.path = m_assetsRoot;
    m_rootNode.name = Engine::ProjectConfig::ROOT;

    std::function<void(FOLDER_NODE&)> fnBuild = [&](FOLDER_NODE& n)
        {
            n.children.clear();

            for (auto& entry : std::filesystem::directory_iterator(n.path))
            {
                if (!entry.is_directory())
                    continue;

                FOLDER_NODE child;
                child.path = entry.path();
                child.name = Editor_Util::To_UTF8(entry.path().filename());
                fnBuild(child);
                n.children.push_back(std::move(child));
            }
        };

    fnBuild(m_rootNode);

    m_bTreeDirty = false;
}

void CProjectPanel::Refresh_File_List()
{
    m_Assets.clear();

    if (!std::filesystem::exists(m_currentFolder))
    {
        m_bListDirty = false;
        return;
    }

    /* folders first */
    for (auto& entry : std::filesystem::directory_iterator(m_currentFolder))
    {
        if (!entry.is_directory())
            continue;

        LIST_ASSET it;
        it.path = entry.path();
        it.name = Editor_Util::To_UTF8(entry.path().filename());
        it.isDirectory = true;
        it.type = SYS_ASSET.Detect_Type(it.path, true);
        m_Assets.push_back(std::move(it));
    }

    /* then files */
    for (auto& entry : std::filesystem::directory_iterator(m_currentFolder))
    {
        if (!entry.is_regular_file())
            continue;

        LIST_ASSET it;
        it.path = entry.path();
        it.name = Editor_Util::To_UTF8(entry.path().filename());
        it.isDirectory = false;
        it.type = SYS_ASSET.Detect_Type(it.path, false);
        m_Assets.push_back(std::move(it));
    }

    m_bListDirty = false;
}

/* =======================================================================*/
/* ============================== Helpers ================================*/
/* =======================================================================*/
_bool CProjectPanel::Is_Visible_By_Filter(const std::string& name, const std::string& filter)
{
    return String_IContains(name, filter);
}

_bool CProjectPanel::Is_Folder_Open(const std::filesystem::path& path) const
{
    return m_openFolders.find(Get_Stable_Id_From_Path(path)) != m_openFolders.end();
}

void CProjectPanel::Set_Folder_Open(const std::filesystem::path& path, bool bOpen)
{
    uint64_t id = Get_Stable_Id_From_Path(path);
    if (bOpen)
        m_openFolders.insert(id);
    else
        m_openFolders.erase(id);
}

void CProjectPanel::Ensure_Path_Exists(std::filesystem::path& inoutPath)
{
    /* Ensures the path exists by creating missing directories, then normalizes it into a stable, weakly-canonical form. */
    if (!std::filesystem::exists(inoutPath))
    {
        std::error_code ec;
        std::filesystem::create_directories(inoutPath, ec);
    }

    inoutPath = std::filesystem::weakly_canonical(inoutPath);
}

/* =======================================================================*/
/* ============================= Operations ==============================*/
/* =======================================================================*/
_bool CProjectPanel::Rename_Path(const std::filesystem::path& src, const std::string& newName)
{
    if (src.empty() || newName.empty())
        return false;

    std::filesystem::path dst = src.parent_path() / std::filesystem::path(newName);

    /* preserve extension if user didn't type it (not a directory) */
    if (!std::filesystem::is_directory(src))
    {
        if (src.has_extension() && !dst.has_extension())
            dst.replace_extension(src.extension());
    }

    if (src == dst)
        return true;

    if (std::filesystem::exists(dst))
        return false;

    std::error_code ec;
    std::filesystem::rename(src, dst, ec);
    if (ec)
        return false;

    /* if selection renamed */
    if (!m_selectedPath.empty() && m_selectedPath == src)
        m_selectedPath = dst;

    /* if current folder renamed (edge) */
    if (m_currentFolder == src)
        m_currentFolder = dst;

    /* Handle meta file */
    if (!Engine::Is_MetaFile(src))
    {
        auto oldMeta = Engine::Make_MetaPath(src);
        auto newMeta = Engine::Make_MetaPath(dst);

        if (std::filesystem::exists(oldMeta))
        {
            ec.clear();
            std::filesystem::rename(oldMeta, newMeta, ec);
            if (ec)
                _DEBUG_INFO_BREAK("Failed to rename meta: %s", oldMeta.string().c_str());
        }
    }

    return true;
}

_bool CProjectPanel::Delete_Path(const std::filesystem::path& target)
{
    if (target.empty() || !std::filesystem::exists(target))
        return false;

    std::error_code ec;
    _bool bSuccess = false;
    const _bool bIsDir = std::filesystem::is_directory(target);

    if (std::filesystem::is_directory(target))
        bSuccess = (std::filesystem::remove_all(target, ec) > 0);
    else
        bSuccess = std::filesystem::remove(target, ec);

    if (!(bSuccess && !ec))
        return false;

    /* Handle meta file */
    if (!Engine::Is_MetaFile(target))
    {
        const auto metaPath = Engine::Make_MetaPath(target);
        if (std::filesystem::exists(metaPath))
        {
            ec.clear();
            std::filesystem::remove(metaPath, ec);
            if (ec)
                _DEBUG_INFO_BREAK("Failed to delete meta: %s", metaPath.string().c_str());
        }
    }

    if (m_currentFolder == target || Is_Subpath(m_currentFolder, target))
    {
        m_currentFolder = m_assetsRoot;
        m_bListDirty = true;
    }

    return true;
}

void CProjectPanel::Show_In_Explorer(const std::filesystem::path& target)
{
    if (target.empty())
        return;

    std::filesystem::path p = target;

    if (!std::filesystem::exists(p))
        return;

    /* explorer /select, */
    std::wstring args = L"/select,\"";
    args += p.wstring();
    args += L"\"";

    ShellExecuteW(nullptr, L"open", L"explorer.exe", args.c_str(), nullptr, SW_SHOWNORMAL);
}

/* =======================================================================*/
/* ============================= Rename UI ===============================*/
/* =======================================================================*/
void CProjectPanel::Draw_Rename_Field(const LIST_ASSET& item)
{
    ImGui::SetNextItemWidth(-1);

    if (m_bJustStartedRename)
    {
        ImGui::SetKeyboardFocusHere();
        m_bJustStartedRename = false;
    }

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue;

    if (ImGui::InputText("##Rename", &m_renameBuffer, flags))
    {
        Commit_Rename();
        return;
    }

    if (ImGui::IsItemDeactivated())
    {
        if (!ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            Commit_Rename();
            return;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        Cancel_Rename();
        return;
    }
}

void CProjectPanel::Commit_Rename()
{
    if (m_renameTargetPath.empty()) return;

    // 기존 헬퍼 함수 호출 (여기 안에 선택 경로 갱신 로직 등이 이미 일부 포함되어 있음)
    if (Rename_Path(m_renameTargetPath, m_renameBuffer))
    {
        m_bListDirty = true;
        m_bTreeDirty = true;
    }

    Cancel_Rename();
}

// ProjectPanel.cpp

_bool CProjectPanel::Is_Scene_Asset(const LIST_ASSET& tAsset) const
{
    if (tAsset.isDirectory)
        return false;

    return tAsset.type == Engine::ASSET_TYPE::SCENE;
}

_bool CProjectPanel::Try_Get_Asset_GUID(const std::filesystem::path& path, Engine::ASSET_GUID& outGuid) const
{
    outGuid = Engine::ASSET_GUID{};
    if (path.empty() || !std::filesystem::exists(path))
        return false;

    return SYS_ASSET.Try_Get_GUID(path, outGuid);
}

void CProjectPanel::Open_Scene_By_GUID(const Engine::ASSET_GUID& guid, SCENE_CHANGE_MODE eMode)
{
    SYS_CORE.Change_Scene(guid, eMode);
}

void CProjectPanel::Try_Open_Scene_On_DoubleClick(const LIST_ASSET& tAsset)
{
    if (!Is_Scene_Asset(tAsset))
        return;

    if (!ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        return;

    Engine::ASSET_GUID guid{};
    if (!Try_Get_Asset_GUID(tAsset.path, guid))
    {
        _DEBUG_WARN("Scene double click but GUID not found: %s", tAsset.path.string().c_str());
        return;
    }

    /* TODO : 에디터의 게임 모드에서는 ??  */
    Open_Scene_By_GUID(guid, SCENE_CHANGE_MODE::EDITOR_EDIT);
}


/* =======================================================================*/
/* ============================== Utility ================================*/
/* =======================================================================*/
uint64_t CProjectPanel::Get_Stable_Id_From_Path(const std::filesystem::path& p)
{
    /* stable enough for editor session; for real GUID, add meta later */
    return std::hash<std::wstring>{}(std::filesystem::weakly_canonical(p).wstring());
}

_bool CProjectPanel::String_IContains(const std::string& haystack, const std::string& needle)
{
    if (needle.empty())
        return true;

    std::string a = haystack;
    std::string b = needle;

    for (auto& c : a) c = (char)tolower(c);
    for (auto& c : b) c = (char)tolower(c);

    return (a.find(b) != std::string::npos);
}

_bool CProjectPanel::Is_Subpath(const std::filesystem::path& path, const std::filesystem::path& base)
{
    auto relativeness = std::ranges::mismatch(base, path);
    return relativeness.in1 == base.end();
}

std::string CProjectPanel::Make_Unique_Folder_Name_Impl(const std::filesystem::path& parent,
    const std::string& baseName)
{
    // "New Folder", "New Folder (1)" ...
    std::string name = baseName;
    std::filesystem::path candidate = parent / name;

    if (!std::filesystem::exists(candidate))
        return name;

    for (int i = 1; i < 9999; ++i)
    {
        name = baseName + " (" + std::to_string(i) + ")";
        candidate = parent / name;
        if (!std::filesystem::exists(candidate))
            return name;
    }

    // fallback
    return baseName + " (9999)";
}



/* =======================================================================*/
/* ============================ Factory/Free =============================*/
/* =======================================================================*/
std::unique_ptr<CProjectPanel> CProjectPanel::Create(const std::string& strPanelName)
{
    auto pInstance = std::make_unique<CProjectPanel>(strPanelName);

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "CProjectPanel Create failed");
    return pInstance;
}

NS_END
