#include "ProjectPanel.h"

#include "Engine_Log.h"
#include "Editor_Util.h"
#include "Asset_Registry.h"
#include "Core_System.h"
#include "Asset_Meta.h"
#include "Create_Asset_Helper.h"
#include "Resource_System.h"

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

    Ensure_Path_Exists(m_assetsRoot);

    m_ScriptPath = m_assetsRoot / "Scripts";
    m_ScenePath = m_assetsRoot / "Scenes";
    m_PrototypePath = m_assetsRoot / "Prototypes";
    m_MaterialPath = m_assetsRoot / "Materials";
    m_ShaderPath = m_assetsRoot / "Shaders";

    m_HeaderPath = std::filesystem::path(Engine::ProjectConfig::CLIENT) / Engine::ProjectConfig::HEADER;
    m_ImplPath = std::filesystem::path(Engine::ProjectConfig::CLIENT) / Engine::ProjectConfig::IMPL;

    Ensure_Path_Exists(m_ScriptPath);
    Ensure_Path_Exists(m_ScenePath);
    Ensure_Path_Exists(m_PrototypePath);
    Ensure_Path_Exists(m_MaterialPath);
    Ensure_Path_Exists(m_ScriptPath);
    Ensure_Path_Exists(m_HeaderPath);
    Ensure_Path_Exists(m_ImplPath);

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
    Draw_Create_Script_Popup();
    Draw_Create_Material_Popup();
    Draw_Context_Popup();

    ImGui::End();
}

/* ----------------------------- Selection ----------------------------- */
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

/* ----------------------------- Rename ----------------------------- */
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

/* ----------------------------- UI State ----------------------------- */
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

/* ----------------------------- Left : FOLDER tree ----------------------------- */
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
    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        m_popupTargetPath = tNode.path;
        m_popupTargetIsItem = true;
    }

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

/* ----------------------------- Right : File list ----------------------------- */
void CProjectPanel::Draw_File_List()
{
    if (!std::filesystem::exists(m_currentFolder))
    {
        ImGui::TextUnformatted("FOLDER not found.");
        return;
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

    const _bool bSelected =
        (!m_selectedPath.empty() &&
            std::filesystem::exists(m_selectedPath) &&
            std::filesystem::exists(tAsset.path) &&
            (m_selectedPath == tAsset.path));

    /* row label */
    const _char* szAssetType = Engine::CAsset_Registry::AssetType_ToStr(tAsset.type);

    /* Display asset type */
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::Text("[%s]", szAssetType);
    ImGui::PopStyleColor();

    ImGui::SameLine(150.f);


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
        if (tAsset.isDirectory)
        {
            /* 폴더: 인스펙터 selection 변경 금지 */
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                m_currentFolder = tAsset.path;
                m_bListDirty = true;
            }
        }
        else
        {
            /* 파일: selection/notify 허용 */
            const _bool bChanged = (m_selectedPath.empty() || !(m_selectedPath == tAsset.path));
            Set_Selection(tAsset.path);

            if (bChanged)
                Notify_Selection_Changed();

            /* 더블클릭: 씬이면 열기*/ 
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                switch (tAsset.type)
                {
                case ASSET_TYPE::SCENE:
                    Try_Open_Scene_On_DoubleClick(tAsset);
                    break;
                case ASSET_TYPE::MATERIAL:
                    Try_Open_Material_On_DoublieClick(tAsset);
                    break;
                }
            }
        }
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        m_popupTargetPath = tAsset.path;
        m_popupTargetIsItem = true;
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

  
    ImGui::PopID();
}

