#include "GUI_System.h"

#include "Engine_Log.h"
#include "GameInstance.h"

IMPLEMENT_SINGLETON(Editor::CGUI_System)

Editor::CGUI_System::CGUI_System()
    : m_pDevice(nullptr), m_pContext(nullptr)
{
}

HRESULT Editor::CGUI_System::Ready_System()
{
    CGameInstance::GetInstance()->Share_GraphicDevice(&m_pDevice, &m_pContext);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    m_pGuiContext = ImGui::GetCurrentContext();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

    static constexpr const char* UI_FONT_PATH = "C:/Windows/Fonts/segoeui.ttf";
    io.Fonts->AddFontFromFileTTF(
        UI_FONT_PATH,
        18.0f,
        nullptr,
        io.Fonts->GetGlyphRangesKorean()
    );

    if (!ImGui_ImplWin32_Init(g_hWnd))
        return E_FAIL;

    if (!ImGui_ImplDX11_Init(m_pDevice, m_pContext))
        return E_FAIL;

    {
        /* --- ImGui style setting --- */
    }

    return S_OK;
}

void Editor::CGUI_System::Update()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Editor::CGUI_System::Render_GUI()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void Editor::CGUI_System::Free()
{
    CBase::Free();
}

