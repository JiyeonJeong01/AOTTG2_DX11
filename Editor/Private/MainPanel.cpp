#include "MainPanel.h"
#include "Engine_Log.h"

#include "Core_System.h"
#include "ConsolePanel.h"
#include "Asset_Registry.h"
#include "Editor_System.h"
#include "Editor_Util.h"
#include "HierarchyPanel.h"
#include "InspectorPanel.h"
#include "ProjectPanel.h"
#include "ProfilerPanel.h"
#include "ScenePanel.h"
#include "Scene.h"
#include "Event_System.h"

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
    auto pScene = CScenePanel::Create(PANEL_SCENE, pHierarchy.get());

    Add_Panel(std::move(pConsole));
    Add_Panel(std::move(pHierarchy));
    Add_Panel(std::move(pProject));
    Add_Panel(std::move(pInspector));
    Add_Panel(std::move(pProfile));
    Add_Panel(std::move(pScene));

    const std::filesystem::path assetRootPath = ProjectConfig::PATH + ProjectConfig::ROOT;

    /* 씬 변경 시 이벤트 등록 */
    SYS_EVENT.Subscribe(EVENT_TYPE::On_Scene_Changed, &CMainPanel::On_SceneChanged, this);

    /* 기본 씬 파일 보장하며, GUID를 가져온다. */
    const Engine::ASSET_GUID ensureGUID = SYS_EDITOR.Ensure_DefaultScene();
    IF_TRUE_RETURN_MSG_BREAK(!ensureGUID.Is_Valid(), E_FAIL, "Default scene guid invalid");

    /* 기본 씬을 로드한다. */
    IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Change_Scene(ensureGUID, APP_MODE::EDITOR_EDIT), E_FAIL,
        "Change_Scene(Default) failed");

    /* UI 캐시용 */
    const Engine::ASSET_RECORD* pRec = SYS_ASSET.Find(ensureGUID);
    if (pRec) m_scenePath = pRec->path.wstring();
    m_bSceneDirty = false;

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
            if (m_pCurScene)
                sceneLabel += m_pCurScene->Get_Label();
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

    if (m_bShowExitPopup)
    {
        if (ImGui::BeginPopupModal("ExitConfirmPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Do you want to save before exiting?");
            ImGui::Separator();

            /* YES : Save and Exit */
            if (ImGui::Button("Yes", ImVec2(120, 0)))
            {
                CScene* pScene = SYS_CORE.Get_CurrentScene();
                IF_NULL_RETURN_MSG_BREAK(pScene, , "Exit failed: current scene is null.");

                const ASSET_GUID& tGUID = pScene->Get_GUID();
                const ASSET_RECORD* pRec = tGUID.Is_Valid() ? SYS_ASSET.Find(tGUID) : nullptr;

                if (pRec && !pRec->path.empty())
                {
                    IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Save_CurrentScene(pRec->path), , "Save before exit failed.");
                }
                else
                {
                    /* Save As fallback */
                    const std::wstring pathW = Editor_Util::SaveFileDialog(
                        L"SCENE Files (*.scene)\0*.scene\0\0",
                        Editor_Util::Get_SceneRoot().wstring().c_str()
                    );

                    IF_TRUE_RETURN_MSG_BREAK(pathW.empty(), , "Exit cancelled: Save As aborted.");
                    IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Save_CurrentScene(pathW), , "Save As before exit failed.");
                }

                m_bShowExitPopup = false;
                m_bSceneDirty = false;
                ImGui::CloseCurrentPopup();

                Request_Exit();
            }

            ImGui::SameLine();

            /* NO : Exit without saving */
            if (ImGui::Button("No", ImVec2(120, 0)))
            {
                m_bSceneDirty = false;
                m_bShowExitPopup = false;
                ImGui::CloseCurrentPopup();
                Request_Exit();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                m_bShowExitPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    ImGui::EndMenuBar();
}