/* ----------------------------- Context menu(blank space) ----------------------------- */
void CProjectPanel::Draw_Context_Menu()
{
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

void CProjectPanel::Draw_Context_Popup()
{
    /* 패널 영역 우클릭 시, 아이템 우클릭 타겟 설정이 아니라면 빈 공간 우클릭 취급 */
    const _bool bPanelHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);

    if (bPanelHovered && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {

        /* 방금 아이템에서 m_popupTargetIsItem을 true로 세팅했을 수도 있으니, 한 프레임에서 우클릭 타겟이 없으면 빈 공간으로 처리 */ 
        if (!m_popupTargetIsItem)
            m_popupTargetPath.clear();

        ImGui::OpenPopup("##ProjectContextUnified");
    }

    /* 팝업 그리기 */
    Draw_Project_Context_Unified();

    /* 프레임 끝에서 reset */ 
    m_popupTargetIsItem = false;
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

void CProjectPanel::Draw_Project_Context_Unified()
{
    if (ImGui::BeginPopup("##ProjectContextUnified"))
    {
        const _bool bHasTarget = !m_popupTargetPath.empty();

        /* TODO /* -------------------------------------------------------------- */
        /* TODO /*  Create 로직에 맞춰서 에셋 생성하는 부분 넣기                     */
        /* TODO /* -------------------------------------------------------------- */
        if (ImGui::BeginMenu("Create"))
        {
            if (ImGui::MenuItem("Folder"))
            {
                const std::filesystem::path createFolder = Resolve_Create_Folder_By_Type_("Folder");
                m_currentFolder = createFolder;

                std::filesystem::path created;
                if (Create_Folder(m_currentFolder, created))
                {
                    m_bTreeDirty = true;
                    m_bListDirty = true;
                    Set_Selection(created);
                    Notify_Selection_Changed();
                }
            }

            if (ImGui::MenuItem("Script"))
            {
                const std::filesystem::path createFolder = Resolve_Create_Folder_By_Type_("Script");
                m_currentFolder = createFolder;

                m_bOpenCreateScriptPopup = true;
                m_createScriptNameBuffer = "New_Script";
                m_bListDirty = true;
            }

            if (ImGui::MenuItem("Scene"))
            {
                const std::filesystem::path createFolder = Resolve_Create_Folder_By_Type_("Scenes");
                m_currentFolder = createFolder;

                m_bListDirty = true;
            }

            if (ImGui::MenuItem("Material"))
            {
                const std::filesystem::path createFolder = Resolve_Create_Folder_By_Type_("Material");
                m_currentFolder = createFolder;
                m_bOpenCreateMaterialPopup = true;
                m_bListDirty = true;
            }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        // Rename/Delete/Show in Explorer는 대상이 있을 때만 의미 있음
        if (ImGui::MenuItem("Rename", "F2", false, bHasTarget))
        {
            m_contextTargetPath = m_popupTargetPath;
            Begin_Rename(m_contextTargetPath);
        }

        if (ImGui::MenuItem("Delete", "Del", false, bHasTarget))
        {
            m_contextTargetPath = m_popupTargetPath;
            Delete_Path(m_contextTargetPath);

            m_bTreeDirty = true;
            m_bListDirty = true;

            if (!m_selectedPath.empty() && (m_selectedPath == m_contextTargetPath))
                Clear_Selection();
        }

        if (ImGui::MenuItem("Show in Explorer", nullptr, false, bHasTarget))
        {
            Show_In_Explorer(m_popupTargetPath);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Refresh"))
        {
            m_bTreeDirty = true;
            m_bListDirty = true;
        }

        ImGui::EndPopup();
    }
}

/* ----------------------------- Refresh ----------------------------- */
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

    const bool bRecursiveSearch = !m_search.empty();

    if (!bRecursiveSearch)
    {
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
    }
    else
    {
        /* search mode : recursive */
        for (auto& entry : std::filesystem::recursive_directory_iterator(m_currentFolder))
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

        for (auto& entry : std::filesystem::recursive_directory_iterator(m_currentFolder))
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
    }

    m_bListDirty = false;
}

/* ------------------------------ Helpers ------------------------------ */

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

/* ------------------------------ Operations ------------------------------ */
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

std::filesystem::path CProjectPanel::Resolve_Create_Base_Folder_() const
{
    /* 우클릭 타겟이 폴더면 그 폴더, 파일이면 부모 폴더, 빈 공간이면 현재 폴더 */
    std::filesystem::path base = m_currentFolder;

    if (!m_popupTargetPath.empty() && std::filesystem::exists(m_popupTargetPath))
    {
        if (std::filesystem::is_directory(m_popupTargetPath))
            base = m_popupTargetPath;
        else
            base = m_popupTargetPath.parent_path();
    }

    return base;
}

std::filesystem::path CProjectPanel::Resolve_Create_Folder_By_Type_(const char* szType) const
{
    // 기본은 우클릭 기준 폴더
    std::filesystem::path base = Resolve_Create_Base_Folder_();

    if (0 == std::strcmp(szType, "Scene"))
        return m_ScenePath;
    if (0 == std::strcmp(szType, "Script"))
        return m_ScriptPath;
    if (0 == std::strcmp(szType, "Material"))
        return m_MaterialPath;

    return base;
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

    if (Rename_Path(m_renameTargetPath, m_renameBuffer))
    {
        m_bListDirty = true;
        m_bTreeDirty = true;
    }

    Cancel_Rename();
}

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

void CProjectPanel::Open_Scene_By_GUID(const Engine::ASSET_GUID& guid)
{
    SYS_CORE.Open_EditScene(guid);
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
    Open_Scene_By_GUID(guid);
}

void CProjectPanel::Try_Open_Material_On_DoublieClick(const LIST_ASSET& tAsset)
{
    /* 경로로 guid 찾기 */
    if (!Try_Get_Asset_GUID(tAsset.path, m_editMaterialGUID))
    {
        _DEBUG_WARN("Material double click but GUID not found: %s", tAsset.path.string().c_str());
        return;
    }
    /* GUID -> ASSET_RECORD 찾기 */
    const ASSET_RECORD* pRec = SYS_ASSET.Find(m_editMaterialGUID);
    IF_NULL_RETURN_MSG_BREAK(pRec, , "Can't find ASSET_RECORD by GUID");

    const std::filesystem::path& matPath = pRec->path;

    /* MATERIAL ENTRY 받아오기 */
    const uint32_t hMat = SYS_RESOURCE.Load_Material(m_editMaterialGUID);
    IF_TRUE_RETURN_MSG_BREAK(hMat == INVALID_HANDLE_UINT, , "Load_Material failed: %s", matPath.string().c_str());

    MATERIAL_ENTRY* pEntry = SYS_RESOURCE.Get_Material(hMat);
    IF_NULL_RETURN_MSG_BREAK(pEntry, , "Get_Material returned null: %s", matPath.string().c_str());

    /* 정상적으로 찾은 경우 보일 값 채우기 */
    m_createMaterialShaderGUID = pEntry->shaderGUID;
    m_createMaterialBaseMapGUID = pEntry->baseMapGUID;
    m_createMaterialNormalMapGUID = pEntry->normalMapGUID;
    m_createMaterialBaseColor = pEntry->baseColor;
    m_createMaterialShininess = pEntry->fShininess;
    m_createMaterialPassIndex = pEntry->passIndex;
    m_createMaterialNameBuffer = Editor_Util::To_UTF8(matPath.stem());

    m_bEditMaterialPopup = true;
    m_bOpenCreateMaterialPopup = true;
}


/* ============================== Utility ================================*/
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

void CProjectPanel::Draw_Create_Script_Popup()
{
    if (!m_bOpenCreateScriptPopup)
        return;
    else
        ImGui::OpenPopup("##CreateScriptPopup");


    if (ImGui::BeginPopup("##CreateScriptPopup", ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Create Script");
        ImGui::Separator();

        ImGui::SetNextItemWidth(360.f);

        /* Enter 누르면 생성 */
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
        _bool bSubmit = ImGui::InputText("##ScriptName", &m_createScriptNameBuffer, flags);

        /* 버튼 눌러도 생성 */
        if (ImGui::Button("Create") || bSubmit)
        {
            std::filesystem::path createdPath;

            /* 이름이 비었으면 막기 */
            if (!m_createScriptNameBuffer.empty())
            {
                if (Create_Script_By_Name(m_createScriptNameBuffer, createdPath))
                {
                    m_bListDirty = true;
                    m_bTreeDirty = true;

                    Set_Selection(createdPath);
                    Notify_Selection_Changed();

                    m_bOpenCreateScriptPopup = false;
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            m_bOpenCreateScriptPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

_bool CProjectPanel::Create_Script_By_Name(const std::string& baseStem, std::filesystem::path& outCreatedPath)
{
    outCreatedPath.clear();

    /* unique stem 보장 */
    const std::string stem = CCreate_Asset_Helper::Make_Unique_File_Stem_Impl(m_ScriptPath, baseStem, m_HeaderPath, m_ImplPath);
    const std::string className = "C" + CCreate_Asset_Helper::Make_Script_File_From_Stem(stem);

    const std::filesystem::path headerPath = m_HeaderPath / (stem + ".h");
    const std::filesystem::path cppPath = m_ImplPath / (stem + ".cpp");

    if (std::filesystem::exists(headerPath) || std::filesystem::exists(cppPath))
        return false;

    /* 헤더/CPP 생성 */
    std::string header;
    std::string cpp;
    CCreate_Asset_Helper::Build_Script_Source(className, headerPath, header, cpp);
    if (!CCreate_Asset_Helper::Write_Script_Source_Files(headerPath, cppPath, header, cpp))
        return false;

    /* .script 생성 + GUID 보장 */
    const std::filesystem::path savePath = m_ScriptPath / (stem + ".script");
    if (!CCreate_Asset_Helper::Write_Script_Asset_File(savePath, className, headerPath))
        return false;

    outCreatedPath = savePath;
    return true;
}

void CProjectPanel::Draw_Create_Material_Popup()
{
    if (!m_bOpenCreateMaterialPopup)
        return;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin("Create Material", &m_bOpenCreateMaterialPopup, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SetNextItemWidth(360.f);
        ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
        _bool bSubmit = ImGui::InputText("##MaterialName", &m_createMaterialNameBuffer, flags);

        ImGui::Spacing();

        {
            std::filesystem::path shaderPath = SYS_ASSET.Get_Asset_Path(m_createMaterialShaderGUID);
            std::string strShaderName = shaderPath.empty() ? std::string("<None>") : Editor_Util::To_UTF8(shaderPath.filename());

            ImGui::Text("Shader");
            ImGui::TextDisabled("%s", strShaderName.c_str());

            Editor_Util::Draw_DropTarget_GUID_Typed(
                "Shader",
                "ASSET_GUID",
                ASSET_TYPE::SHADER,
                [&](const ASSET_GUID& dropped)
                {
                    m_createMaterialShaderGUID = dropped;
                },
                "Drop Shader here"
            );
        }

        ImGui::Spacing();

        {
            std::filesystem::path baseMapPath = SYS_ASSET.Get_Asset_Path(m_createMaterialBaseMapGUID);
            std::string strBaseMapName = baseMapPath.empty() ? std::string("<None>") : Editor_Util::To_UTF8(baseMapPath.filename());

            ImGui::Text("BaseMap");
            ImGui::TextDisabled("%s", strBaseMapName.c_str());

            Editor_Util::Draw_DropTarget_GUID_Typed(
                "BaseMap",
                "ASSET_GUID",
                ASSET_TYPE::TEXTURE,
                [&](const ASSET_GUID& dropped)
                {
                    m_createMaterialBaseMapGUID = dropped;
                },
                "Drop Texture here"
            );
        }

        ImGui::Spacing();

        {
            std::filesystem::path normalMapPath = SYS_ASSET.Get_Asset_Path(m_createMaterialNormalMapGUID);
            std::string strNormalMapName = normalMapPath.empty() ? std::string("<None>") : Editor_Util::To_UTF8(normalMapPath.filename());

            ImGui::Text("NormalMap");
            ImGui::TextDisabled("%s", strNormalMapName.c_str());

            Editor_Util::Draw_DropTarget_GUID_Typed(
                "NormalMap",
                "ASSET_GUID",
                ASSET_TYPE::TEXTURE,
                [&](const ASSET_GUID& dropped)
                {
                    m_createMaterialNormalMapGUID = dropped;
                },
                "Drop Normal Texture here"
            );
        }

        ImGui::Spacing();

        ImGui::Text("BaseColor");
        ImGui::ColorEdit4("##MaterialBaseColor", &m_createMaterialBaseColor.x);

        ImGui::Spacing();

        ImGui::Text("Shininess");
        ImGui::SetNextItemWidth(160.f);
        ImGui::DragFloat("##MaterialShininess", &m_createMaterialShininess, 0.1f, 0.f, 512.f, "%.2f");

        ImGui::Spacing();

        {
            int iPassIndex = SCAST(int, m_createMaterialPassIndex);
            ImGui::Text("PassIndex");
            ImGui::SetNextItemWidth(160.f);
            if (ImGui::DragInt("##MaterialPassIndex", &iPassIndex, 1.f, 0, 255))
                m_createMaterialPassIndex = SCAST(_uint, iPassIndex);
        }

        ImGui::Spacing();

        if (ImGui::Button(m_bEditMaterialPopup ? "Save" : "Create") || bSubmit)
        {
            std::filesystem::path targetPath;

            if (m_bEditMaterialPopup) /* ----------------- Edit ----------------- */
            {
                const ASSET_RECORD* pRec = SYS_ASSET.Find(m_editMaterialGUID);
                if (pRec)
                    targetPath = pRec->path;

                if (!targetPath.empty())
                {
                    if (CCreate_Asset_Helper::Write_Material_Asset_File(
                        targetPath,
                        m_editMaterialGUID,
                        m_createMaterialShaderGUID,
                        m_createMaterialBaseMapGUID,
                        m_createMaterialNormalMapGUID,
                        m_createMaterialBaseColor,
                        m_createMaterialShininess,
                        m_createMaterialPassIndex))
                    {
                        m_bListDirty = true;
                        m_bTreeDirty = true;

                        Set_Selection(targetPath);
                        Notify_Selection_Changed();

                        auto hMaterial = SYS_RESOURCE.Load_Material(m_editMaterialGUID);
                        auto* pMat = SYS_RESOURCE.Get_Material(hMaterial);
                        if (pMat)
                        {
                            pMat->shaderGUID = m_createMaterialShaderGUID;
                            pMat->hShader = SYS_RESOURCE.Load_Shader(m_createMaterialShaderGUID);

                            pMat->baseMapGUID = m_createMaterialBaseMapGUID;
                            pMat->hBaseMap = SYS_RESOURCE.Load_Texture(m_createMaterialBaseMapGUID);

                            pMat->normalMapGUID = m_createMaterialNormalMapGUID;
                            pMat->hNormalMap = SYS_RESOURCE.Load_Texture(m_createMaterialNormalMapGUID);

                            pMat->baseColor = m_createMaterialBaseColor;
                            pMat->fShininess = m_createMaterialShininess;
                            pMat->passIndex = m_createMaterialPassIndex;
                        }

                        SYS_ASSET.Register_File_Asset(targetPath, ASSET_TYPE::MATERIAL, m_editMaterialGUID);
                    }
                }
            }
            else /* ----------------- Create ----------------- */
            {
                std::filesystem::path createdPath;
                if (!m_createMaterialNameBuffer.empty())
                {
                    if (Create_Material_By_Name(m_createMaterialNameBuffer, createdPath))
                    {
                        m_bListDirty = true;
                        m_bTreeDirty = true;

                        Set_Selection(createdPath);
                        Notify_Selection_Changed();

                        SYS_ASSET.Register_File_Asset(createdPath, ASSET_TYPE::MATERIAL);
                    }
                }
            }

            m_createMaterialNameBuffer.clear();
            m_createMaterialShaderGUID = DEFAULT_ASSET_GUID::SHADER_VTXTEX;
            m_createMaterialBaseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;
            m_createMaterialNormalMapGUID = DEFAULT_ASSET_GUID::TEXTURE_NORMALMAP_DEFAULT;
            m_createMaterialBaseColor = _float4{ 1.f, 1.f, 1.f, 1.f };
            m_createMaterialShininess = 32.f;
            m_createMaterialPassIndex = 0;

            m_bEditMaterialPopup = false;
            m_editMaterialGUID = ASSET_GUID{};
            m_bOpenCreateMaterialPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape))
        {
            m_createMaterialNameBuffer.clear();
            m_createMaterialShaderGUID = DEFAULT_ASSET_GUID::SHADER_VTXTEX;
            m_createMaterialBaseMapGUID = DEFAULT_ASSET_GUID::TEXTURE_BASEMAP_DEFAULT;
            m_createMaterialNormalMapGUID = DEFAULT_ASSET_GUID::TEXTURE_NORMALMAP_DEFAULT;
            m_createMaterialBaseColor = _float4{ 1.f, 1.f, 1.f, 1.f };
            m_createMaterialShininess = 32.f;
            m_createMaterialPassIndex = 0;

            m_bEditMaterialPopup = false;
            m_editMaterialGUID = ASSET_GUID{};
            m_bOpenCreateMaterialPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::End();
    }
}

_bool CProjectPanel::Create_Material_By_Name(const std::string& baseStem, std::filesystem::path& outCreatedPath)
{
    outCreatedPath.clear();

    const std::string stem = CCreate_Asset_Helper::Make_Unique_File_Stem_Single(m_MaterialPath, baseStem, ".mat");
    const std::filesystem::path savePath = m_MaterialPath / (stem + ".mat");

    if (std::filesystem::exists(savePath))
        return false;

    ASSET_GUID materialGUID = ASSET_GUID::New_GUID();

    if (!CCreate_Asset_Helper::Write_Material_Asset_File(
        savePath,
        materialGUID,
        m_createMaterialShaderGUID,
        m_createMaterialBaseMapGUID,
        m_createMaterialNormalMapGUID,
        m_createMaterialBaseColor,
        m_createMaterialShininess,
        m_createMaterialPassIndex))
    {
        return false;
    }

    outCreatedPath = savePath;

    return true;
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
