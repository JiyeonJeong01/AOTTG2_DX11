//#pragma once
//#include "EditorPanel.h"
//
//NS_BEGIN(Engine)
//class CGameObject;
//class CInspectorPanel;
//NS_END
//
//NS_BEGIN(Editor)
//
//class CHierarchyPanel final : public CEditorPanel
//{
//public:
//    CHierarchyPanel();
//    ~CHierarchyPanel() override = default;
//
//    void Init() override;
//    void Update() override;
//    void RenderUI() override;
//
//    void Bind_Inspector(Engine::CInspectorPanel* pInspector);
//
//    // Selection API
//    Engine::CGameObject* Get_Primary_Selection() const;
//    const std::vector<Engine::CGameObject*>& Get_Selection() const { return m_Selected; }
//
//    void Set_Selection_Single(Engine::CGameObject* pObj);
//    void Add_Selection(Engine::CGameObject* pObj);
//    void Remove_Selection(Engine::CGameObject* pObj);
//    void Clear_Selection();
//
//    _bool Is_Selected(Engine::CGameObject* pObj) const;
//
//    // Scene Object Sync
//    void Add_Object(Engine::CGameObject* pObj);
//    void Remove_Object(Engine::CGameObject* pObj);
//    void Clear_Objects();
//
//private:
//    /* ---------- UI ---------- */
//    void Draw_Toolbar();
//    void Draw_Object_List();
//    void Draw_Create_Object_Popup();
//    void Draw_Object_Context_Menu(Engine::CGameObject* pObj);
//
//private:
//    /* ---------- Input ---------- */
//    void Handle_Input_Shortcuts();
//    void Handle_Keyboard_Navigation();
//
//    _int Get_Current_Selected_Index() const;
//    void Move_Selection(_int iDirection, _bool bAdditive);
//
//    void Focus_To_Object(Engine::CGameObject* pObj); // 더블클릭/Enter 공용
//
//private:
//    /* ---------- Utilities ---------- */
//    _bool Pass_Search_Filter(Engine::CGameObject* pObj) const;
//    Engine::CGameObject* Ensure_Unique_Name(Engine::CGameObject* pObj);
//
//    void Sort_If_Dirty();
//    void Sync_Inspector_Selection();
//
//private:
//    Engine::CInspectorPanel* m_pInspector = nullptr;
//
//    std::vector<Engine::CGameObject*> m_SceneObjects;
//    std::vector<Engine::CGameObject*> m_Selected;
//    std::vector<Engine::CGameObject*> m_Clipboard;
//
//    char  m_SearchBuf[128]{};
//    char  m_CreateSearchBuf[128]{};
//
//    _float m_KeyRepeatTimer = 0.f;
//    _float m_KeyRepeatDelay = 0.12f;
//
//    _bool  m_bSortDirty = true;
//
//    std::queue<Engine::CGameObject*> m_PendingDelete;
//};
//
//NS_END
