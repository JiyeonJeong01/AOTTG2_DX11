#pragma once
#include "EditorPanel.h"
#include "Event.h"

NS_BEGIN(Engine)
class CGameObject;
class CInspectorPanel;
NS_END

NS_BEGIN(Editor)

class CHierarchyPanel final : public CEditorPanel
{
public:
    CHierarchyPanel(const std::string& strPanelName);
    ~CHierarchyPanel() override = default;

    HRESULT Initialize() override;
    void Update() override;
    void Render() override;

    /* Bind with Inspector Panel */
    void Set_On_Primary_Selection_Changed(std::function<void(Engine::CGameObject*)> fn);
    void Set_On_Selection_Changed(std::function<void(const std::vector<Engine::CGameObject*>&)> fn);

    /* Object selection API */
    Engine::CGameObject* Get_Primary_Selection() const;
    const std::vector<Engine::CGameObject*>& Get_Selection() const;

    void Set_Selection_Single(Engine::CGameObject* pObj);
    void Add_Selection(Engine::CGameObject* pObj);                                  /* Ctrl */
    void Toggle_Selection(Engine::CGameObject* pObj);                               /* Ctrl + Click */
    void Set_Selection_Range(Engine::CGameObject* pFrom, Engine::CGameObject* pTo); /* Shift */
    void Clear_Selection();

    bool Is_Selected(Engine::CGameObject* pObj) const;

    /* Rename object */
    void Begin_Rename(Engine::CGameObject* pObj);
    void Cancel_Rename();
    bool Is_Renaming() const;

    /* Options */
    void Set_Allow_Multi_Select(_bool b) { m_bAllowMultiSelect = b; }
    void Set_Show_Search(_bool b) { m_bShowSearch = b; }
    void Set_Show_Toolbar(_bool b) { m_bShowToolbar = b; }
    void Set_Show_Active_Toggle(_bool b) { m_bShowActiveToggle = b; }
    void Set_Allow_DragDrop(_bool b) { m_bAllowDragDrop = b; }
    void Set_Allow_Rename(_bool b) { m_bAllowRename = b; }
    void Set_Allow_Delete(_bool b) { m_bAllowDelete = b; }
    void Set_Allow_Duplicate(_bool b) { m_bAllowDuplicate = b; }
    void Set_Double_Click_To_Rename(_bool b) { m_bDoubleClickToRename = b; }

private:
    /* Main render section */
    void Draw_Toolbar();
    void Draw_Search_Bar();
    void Draw_Object_Tree();
    void Draw_Context_Menu();

    /* Tree render helpers */
    void Refresh_Roots();   /* Refresh if dirty */
    void Draw_Root_List();
    void Draw_Node_Recursive(Engine::CGameObject* pObj, _int iDepth);

    bool Is_Visible_By_Filter(Engine::CGameObject* pObj) const;
    bool Is_Visible_By_Filter_Recursive(Engine::CGameObject* pObj);

    /* Interaction helpers */
    void Handle_Node_Click(Engine::CGameObject* pObj);
    void Handle_Shortcuts();                            /* Delete, F2, Ctrl + D, Ctrl + F, Esc... */
    void Handle_DragDrop(Engine::CGameObject* pObj);

    /* Operations */
    Engine::CGameObject* Create_Empty_Object(Engine::CGameObject* pParent /*nullable*/);
    Engine::CGameObject* Duplicate_Object(Engine::CGameObject* pSrc, Engine::CGameObject* pParent /*nullable*/);
    void Destroy_Object(Engine::CGameObject* pObj);

    /* Rename UI */
    void Draw_Rename_Field(Engine::CGameObject* pObj);
    void Commit_Rename();

    /* Internal */
    void Notify_Selection_Changed();
    void Validate_Selection();

    static uint64_t Get_Stable_Id(Engine::CGameObject* pObj); // 기본: pointer cast
    static bool String_IContains(const std::string& haystack, const std::string& needle);

public :
    CEvent<Engine::CGameObject*> m_OnPrimarySelectionChanged{};

private:
    // data
    std::vector<Engine::CGameObject*> m_roots;
    std::vector<Engine::CGameObject*> m_selection;
    Engine::CGameObject* m_pLastClicked = nullptr;

    std::unordered_set<uint64_t> m_openNodes;

    // filter
    std::string m_search;
    bool m_bRequestFocusSearch = false;

    // rename
    Engine::CGameObject* m_pRenameTarget = nullptr;
    std::string m_renameBuffer;
    bool m_bJustStartedRename = false;

    // context menu target
    Engine::CGameObject* m_pContextTarget = nullptr;

    // callbacks
    std::function<void(Engine::CGameObject*)> m_fnPrimarySelectionChanged;
    std::function<void(const std::vector<Engine::CGameObject*>&)> m_fnSelectionChanged;

    // options
    bool m_bAllowMultiSelect = true;
    bool m_bShowSearch = true;
    bool m_bShowToolbar = true;
    bool m_bShowActiveToggle = true;

    bool m_bAllowDragDrop = true;
    bool m_bAllowRename = true;
    bool m_bAllowDelete = true;
    bool m_bAllowDuplicate = true;
    bool m_bDoubleClickToRename = true;

    // window
    static constexpr const char* PAYLOAD_GO_PTR = "HIERARCHY_GO_PTR";

public:
        static CHierarchyPanel* Create(const std::string& strPanelName);
private:
    void Free() override;
};

NS_END
