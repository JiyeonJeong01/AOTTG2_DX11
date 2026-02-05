#include "HierarchyPanel.h"

#include "Editor_Util.h"
#include "GameObject.h"
#include "GameObject_System.h"
#include "Asset_Registry.h"

NS_BEGIN(Editor)

CHierarchyPanel::CHierarchyPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

HRESULT CHierarchyPanel::Initialize()
{
    Refresh_Roots();

    return S_OK;
}

void CHierarchyPanel::Update()
{
    /* TODO : Consider using a dirty list for refresh,,, */
    Refresh_Roots();
    Validate_Selection();
}

void CHierarchyPanel::Render()
{
    if (!ImGui::Begin(m_strPanelName.c_str())) /* MainPanel::PANEL_HIERARCHY = "Hierarchy" */
    {
        ImGui::End();
        return;
    }

    Draw_Toolbar();
    Draw_Search_Bar();

    Handle_Shortcuts();
    Draw_Object_Tree();
    Draw_Context_Menu();
    Draw_DropTarget();

    ImGui::End();
}

/* =======================================================================*/
/* ============================= Callbacks ===============================*/
/* =======================================================================*/
void CHierarchyPanel::Set_On_Primary_Selection_Changed(std::function<void(Engine::CGameObject*)> fn)
{
    m_fnPrimarySelectionChanged = std::move(fn);
}

void CHierarchyPanel::Set_On_Selection_Changed(std::function<void(const std::vector<Engine::CGameObject*>&)> fn)
{
    m_fnSelectionChanged = std::move(fn);
}

/* =======================================================================*/
/* =========================== Selection API =============================*/
/* =======================================================================*/
Engine::CGameObject* CHierarchyPanel::Get_Primary_Selection() const
{
    return m_selection.empty() ? nullptr : m_selection.front();
}

const std::vector<Engine::CGameObject*>& CHierarchyPanel::Get_Selection() const
{
    return m_selection;
}

bool CHierarchyPanel::Is_Selected(Engine::CGameObject* pObj) const
{
    return std::find(m_selection.begin(), m_selection.end(), pObj) != m_selection.end();
}

void CHierarchyPanel::Set_Selection_Single(Engine::CGameObject* pObj)
{
    m_selection.clear();
    if (pObj)
        m_selection.push_back(pObj);
    m_pLastClicked = pObj;

    Notify_Selection_Changed();
}

void CHierarchyPanel::Add_Selection(Engine::CGameObject* pObj)
{
    if (!pObj)
        return;
    if (!Is_Selected(pObj))
    {
        m_selection.push_back(pObj);
        m_pLastClicked = pObj;

        Notify_Selection_Changed();
    }
}

void CHierarchyPanel::Toggle_Selection(Engine::CGameObject* pObj)
{
    if (!pObj)
        return;

    auto it = std::find(m_selection.begin(), m_selection.end(), pObj);
    if (it != m_selection.end())
        m_selection.erase(it);
    else
        m_selection.push_back(pObj);

    m_pLastClicked = pObj;
    Notify_Selection_Changed();
}

void CHierarchyPanel::Set_Selection_Range(Engine::CGameObject* pFrom, Engine::CGameObject* pTo)
{
    /* TODO : Support only the parent's sibling range, as the current tree display order is undefined */
    if (!pFrom || !pTo)
    {
        Set_Selection_Single(pTo);
        return;
    }

    Engine::CGameObject* pFromParent = pFrom->Get_Parent();
    Engine::CGameObject* pToParent = pTo->Get_Parent();
    if (pFromParent != pToParent)
    {
        /* TODO : Expand to display order based range */
        Set_Selection_Single(pTo);
        return;
    }

    const std::vector<Engine::CGameObject*>& siblings = (pFromParent ? pFromParent->Get_Children() : m_roots);

    auto itA = std::find(siblings.begin(), siblings.end(), pFrom);
    auto itB = std::find(siblings.begin(), siblings.end(), pTo);
    if (itA == siblings.end() || itB == siblings.end())
    {
        Set_Selection_Single(pTo);
        return;
    }

    if (itA > itB)
        std::swap(itA, itB);

    m_selection.clear();
    for (auto it = itA; it != itB + 1; ++it)
        m_selection.push_back(*it);

    m_pLastClicked = pTo;

    Notify_Selection_Changed();
}

