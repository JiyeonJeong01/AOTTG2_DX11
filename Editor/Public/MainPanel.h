#pragma once
#include "EditorPanel.h"
#include "SceneChange_Event.h"

NS_BEGIN(Engine)

class CScene;

NS_END

NS_BEGIN(Editor)

class CMainPanel final : public Editor::CEditorPanel
{
public:
    CMainPanel(const std::string& strPanelName);
    ~CMainPanel() override;

public :
    HRESULT Initialize() override;
    void Update() override;
    void Render() override;

    /* Panels to dock */
    void Add_Panel(std::unique_ptr<Editor::CEditorPanel> pPanel);

    /* SCENE operations */

    /* Optional state for UI */
    void Set_Scene_Path(const std::wstring& path) { m_scenePath = path; }
    void Set_Scene_Dirty(_bool bDirty) { m_bSceneDirty = bDirty; }
    void Set_Playing(_bool bPlaying) { m_bPlaying = bPlaying; }

    void On_SceneChanged(Engine::EVENT_DATA& event);

private:
    void Build_Default_Layout();
    void Draw_Dockspace();
    void Draw_MenuBar();
    void Draw_Toolbar();
    void Draw_Panels();

    void Draw_Menu_File();
    void Draw_Menu_Window();   /* optional */
    void Draw_Menu_Tools();    /* optional */

    void Request_Exit();

private:
    /* --- Panels --- */
    std::vector<std::unique_ptr<Editor::CEditorPanel>> m_panels;

    /* --- SCENE UI State --- */
    std::wstring m_scenePath;
    _bool m_bSceneDirty = false;
    _bool m_bBuiltLayer = false;
    _bool m_bShowExitPopup = false;

    class Engine::CScene* m_pCurScene = nullptr;

    /* --- Playe UI State --- */
    _bool m_bPlaying = false;
    _bool m_bSceneStarted = false;

    static constexpr const char* PANEL_HIERARCHY = "Hierarchy";
    static constexpr const char* PANEL_INSPECTOR = "Inspector";
    static constexpr const char* PANEL_CONSOLE = "Console";
    static constexpr const char* PANEL_PROJECT = "Project";
    static constexpr const char* PANEL_PROFILE = "Profile";
    static constexpr const char* PANEL_SCENE = "Scene";

public:
    static std::unique_ptr<CMainPanel> Create(const std::string& strPanelName);
};

NS_END
