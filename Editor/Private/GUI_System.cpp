#include "GUI_System.h"

#include "Engine_Log.h"
#include "Core_System.h"

IMPLEMENT_SINGLETON(Editor::CGUI_System)

Editor::CGUI_System::CGUI_System()
    : m_pDevice(nullptr), m_pContext(nullptr)
{
}

Editor::CGUI_System::~CGUI_System()
{
}

HRESULT Editor::CGUI_System::Initialize()
{
    SYS_CORE.Share_GraphicDevice(&m_pDevice, &m_pContext);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    m_pGuiContext = ImGui::GetCurrentContext();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

    io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

    static constexpr const char* UI_FONT_PATH = "C:/Windows/Fonts/segoeui.ttf";
    io.Fonts->AddFontFromFileTTF(
        UI_FONT_PATH,
        25.0f,
        nullptr,
        io.Fonts->GetGlyphRangesKorean()
    );

    ImGui_ImplWin32_EnableDpiAwareness();

    if (!ImGui_ImplWin32_Init(g_hWnd))
        return E_FAIL;

    if (!ImGui_ImplDX11_Init(m_pDevice, m_pContext))
        return E_FAIL;

    /* --- ImGui style setting --- */
    Setup_ImGuiStyle();

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

void Editor::CGUI_System::Setup_ImGuiStyle()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // ===== Spacing & Shape =====
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.PopupRounding = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 3.0f;

    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ScrollbarSize = 14.0f;

    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    // ===== Base Colors (Unity gray) =====
    const ImVec4 bg_dark = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    const ImVec4 bg_mid = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    const ImVec4 bg_light = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);

    const ImVec4 bg_hover = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
    const ImVec4 bg_active = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);

    const ImVec4 text = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    const ImVec4 text_dim = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);

    // ===== Text =====
    style.Colors[ImGuiCol_Text] = text;
    style.Colors[ImGuiCol_TextDisabled] = text_dim;

    /* Docking Preview */
    style.Colors[ImGuiCol_DockingPreview] = ImVec4(0.40f, 0.40f, 0.40f, 0.60f);

    // ===== Windows =====
    style.Colors[ImGuiCol_WindowBg] = bg_dark;
    style.Colors[ImGuiCol_ChildBg] = bg_dark;
    style.Colors[ImGuiCol_PopupBg] = bg_mid;
    style.Colors[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);

    // ===== Frame (Input, Checkbox, Slider background) =====
    style.Colors[ImGuiCol_FrameBg] = bg_mid;
    style.Colors[ImGuiCol_FrameBgHovered] = bg_hover;
    style.Colors[ImGuiCol_FrameBgActive] = bg_active;

    // ===== Buttons =====
    style.Colors[ImGuiCol_Button] = bg_mid;
    style.Colors[ImGuiCol_ButtonHovered] = bg_hover;
    style.Colors[ImGuiCol_ButtonActive] = bg_active;

    // ===== Headers (Hierarchy selection etc.) =====
    style.Colors[ImGuiCol_Header] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = bg_hover;
    style.Colors[ImGuiCol_HeaderActive] = bg_active;

    // ===== Tabs =====
    style.Colors[ImGuiCol_Tab] = bg_dark;
    style.Colors[ImGuiCol_TabHovered] = bg_hover;
    style.Colors[ImGuiCol_TabActive] = bg_mid;
    style.Colors[ImGuiCol_TabUnfocused] = bg_dark;
    style.Colors[ImGuiCol_TabUnfocusedActive] = bg_mid;

    style.Colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);

    // ===== Title Bar =====
    style.Colors[ImGuiCol_TitleBg] = bg_dark;
    style.Colors[ImGuiCol_TitleBgActive] = bg_mid;
    style.Colors[ImGuiCol_TitleBgCollapsed] = bg_dark;

    // ===== Check / Slider =====
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);

    // ===== Scrollbar =====
    style.Colors[ImGuiCol_ScrollbarBg] = bg_dark;
    style.Colors[ImGuiCol_ScrollbarGrab] = bg_mid;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = bg_hover;
    style.Colors[ImGuiCol_ScrollbarGrabActive] = bg_active;
}