void CHierarchyPanel::Clear_Selection()
{
    if (m_selection.empty())
        return;
    m_selection.clear();
    m_pLastClicked = nullptr;

    Notify_Selection_Changed();
}

/* =======================================================================*/
/* =============================== Rename ================================*/
/* =======================================================================*/
void CHierarchyPanel::Begin_Rename(Engine::CGameObject* pObj)
{
    if (!pObj)
        return;

    m_pRenameTarget = pObj;
    m_renameBuffer.assign(pObj->Get_Label());
    m_renameBuffer.reserve(256);
    m_bJustStartedRename = true;
}

void CHierarchyPanel::Cancel_Rename()
{
    m_pRenameTarget = nullptr;
    m_renameBuffer.clear();
    m_bJustStartedRename = false;
}

bool CHierarchyPanel::Is_Renaming() const
{
    return m_pRenameTarget != nullptr;
}

/* =======================================================================*/
/* ============================ Draw Section =============================*/
/* =======================================================================*/
void CHierarchyPanel::Draw_Toolbar()
{
    /* Create Object */
    if (ImGui::Button("Create"))
    {
        Engine::CGameObject* pNew = Create_Empty_Object(nullptr);
        if (pNew)
        {
            Set_Selection_Single(pNew);
            Begin_Rename(pNew);
        }
    }

    ImGui::SameLine();

    /* Duplicate Object */ 
    ImGui::BeginDisabled(Get_Primary_Selection() == nullptr);
    if (ImGui::Button("Duplicate"))
    {
        Engine::CGameObject* pSel = Get_Primary_Selection();
        Engine::CGameObject* pParent = pSel ? pSel->Get_Parent() : nullptr;
        Engine::CGameObject* pDup = Duplicate_Object(pSel, pParent);
        if (pDup)
        {
            Set_Selection_Single(pDup);
            Begin_Rename(pDup);
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();

    /* Delete Object */
    ImGui::BeginDisabled(m_selection.empty());
    if (ImGui::Button("Delete"))
    {
        auto toDelete = m_selection;
        Clear_Selection();
        for (auto* obj : toDelete)
            Destroy_Object(obj);
    }
    ImGui::EndDisabled();

    ImGui::Separator();
}

void CHierarchyPanel::Draw_Search_Bar()
{
    ImGui::TextUnformatted("Search");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(-1.f);
    if (m_bRequestFocusSearch)
    {
        ImGui::SetKeyboardFocusHere();
        m_bRequestFocusSearch = false;
    }

    if (ImGui::InputText("##HierarchySearch", &m_search))
    {
        // filter changed
    }
    ImGui::Separator();
}

void CHierarchyPanel::Draw_Object_Tree()
{
    /* Background right-click for context menu */
    ImGui::BeginChild("##HierarchyTreeRegion", ImVec2(0, 0), false,
        ImGuiWindowFlags_HorizontalScrollbar);

    /* Deselect in empty space on click */
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (!ImGui::IsAnyItemHovered())
            Clear_Selection();
    }

    Draw_Root_List();

    ImGui::EndChild();
}

void CHierarchyPanel::Draw_Context_Menu()
{
    /*  Right-click on GameOjbect */
    if (ImGui::BeginPopupContextWindow("##HierarchyBlankContext", ImGuiPopupFlags_MouseButtonRight))
    {
        m_pContextTarget = nullptr;

        /* Right-click on empty space, Context menu */
        if (ImGui::MenuItem("Create Empty"))
        {
            Engine::CGameObject* pNew = Create_Empty_Object(nullptr);
            if (pNew) { Set_Selection_Single(pNew); Begin_Rename(pNew); }
        }

        ImGui::EndPopup();
    }
}

void CHierarchyPanel::Draw_DropTarget()
{
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ASSET_GUID"))
        {
            const Engine::ASSET_GUID* pg = SCAST(const Engine::ASSET_GUID*, p->Data);

            const Engine::ASSET_RECORD* rec = SYS_RESOURCE->Find(*pg);
            if (rec)
            {
                // GUID + path 로그
                LOG_INFO("Dropped Asset GUID=%s path=%s", pg->To_String_Utf8().c_str(), rec->path.string().c_str());

                switch (rec->eType)
                {
                case Engine::ASSET_TYPE::PROTOTYPE:
                    LOG_INFO("Prototype dropped");
                    break;
                    case Engine::ASSET_TYPE::TEXTURE:
                    LOG_INFO("Texture dropped");
                    break;
                default:
                    LOG_INFO("Other asset dropped");
                    break;
                }
            }
            else
            {
                _DEBUG_WARN("Dropped GUID but not found in registry");
            }
        }
        ImGui::EndDragDropTarget();
    }
}

