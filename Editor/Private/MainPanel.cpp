#include "MainPanel.h"
#include "Engine_Log.h"

#include "ConsolePanel.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "ProjectPanel.h"
#include "ProfilerPanel.h"
#include "ScenePanel.h"

NS_BEGIN(Editor)

CMainPanel::CMainPanel(const std::string& strPanelName)
    : CEditorPanel(strPanelName)
{
}

CMainPanel::~CMainPanel()
{
}

HRESULT CMainPanel::Initialize()
{
    auto pConsole = CConsolePanel::Create(PANEL_CONSOLE);
    auto pHierarchy = CHierarchyPanel::Create(PANEL_HIERARCHY);
    auto pProject = CProjectPanel::Create(PANEL_PROJECT);
    auto pInspector = CInspectorPanel::Create(PANEL_INSPECTOR, pHierarchy.get(), pProject.get());
    auto pProfile = CProfilerPanel::Create(PANEL_PROFILE);
    auto pScene = CScenePanel::Create(PANEL_SCENE);

    Add_Panel(std::move(pConsole));
    Add_Panel(std::move(pHierarchy));
    Add_Panel(std::move(pProject));
    Add_Panel(std::move(pInspector));
    Add_Panel(std::move(pProfile));
    Add_Panel(std::move(pScene));

    const std::filesystem::path assetRootPath = ProjectConfig::PATH + ProjectConfig::ROOT;

    return S_OK;
}

void CMainPanel::Update()
{
    if (!m_bOpen)
        return;

    for (const auto& upPanel : m_panels)
    {
        CEditorPanel* pPanel = upPanel.get();
        if (!pPanel)
        {
            _DEBUG_ERROR_BREAK("CMainPanel Update failed : panel is nullptr");
            continue;
        }
        pPanel->Update();
    }
}

void CMainPanel::Add_Panel(std::unique_ptr<Editor::CEditorPanel> pPanel)
{
    if (!pPanel)
    {
        _DEBUG_WARN("Add_Panel failed : pPanel is nullptr");
        return;
    }
    m_panels.push_back(std::move(pPanel));
}

void CMainPanel::Render()
{
    if (!m_bOpen)
        return;

    /* Fullscreen host window for DockSpace */
    ImGuiWindowFlags flags;
    {
        flags = ImGuiWindowFlags_NoDocking |
                ImGuiWindowFlags_MenuBar |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoNavFocus;
    }

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

    if (!ImGui::Begin(m_strPanelName.c_str(), (bool*)&m_bOpen, flags))
    {
        ImGui::End();
        ImGui::PopStyleVar(3);
        return;
    }

    ImGui::PopStyleVar(3);

    Draw_MenuBar();
    Draw_Toolbar();
    Draw_Dockspace();

    if (!m_bBuiltLayer)
        Build_Default_Layout();

    /* After Draw_DockSpace() : ImGui-Docking is order-dependent */
    Draw_Panels();

    ImGui::End();
}

void CMainPanel::Draw_MenuBar()
{
    if (!ImGui::BeginMenuBar())
        return;

    if (ImGui::BeginMenu("File"))
    {
        Draw_Menu_File();
        ImGui::EndMenu();
    }

    /* Toggle panel on/off */
    if (ImGui::BeginMenu("Window"))
    {
        Draw_Menu_Window();
        ImGui::EndMenu();
    }

    /* TODO : Tools menu to be added later */
    // if (ImGui::BeginMenu("Tools")) { Draw_Menu_Tools(); ImGui::EndMenu(); }

    /* Right-side : Display the current scene */
    {
        /* Place the txet about 360px from the right edge */
        _float fWidth = ImGui::CalcTextSize("SCENE: ").x;
        (void)fWidth;
        ImGui::SameLine(ImGui::GetWindowWidth() - 360.f);

        std::string sceneLabel = "SCENE: ";
        if (m_scenePath.empty())
            sceneLabel += "(Untitled)";
        else
        {
            sceneLabel += "[loaded]";
        }

        if (m_bSceneDirty)
            sceneLabel += " *";

        ImGui::TextDisabled("%s", sceneLabel.c_str());
    }

    ImGui::EndMenuBar();
}

