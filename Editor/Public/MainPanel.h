#pragma once
#include "EditorPanel.h"

NS_BEGIN(Editor)

class CMainPanel final : public Editor::CEditorPanel
{
public:
    CMainPanel(const std::string& strPanelName);
    ~CMainPanel() override = default;

    HRESULT Initialize() override;
    void Update() override;
    void Render() override;

    /* Panels to dock */
    void Add_Panel(Editor::CEditorPanel* pPanel);

    /* Scene operations */
    /* TODO : Hook these to Engine layer */
    void Set_On_New_Scene(std::function<void()> fn) { m_fnNewScene = std::move(fn); }
    void Set_On_Open_Scene(std::function<void(const std::wstring&)> fn) { m_fnOpenScene = std::move(fn); }
    void Set_On_Save_Scene(std::function<void()> fn) { m_fnSaveScene = std::move(fn); }
    void Set_On_SaveAs_Scene(std::function<void(const std::wstring&)> fn) { m_fnSaveAsScene = std::move(fn); }
    void Set_On_Exit(std::function<void()> fn) { m_fnExit = std::move(fn); }

    /* Play controls */
    void Set_On_Play(std::function<void()> fn) { m_fnPlay = std::move(fn); }
    void Set_On_Stop(std::function<void()> fn) { m_fnStop = std::move(fn); }
    void Set_On_Step(std::function<void()> fn) { m_fnStep = std::move(fn); }

    /* Optional state for UI */
    void Set_Scene_Path(const std::wstring& path) { m_scenePath = path; }
    void Set_Scene_Dirty(_bool bDirty) { m_bSceneDirty = bDirty; }
    void Set_Playing(_bool bPlaying) { m_bPlaying = bPlaying; }

private:
    void Build_Default_Layout();
    void Draw_Dockspace();
    void Draw_MenuBar();
    void Draw_Toolbar();
    void Draw_Panels();

    void Draw_Menu_File();
    void Draw_Menu_Window();   /* optional */
    void Draw_Menu_Tools();    /* optional */

private:
    /* --- Panels --- */
    std::vector<CEditorPanel*> m_panels;

    /* --- Scene UI State --- */
    std::wstring m_scenePath;
    _bool m_bSceneDirty = false;
    _bool m_bBuiltLayer = false;

    /* --- Playe UI State --- */
    bool m_bPlaying = false;

    /* --- Callbacks --- */
    std::function<void()> m_fnNewScene;
    std::function<void(const std::wstring&)> m_fnOpenScene;
    std::function<void()> m_fnSaveScene;
    std::function<void(const std::wstring&)> m_fnSaveAsScene;
    std::function<void()> m_fnExit;

    std::function<void()> m_fnPlay;
    std::function<void()> m_fnStop;
    std::function<void()> m_fnStep;

    static constexpr const char* PANEL_HIERARCHY = "Hierarchy";
    static constexpr const char* PANEL_INSPECTOR = "Inspector";
    static constexpr const char* PANEL_CONSOLE = "Console";

public:
    static CMainPanel* Create(const std::string& strPanelName);
private:
    void Free() override;
};

NS_END