/* =======================================================================*/
/* ============================ Tree Helpers =============================*/
/* =======================================================================*/
void CHierarchyPanel::Refresh_Roots()
{
    SYS_GAMEOBJECT->Get_Roots(m_roots);
}

void CHierarchyPanel::Draw_Root_List()
{
    /* TODO : Wrap with an additional TreeNode for a SCENE View in Unity */
    for (auto* root : m_roots)
    {
        if (!root)
            continue;
        if (!Is_Visible_By_Filter_Recursive(root))
            continue;
        Draw_Node_Recursive(root, 0);
    }
}

bool CHierarchyPanel::Is_Visible_By_Filter(Engine::CGameObject* pObj) const
{
    if (m_search.empty())
        return true;
    if (!pObj)
        return false;

    const char* szName = pObj->Get_Label().data();
    std::string strName = (szName ? szName : "");
    return Editor_Util::Str_IContains(strName, m_search);
}

bool CHierarchyPanel::Is_Visible_By_Filter_Recursive(Engine::CGameObject* pObj)
{
    if (Is_Visible_By_Filter(pObj))
        return true;

    const std::vector<Engine::CGameObject*>& pChildren = pObj->Get_Children();

    for (auto* ch : pChildren)
    {
        if (!ch) continue;
        if (Is_Visible_By_Filter_Recursive(ch))
            return true;
    }
    return false;
}

void CHierarchyPanel::Draw_Node_Recursive(Engine::CGameObject* pObj, int /*iDepth*/)
{
    if (!pObj)
        return;

    /* Check children */
    const std::vector<Engine::CGameObject*>& pChildren = pObj->Get_Children();

    const _bool bHasChildren = !pChildren.empty();

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (!bHasChildren)
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (Is_Selected(pObj))
        flags |= ImGuiTreeNodeFlags_Selected;

    /* Keep open state */
    uint64_t iID = Get_Stable_Id(pObj);
    _bool bOpen = (m_openNodes.find(iID) != m_openNodes.end());

    _bool bActive = pObj->Get_Active();

    const char* szName = pObj->Get_Label().data();
    if (!szName)
        szName = "";

    /* Unique label to avoid ImGui ID conflicts */
    std::string strLabel = std::string(szName) + "##" + std::to_string((uintptr_t)pObj);

    if (bHasChildren)
        ImGui::SetNextItemOpen(bOpen, ImGuiCond_Always);

    _bool bOpened = ImGui::TreeNodeEx(strLabel.c_str(), flags);

    /* Handle click */
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        Handle_Node_Click(pObj);

    /* Handle double click to rename */
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        Begin_Rename(pObj);

    /* Handle right-click to display menu */
        if (ImGui::BeginPopupContextItem(("##HierarchyNodeCtx" + std::to_string((uintptr_t)pObj)).c_str()))
        {
            m_pContextTarget = pObj;

            if (ImGui::MenuItem("Create Empty Child"))
            {
                Engine::CGameObject* pNew = Create_Empty_Object(pObj);
                if (pNew)
                {
                    Set_Selection_Single(pNew);
                    Begin_Rename(pNew);
                }
            }

            if (ImGui::MenuItem("Duplicate"))
            {
                Engine::CGameObject* pDup = Duplicate_Object(pObj, pObj->Get_Parent());
                if (pDup)
                {
                    Set_Selection_Single(pDup);
                    Begin_Rename(pDup);
                }
            }

            if (ImGui::MenuItem("Rename", "F2"))
                Begin_Rename(pObj);

            if (ImGui::MenuItem("Delete", "Del"))
            {
                /* Delete all selected items if the target is selected, otherwise delete only the target */
                if (Is_Selected(pObj) && !m_selection.empty())
                {
                    auto toDelete = m_selection;
                    Clear_Selection();
                    for (auto* obj : toDelete)
                        Destroy_Object(obj);
                }
                else
                {
                    if (Get_Primary_Selection() == pObj)
                        Clear_Selection();
                    Destroy_Object(pObj);
                }
            }


            _bool bCur = pObj->Get_Active();
            if (ImGui::MenuItem(bCur ? "Set Inactive" : "Set Active"))
                pObj->Set_Active(!bCur);

            ImGui::EndPopup();
        }

    /* Reparent by drag-and-drop */
    Handle_DragDrop(pObj);
        

    /* Rename the target */
    if (m_pRenameTarget == pObj)
        Draw_Rename_Field(pObj);

    /* Save open state */
    if (bHasChildren)
    {
        if (bOpened && !bOpen)
            m_openNodes.insert(iID);
        if (!bOpened && bOpen)
            m_openNodes.erase(iID);
    }

    /* Draw Children */
    if (bHasChildren && bOpened)
    {
        for (auto* child : pChildren)
        {
            if (!child) continue;
            if (!Is_Visible_By_Filter_Recursive(child)) continue;
            Draw_Node_Recursive(child, 0);
        }
        ImGui::TreePop();
    }
}

