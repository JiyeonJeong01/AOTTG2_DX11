#pragma once
#include "EditorPanel.h"
#include "Event.h"
#include "GameObject_Event.h"

NS_BEGIN(Engine)
class CGameObject;
class CGameObject_System;
class CInspectorPanel;
NS_END

NS_BEGIN(Editor)

class CHierarchyPanel final : public CEditorPanel
{
public:
    CHierarchyPanel(const std::string& strPanelName);
    ~CHierarchyPanel() override ;

public :
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

    _bool Is_Selected(Engine::CGameObject* pObj) const;
    void On_PickedObject(Engine::GAMEOBJECT_EVENT_DATA& tEvent);

    /* Rename object */
    void Begin_Rename(Engine::CGameObject* pObj);
    void Cancel_Rename();
    bool Is_Renaming() const;

private:
    /* Main render section */
    void Draw_Toolbar();
    void Draw_Search_Bar();
    void Draw_Object_Tree();
    void Draw_Context_Menu();
    void Draw_DropTarget();

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

    static uint64_t Get_Stable_Id(Engine::CGameObject* pObj);
    static bool String_IContains(const std::string& haystack, const std::string& needle);

public :
    CEvent<Engine::CGameObject*> m_OnPrimarySelectionChanged;

private:
    /* Data */
    std::vector<Engine::CGameObject*> m_roots;
    std::vector<Engine::CGameObject*> m_selection;
    Engine::CGameObject* m_pLastClicked = nullptr;

    std::unordered_set<uint64_t> m_openNodes;

    /* Filter */
    std::string m_search;
    _bool m_bRequestFocusSearch = false;

    /* Renmae */
    Engine::CGameObject* m_pRenameTarget = nullptr;
    std::string m_renameBuffer;
    bool m_bJustStartedRename = false;

    /* Context menu */
    Engine::CGameObject* m_pContextTarget = nullptr;

    /* Callbacks */
    std::function<void(Engine::CGameObject*)> m_fnPrimarySelectionChanged;
    std::function<void(const std::vector<Engine::CGameObject*>&)> m_fnSelectionChanged;

    /* Window */
    static constexpr const char* PAYLOAD_GO_PTR = "HIERARCHY_GO_PTR";

    /* Create Prefab*/
    std::filesystem::path m_PrototypePath;

public:
        static std::unique_ptr<CHierarchyPanel>Create(const std::string& strPanelName);
};

NS_END