void CMainPanel::Draw_Menu_File()
{
    std::wstring sceneFolder = Editor_Util::Get_SceneRoot().wstring();

    CScene* pScene = SYS_CORE.Get_CurrentScene();
    const ASSET_GUID& curGuid = pScene ? pScene->Get_GUID(): ASSET_GUID{};
    const ASSET_RECORD* pRec = (curGuid.Is_Valid()) ? SYS_ASSET.Find(curGuid) : nullptr;

    /* cur save path; */
    std::wstring curScenePathW{};
    if (pRec)
        curScenePathW = pRec->path.wstring();

    if (ImGui::MenuItem("New SCENE", "Ctrl+N"))
    {
        std::filesystem::path newPath;
        const ASSET_GUID newGuid = SYS_EDITOR.Create_NewScene_Asset(&newPath);
        IF_TRUE_RETURN_MSG_BREAK(!newGuid.Is_Valid(), , "Create_NewScene_Asset failed");

        IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Change_Scene(newGuid, APP_MODE::EDITOR_EDIT), ,
            "Change_Scene(New) failed");

        m_scenePath = newPath.wstring();
        m_bSceneDirty = false;
    }

    if (ImGui::MenuItem("Open SCENE...", "Ctrl+O"))
    {
        /* 오픈할 씬 선택 -> 경로 저장 */
        const std::wstring pathW = Editor_Util::SaveFileDialog(
            L"SCENE Files (*.scene)\0*.scene\0All Files (*.*)\0*.*\0\0",
            sceneFolder.c_str()
        );

        if (!pathW.empty())
        {
            std::filesystem::path path(pathW);

            /* GUID 찾기 */
            ASSET_GUID outGUID{};
            if (!SYS_ASSET.Try_Get_GUID(path, outGUID) || !outGUID.Is_Valid())
            {
                /* Registry에 없으면 등록 시도 */
                IF_TRUE_RETURN_MSG_BREAK(!SYS_ASSET.Register_File_Asset(path, ASSET_TYPE::SCENE, ASSET_GUID{}), ,
                    "Open Scene failed: Register_File_Asset failed.");

                /* 등록 후 다시 GUID를 얻는다 */
                (void)SYS_ASSET.Try_Get_GUID(path, outGUID);
                IF_TRUE_RETURN_MSG_BREAK(!outGUID.Is_Valid(), , "Open Scene failed: GUID invalid after register.");
            }


            IF_TRUE_RETURN_MSG_BREAK(!outGUID.Is_Valid(), , "Open Scene failed: GUID invalid.");

            IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Change_Scene(outGUID, APP_MODE::EDITOR_EDIT), ,
                "Open Scene failed: Change_Scene failed.");

            m_scenePath = pathW;
            m_bSceneDirty = false;
        }
    }

    ImGui::Separator();

    /* Save the asset at the path if GUID is valid and CAsset_Registry has the path. */
    if (ImGui::MenuItem("Save SCENE", "Ctrl+S"))
    {
        if (pRec && !pRec->path.empty())
        {
            IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Save_CurrentScene(pRec->path), , "Save scene failed");
            m_bSceneDirty = false;

            /* UI cache (optional) */
            m_scenePath = pRec->path.wstring();
        }
        else
        {
            /* Fallback to SAVE AS */
            const std::wstring path = Editor_Util::SaveFileDialog(
                L"SCENE Files (*.scene)\0*.scene\0\0",
                sceneFolder.c_str()
            );

            if (!path.empty())
            {
                /* SaveAs saves the asset to the specified path and :
                 * - Generates a New() GUID if none exits
                 * - Registers/Update the GUID->path mapping in CAsset_Registry
                 */
                IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Save_CurrentScene(path), , "Save scene failed");
                m_bSceneDirty = false;

                /* UI cache (optional) */
                m_scenePath = path;
            }
        }
    }

    if (ImGui::MenuItem("Save As SCENE...", "Ctrl+Shift+S"))
    {
        const std::wstring pathW = Editor_Util::SaveFileDialog(
            L"SCENE Files (*.scene)\0*.scene\0\0",
            sceneFolder.c_str()
        );

        if (!pathW.empty())
        {
            std::filesystem::path path(pathW);

            CScene* pScene = SYS_CORE.Get_CurrentScene();

            IF_NULL_RETURN_MSG_BREAK(pScene, , "Save As failed: current scene is null.");

            /* 새 파일의 GUID 발급 */
            ASSET_GUID oldGuid = pScene->Get_GUID();
            ASSET_GUID newGuid = ASSET_GUID::New_GUID();

            IF_TRUE_RETURN_MSG_BREAK(!newGuid.Is_Valid(), , "Save As failed: new GUID invalid.");

            /* 임시로 GUID 교체 */
            pScene->Set_GUID(newGuid);

            if (FAILED(SYS_CORE.Save_CurrentScene(path)))
            {
                /* 실패 시 원복 */
                pScene->Set_GUID(oldGuid);
                _DEBUG_ERROR_BREAK("Save As failed: Save_CurrentScene failed.");
                return;
            }

            /* UI 캐시 갱신 */
            m_scenePath = pathW;
            m_bSceneDirty = false;
        }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Exit", "Alt+F4"))
    {
        if (m_bSceneDirty)
        {
            m_bShowExitPopup = true;
            ImGui::OpenPopup("ExitConfirmPopup");
        }
        else
        {
            Request_Exit();
        }
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

void CMainPanel::Request_Exit()
{
    HWND hWnd = (HWND)ImGui::GetMainViewport()->PlatformHandleRaw;
    IF_NULL_RETURN_MSG_BREAK(hWnd, , "Request_Exit failed: hwnd null");

    PostMessage(hWnd, WM_CLOSE, 0, 0);
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

    /* Play / Pause / Step */
    {
        ImGui::BeginDisabled(m_bPlaying);
        if (ImGui::Button("Play", ImVec2(button_size, 26)))
        {
            if (!m_bSceneStarted)
            {
                SYS_EDITOR.Play();
            }
            m_bPlaying = true;
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(!m_bPlaying);
        if (ImGui::Button("Pause", ImVec2(button_size, 26)))
        {
            if (m_pCurScene)
            {
                m_bPlaying = false;
                SYS_EDITOR.Pause();
            }
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::BeginDisabled(m_bPlaying);
        if (ImGui::Button("Step", ImVec2(button_size, 26)))
        {
            SYS_EDITOR.Step(SYS_CORE.Compute_FrameDT());
        }
        ImGui::EndDisabled();
    }

    ImGui::EndChild();
}

void CMainPanel::On_SceneChanged(Engine::EVENT_DATA& event)
{
    SCENECHANGE_EVENT_DATA& onSceneChanged = SCAST(SCENECHANGE_EVENT_DATA&, event);
    m_pCurScene = onSceneChanged.m_pNewScene;
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

    IF_FAIL_RETURN_MSG_BREAK(pInstance->Initialize(), nullptr, "CMainPanel Create failed");
    return pInstance;
}

NS_END