/* =======================================================================*/
/* ============================ Interaction ==============================*/
/* =======================================================================*/
void CHierarchyPanel::Handle_Node_Click(Engine::CGameObject* pObj)
{
    ImGuiIO& io = ImGui::GetIO();

    const _bool bSift = io.KeyShift;
    const _bool bCtrl = io.KeyCtrl;

    if (bSift && m_pLastClicked)
    {
        Set_Selection_Range(m_pLastClicked, pObj);
        return;
    }

    if (bCtrl)
    {
        Toggle_Selection(pObj);
        return;
    }

    Set_Selection_Single(pObj);
}

void CHierarchyPanel::Handle_Shortcuts()
{
    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        return;

    /* Handle ESC : Cancel renaming if active; otherwise, clear the current selection*/
    if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
    {
        if (Is_Renaming())
            Cancel_Rename();
        else
            Clear_Selection();
    }

    /* Handle Ctrl+F : Set input focus to the search bar */
    if (Editor_Util::Is_KeyChord_Pressed(true, false, false, ImGuiKey_F))
        m_bRequestFocusSearch = true;

    /* Handle F2 : Trigger the renaming process */
    if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
    {
        if (auto* p = Get_Primary_Selection())
            Begin_Rename(p);
    }

    /* Handle Delete : Permanently remove all currently objects */
    if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
    {
        if (!m_selection.empty())
        {
            auto toDelete = m_selection;
            Clear_Selection();
            for (auto* obj : toDelete)
                Destroy_Object(obj);
        }
    }

    /* Handle Ctrl + D: Duplicate of the selected object */
    if (Editor_Util::Is_KeyChord_Pressed(true, false, false, ImGuiKey_D))
    {
        if (auto* p = Get_Primary_Selection())
        {
            auto* parent = p->Get_Parent();
            Engine::CGameObject* pDup = Duplicate_Object(p, parent);
            if (pDup)
            {
                Set_Selection_Single(pDup);
                Begin_Rename(pDup);
            }
        }
    }
}