void CMainPanel::Draw_Menu_File()
{
    if (ImGui::MenuItem("New SCENE", "Ctrl+N"))
    {
        if (m_fnNewScene)
            m_fnNewScene();
        m_scenePath.clear();
        m_bSceneDirty = false;
    }

    if (ImGui::MenuItem("Open SCENE...", "Ctrl+O"))
    {
        const std::wstring path = OpenFileDialog(L"SCENE Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0\0");
        if (!path.empty())
        {
            if (m_fnOpenScene)
                m_fnOpenScene(path);
            m_scenePath = path;
            m_bSceneDirty = false;
        }
    }

    ImGui::Separator();

    // Save: if no path is set, behave like Save As.
    if (ImGui::MenuItem("Save SCENE", "Ctrl+S"))
    {
        if (!m_scenePath.empty())
        {
            if (m_fnSaveScene)
                m_fnSaveScene();
            m_bSceneDirty = false;
        }
        else
        {
            const std::wstring path = OpenFileDialog(L"SCENE Files (*.scene)\0*.scene\0\0");
            if (!path.empty())
            {
                if (m_fnSaveAsScene)
                    m_fnSaveAsScene(path);
                m_scenePath = path;
                m_bSceneDirty = false;
            }
        }
    }

    if (ImGui::MenuItem("Save As SCENE...", "Ctrl+Shift+S"))
    {
        const std::wstring path = OpenFileDialog(L"SCENE Files (*.scene)\0*.scene\0\0");
        if (!path.empty())
        {
            if (m_fnSaveAsScene)
                m_fnSaveAsScene(path);
            m_scenePath = path;
            m_bSceneDirty = false;
        }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Exit", "Alt+F4"))
    {
        if (m_fnExit) m_fnExit();
    }
}

void CMainPanel::Draw_Menu_Window()
{
    /* Panel on/off */
    for (const auto& upPanel  : m_panels)
    {
        CEditorPanel* pPanel = upPanel.get();
        if (!pPanel) continue;

        bool bOpen = (bool)pPanel->IsOpen();
        if (ImGui::MenuItem(pPanel->GetTitle().c_str(), nullptr, &bOpen))
            pPanel->SetOpen(bOpen);
    }
}

void CMainPanel::Draw_Menu_Tools()
{

}

void CMainPanel::Draw_Toolbar()
{
    ImGui::BeginChild("##MainToolbar", ImVec2(0, 34.f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    const _float button_size = 60.0f;
    const _float spacing = ImGui::GetStyle().ItemSpacing.x;
    const _float total_width = button_size * 3 + spacing * 2;

    float start_x = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;
    if (start_x > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);

    /* Play / Stop / Step */
    {
        ImGui::BeginDisabled(m_bPlaying);
        if (ImGui::Button("Play", ImVec2(button_size, 26)))
        {
            if (m_fnPlay)
                m_fnPlay();
            m_bPlaying = true;
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(!m_bPlaying);
        if (ImGui::Button("Stop", ImVec2(button_size, 26)))
        {
            if (m_fnStop)
                m_fnStop();
            m_bPlaying = false;
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(!m_bPlaying);
        if (ImGui::Button("Step", ImVec2(button_size, 26)))
        {
            if (m_fnStep)
                m_fnStep();
        }
        ImGui::EndDisabled();
    }

    ImGui::EndChild();
}

void CMainPanel::Build_Default_Layout()
{
    /* TODO : =======================================================*/
    /* TODO : =============== After learning RTV ====================*/
    /* TODO : =======================================================*/
    //ImGuiID dockspace_id = ImGui::GetID(m_strPanelName.c_str());

    //ImGui::DockBuilderRemoveNode(dockspace_id);
    //ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    //ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    //ImGuiID dock_main = dockspace_id;

    //ImGuiID dock_left = 0;
    //ImGuiID dock_right = 0;
    //ImGuiID dock_bottom = 0;
    //ImGuiID dock_center = 0;

    ///* Inspector : right-side */
    //dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, nullptr, &dock_main);

    ///* Hierarchy : left-side */
    //dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.20f, nullptr, &dock_main);

    ///* Console : bottom */
    //dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.28f, nullptr, &dock_main);

    ///* Center : scene-game view */
    ////dock_center = dock_main;

    //ImGui::DockBuilderDockWindow(PANEL_HIERARCHY, dock_left);
    //ImGui::DockBuilderDockWindow(PANEL_INSPECTOR, dock_right);
    //ImGui::DockBuilderDockWindow(PANEL_CONSOLE, dock_bottom);
    ////ImGui::DockBuilderDockWindow("SCENE", dock_center); 

    //ImGui::DockBuilderFinish(dockspace_id);

    //m_bBuiltLayer = true;
}

void CMainPanel::Draw_Dockspace()
{
    ImGuiID dockspace_id = ImGui::GetID("##MainDockSpace");
    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dock_flags);
}

void CMainPanel::Draw_Panels()
{
    for (const auto& upPanel : m_panels)
    {
        CEditorPanel* pPanel = upPanel.get();

        if (!pPanel)
        {
            _DEBUG_ERROR_BREAK("CMainPanel DrawPanels failed : panel is nullptr");
            continue;
        }
        if (!pPanel->IsOpen())
            continue;
        pPanel->Render();
    }
}

std::unique_ptr<CMainPanel> CMainPanel::Create(const std::string& strPanelName)
{
    auto pInstance = std::make_unique<CMainPanel>(strPanelName);
    if (FAILED(pInstance->Initialize()))
    {
        _DEBUG_ERROR_BREAK("CMainPanel Create failed");
        return nullptr;
    }
    return pInstance;
}

NS_END
