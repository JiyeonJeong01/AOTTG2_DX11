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
#include "ResourcePanel.h"
#include "CinematicPanel.h"
#include "Scene.h"
#include "Event_System.h"
#include "magic_enum.hpp"
#include "Input_System.h"

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
    auto pScene = CScenePanel::Create(PANEL_SCENE, pHierarchy.get(), this);
    auto pResource = CResourcePanel::Create(PANEL_RESOURCE);
    auto pCinematic = CCinematicPanel::Create(PANEL_CINEMATIC, pHierarchy.get());

    Add_Panel(std::move(pConsole));
    Add_Panel(std::move(pHierarchy));
    Add_Panel(std::move(pInspector));
    Add_Panel(std::move(pProfile));
    Add_Panel(std::move(pScene));
    Add_Panel(std::move(pResource));
    Add_Panel(std::move(pCinematic));
    Add_Panel(std::move(pProject));

    const std::filesystem::path assetRootPath = ProjectConfig::PATH + ProjectConfig::ROOT;

    /* 씬 변경 시 이벤트 등록 */
    SYS_EVENT.Subscribe(EVENT_TYPE::On_Scene_Changed, &CMainPanel::On_SceneChanged, this);

    /* 기본 씬 파일 보장하며, GUID를 가져온다. */
    const Engine::ASSET_GUID ensureGUID = SYS_EDITOR.Ensure_DefaultScene();
    IF_TRUE_RETURN_MSG_BREAK(!ensureGUID.Is_Valid(), E_FAIL, "Default scene guid invalid");

    /* 기본 씬을 로드한다. */
    IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Open_EditScene(ensureGUID), E_FAIL,
        "Change_Scene(Default) failed");

    /* UI 캐시용 */
    const Engine::ASSET_RECORD* pRec = SYS_ASSET.Find(ensureGUID);
    if (pRec) m_scenePath = pRec->path.wstring();
    m_bSceneDirty = false;

    SYS_EDITOR.Load_SavedNav(L"../../Client/Bin/Assets/DataFiles/NavMesh.dat");

    m_eDebugDraw = DEBUG_DRAW::NONE;
    SYS_CORE.Set_DebugRender(m_eDebugDraw);

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

        IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Open_EditScene(newGuid), ,
            "Change_Scene(New) failed");

        m_scenePath = newPath.wstring();
        m_bSceneDirty = false;
    }

    if (ImGui::MenuItem("Open SCENE...", "Ctrl+O"))
    {
        /* 오픈할 씬 선택 -> 경로 저장 */
        const std::wstring pathW = Editor_Util::OpenFileDialog(
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

            IF_FAIL_RETURN_MSG_BREAK(SYS_CORE.Open_EditScene(outGUID), ,
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

    if (SYS_INPUT.Get_KeyDown(VK_F8))
    {
        const _float3 vPos = SYS_EDITOR.Get_Position();
        const _float3 vRot = SYS_EDITOR.Get_RotationEuler();

        char szBuffer[256]{};
        sprintf_s(szBuffer, "Pos : %.2f, %.2f, %.2f | Rot : %.2f, %.2f, %.2f",
            vPos.x, vPos.y, vPos.z,
            vRot.x, vRot.y, vRot.z);

        m_strCameraPreview = szBuffer;

        ImGui::SameLine();
    }

    if (!m_strCameraPreview.empty())
    {
        ImGui::TextUnformatted(m_strCameraPreview.c_str());
        ImGui::SameLine();
    }

    const _float button_size = 70.0f;
    const _float checkbox_size = 250.0f;
    const _float spacing = ImGui::GetStyle().ItemSpacing.x;
    const _float total_width = button_size * 3 + spacing * 2;

    float start_x = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f - checkbox_size;
    if (start_x > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);

    ImGui::SetNextItemWidth(150.f);
    if (ImGui::BeginCombo("##DebugRender", magic_enum::enum_name(m_eDebugDraw).data()))
    {
        const DEBUG_DRAW arrDebugDraw[] =
        {
            DEBUG_DRAW::NONE,
            DEBUG_DRAW::ALL,
            DEBUG_DRAW::SELECT,
            DEBUG_DRAW::ALL_GRID,
            DEBUG_DRAW::SELECT_GRID,
            DEBUG_DRAW::ALL_NAV,
            DEBUG_DRAW::SELECT_NAV,
            DEBUG_DRAW::NAV,
        };

        for (DEBUG_DRAW eDraw : arrDebugDraw)
        {
            const bool bSelected = (m_eDebugDraw == eDraw);
            const std::string_view svName = magic_enum::enum_name(eDraw);

            if (ImGui::Selectable(svName.data(), bSelected))
            {
                m_eDebugDraw = eDraw;
                SYS_CORE.Set_DebugRender(eDraw);
            }

            if (bSelected)
                ImGui::SetItemDefaultFocus();
        }

        ImGui::EndCombo();
    }

    ImGui::SameLine();


    /* Play / Pause / Step */
    {
        if (!m_bPlaying)
        {
            if (ImGui::Button("Play", ImVec2(button_size, 35)))
            {
                if (!m_bSceneStarted)
                {
                    SYS_EDITOR.Play();
                }
                m_bPlaying = true;
                SYS_EDITOR.Toggle_DebugCamera(false);
            }

            ImGui::SameLine(0, 20.0f);

            const bool bNavEdit = (m_ePickMode == EDITOR_PICK_MODE::NAV_EDIT);
            if (bNavEdit)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.f));

            if (ImGui::Button(bNavEdit ? "SAVE_NAV" : "EDIT_NAV", ImVec2( button_size + 10, 35)))
            {
                if (bNavEdit)
                {
                    SYS_EDITOR.Save_Nav();
                    m_ePickMode = EDITOR_PICK_MODE::NORMAL;
                }
                else
                {
                    m_ePickMode = EDITOR_PICK_MODE::NAV_EDIT;
                }
            }

            if (bNavEdit)
                ImGui::PopStyleColor();

            ImGui::SameLine();
        }
        else
        {
            if (ImGui::Button("Pause", ImVec2(button_size, 35)))
            {
                if (m_pCurScene)
                {
                    m_bPlaying = false;
                    SYS_EDITOR.Pause();
                }
            }
        }

        ImGui::SameLine();

        ImGui::BeginDisabled(m_bPlaying);
        if (ImGui::Button("Step", ImVec2(button_size, 35)))
        {
            SYS_EDITOR.Step(SYS_CORE.Compute_FrameDT());
        }
        ImGui::EndDisabled();

        ImGui::SameLine();

        ImGui::PushItemWidth(45.f);
        ImGui::DragFloat("##Speed", &g_fPlaySpeed, 0.005f, 0.1f, 100.f);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (ImGui::Button("Reload", ImVec2(button_size, 35)))
        {
            m_bPlaying = false;
            SYS_EDITOR.Pause();
            SYS_CORE.Restart();
        }

        ImGui::SameLine(0.f, 8.f);
        ImGui::TextDisabled("|");
        ImGui::SameLine(0.f, 8.f);

        if (m_bForceSceneView)
        {
            if (ImGui::Button("> Game", ImVec2(button_size + 15, 35)))
            {
                m_bForceSceneView = false;
                SYS_EDITOR.Toggle_DebugCamera(false);
            }
        }
        else
        {
            if (ImGui::Button("> Debug", ImVec2(button_size + 15, 35)))
            {
                m_bForceSceneView = true;
                SYS_EDITOR.Toggle_DebugCamera(true);
            }
        }
    }

    ImGui::EndChild();
}

void CMainPanel::On_SceneChanged(Engine::EVENT_DATA& event)
{
    SCENECHANGE_EVENT_DATA& onSceneChanged = SCAST(SCENECHANGE_EVENT_DATA&, event);
    m_pCurScene = onSceneChanged.m_pNewScene;
}

EDITOR_PICK_MODE CMainPanel::Get_NavMode() const
{
    return m_ePickMode;
}

void CMainPanel::Build_Default_Layout()
{
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

    m_bBuiltLayer = true;
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