void CHierarchyPanel::Handle_DragDrop(Engine::CGameObject* pObj)
{
    /* Drag and Drop */
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
        Engine::CGameObject* pPayloadObj = pObj;
        ImGui::SetDragDropPayload(PAYLOAD_GO_PTR, &pPayloadObj, sizeof(pPayloadObj));
        ImGui::Text("Move: %s", pObj->Get_Label().data());
        ImGui::EndDragDropSource();
    }

    /* This node can accept drop */
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload(PAYLOAD_GO_PTR))
        {
            /* pPayloadObj is CGameObject*, so &pPayloadObj is CGameObject*, therefore cast void* to CGameObject** */
            Engine::CGameObject* pDropped = *(Engine::CGameObject**)p->Data;
            if (pDropped && pDropped != pObj)
            {
                _bool bCycle = false;
                {
                    Engine::CGameObject* pCurObj = pObj;
                    while (pCurObj)
                    {
                        if (pCurObj == pDropped)
                        {
                            bCycle = true;
                            break;
                        }
                        pCurObj = pCurObj->Get_Parent();
                    }
                }

                if (!bCycle)
                {
                    pDropped->Set_Parent(pObj);
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

/* =======================================================================*/
/* ============================= Operation ===============================*/
/* =======================================================================*/
Engine::CGameObject* CHierarchyPanel::Create_Empty_Object(Engine::CGameObject* pParent)
{
    std::string szBaseName = "New GameObject";
    Engine::CGameObject* pNew = CGameObject::Create(Layer::DEFAULT_LAYER, szBaseName, pParent);

    return pNew;
}

Engine::CGameObject* CHierarchyPanel::Duplicate_Object(Engine::CGameObject* pSrc, Engine::CGameObject* pParent)
{
    if (!pSrc)
        return nullptr;

    Engine::CGameObject* pDup = pSrc->Clone();

    return pDup;
}

void CHierarchyPanel::Destroy_Object(Engine::CGameObject* pObj)
{
    if (!pObj)
        return;

    if (m_pRenameTarget == pObj)
        Cancel_Rename();

    SYS_GAMEOBJECT->Destroy_Object(pObj);
}

/* =======================================================================*/
/* ============================= Rename UI ===============================*/
/* =======================================================================*/

void CHierarchyPanel::Draw_Rename_Field(Engine::CGameObject* pObj)
{
    if (m_bJustStartedRename)
    {
        ImGui::SetKeyboardFocusHere();
    }

    ImGui::PushID((int)(uintptr_t)pObj);
    ImGui::SetNextItemWidth(-1.f);

    ImGuiInputTextFlags flags =
        ImGuiInputTextFlags_AutoSelectAll |
        ImGuiInputTextFlags_EnterReturnsTrue;

    bool enter = ImGui::InputText("##Rename", &m_renameBuffer, flags);

    if (enter)
    {
        Commit_Rename();
        ImGui::PopID();
        m_bJustStartedRename = false;
        return;
    }

    if (m_bJustStartedRename)
    {
        m_bJustStartedRename = false;
        ImGui::PopID();
        return;
    }

    if (ImGui::IsItemDeactivated())
    {
        Commit_Rename();
        ImGui::PopID();
        return;
    }

    ImGui::PopID();
}


void CHierarchyPanel::Commit_Rename()
{
    if (!m_pRenameTarget)
        return;

    if (m_renameBuffer.empty())
        m_renameBuffer = "GameObject";

    m_pRenameTarget->Set_Label(m_renameBuffer.c_str());
    Cancel_Rename();
}

/* =======================================================================*/
/* ============================== Internal ===============================*/
/* =======================================================================*/
void CHierarchyPanel::Notify_Selection_Changed()
{
    if (m_fnSelectionChanged)
        m_fnSelectionChanged(m_selection);

    if (m_fnPrimarySelectionChanged)
        m_fnPrimarySelectionChanged(Get_Primary_Selection());

    m_OnPrimarySelectionChanged.Invoke(Get_Primary_Selection());
}

void CHierarchyPanel::Validate_Selection()
{
    /* TODO : Things to switch from per-frame updates to event-based handling */
    /* TODO : If there's no way to detect engine-deleted pointers here, at least remove nullptr entries */
    m_selection.erase(
        std::remove(m_selection.begin(), m_selection.end(), nullptr),
        m_selection.end()
    );

    if (m_pLastClicked && !Is_Selected(m_pLastClicked))
    {
        /* If the last clicked GameObject is removed from the selection, the primary selection need to be updated */
        if (!m_selection.empty())
            m_pLastClicked = m_selection.back();
        else
            m_pLastClicked = nullptr;
    }
}

uint64_t CHierarchyPanel::Get_Stable_Id(Engine::CGameObject* pObj)
{
    return Editor_Util::Stable_Id(pObj);
}

bool CHierarchyPanel::String_IContains(const std::string& haystack, const std::string& needle)
{
    return Editor_Util::Str_IContains(haystack, needle);
}

CHierarchyPanel* CHierarchyPanel::Create(const std::string& strPanelName)
{
    CHierarchyPanel* pInstance = new CHierarchyPanel(strPanelName);
    if (FAILED(pInstance->Initialize()))
    {
        Safe_Release(pInstance);
        _DEBUG_ERROR_BREAK("CHierarchyPanel Create failed");
    }
    return pInstance;
}

void CHierarchyPanel::Free()
{
    CEditorPanel::Free();
}

NS_END
