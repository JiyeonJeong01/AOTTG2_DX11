#define _EDITOR
#include "framework.h"
#include "Editor.h"
#include "MainApp.h"
#include "GUI_System.h"
#include "MainPanel.h"
#include "ProfilerPanel.h"

#include "Core_System.h"
#include "Event_System.h"

#include "WindowResize_Event.h"



#define MAX_LOADSTRING 100

// 전역 변수:
HWND g_hWnd;
HINSTANCE g_hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
FILE* debug;

constexpr float FRAME_DT = 1.f / 60.f;
constexpr float FIXED_DT = 0.02f;

_float4 g_vClearColor = { 0.18f, 0.18f, 0.18f, 1.0f };

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#ifdef _DEBUG
    //_CrtSetBreakAlloc(405);
#endif

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    unique_ptr<Client::CMainApp> pMainApp = { nullptr };

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EDITOR));

    MSG msg;

    ENGINE_DESC EngineDesc{};
    EngineDesc.eWinMode = WINMODE::WIN;
    EngineDesc.hWnd = g_hWnd;
    EngineDesc.hInst = g_hInst;
    EngineDesc.iViewportSize = {Client::g_iWinSizeX, Client::g_iWinSizeY };

    pMainApp = Client::CMainApp::Create(EngineDesc);
    if (nullptr == pMainApp)
        return FALSE;

    if (!CCore_System::GetInstancePtr())
    {
        MessageBox(nullptr, L"Core_System이 없습니다!", L"에러", MB_OK);
        return FALSE;
    }

    _float      fTimeAcc = {};
    _float      fFixedAcc = {};

    SYS_GUI.Initialize();
    const string strMain = "PANEL_MAIN";
    unique_ptr<Editor::CMainPanel> upMainPanel = Editor::CMainPanel::Create(strMain);
    

    // 기본 메시지 루프입니다:
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
                break;

            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        fTimeAcc += SYS_CORE.Compute_SystemDT();

        if (fTimeAcc >= FRAME_DT)
        {
            _float fDT = SYS_CORE.Compute_FrameDT();

            Editor::CProfilerPanel::CScope _update("Engine::Begin_Render");
            pMainApp->Update(fDT, APP_MODE::EDITOR_EDIT);
            upMainPanel->Update();

            /* --- Render --- */
            Editor::CProfilerPanel::CScope _render("Engine::Render");
            SYS_CORE.Bind_SceneRTV();
            _float4 g_vClearColor = { 0.1f, 0.8f, 0.8f, 1.0f };
            SYS_CORE.Clear_Scene_Buffers(&g_vClearColor);

            pMainApp->Render();

            SYS_CORE.Bind_DefaultRTV();
            _float4 k_vClearColor = { 0.18f, 0.18f, 0.18f, 1.0f };
            SYS_CORE.Clear_Default_Buffers(&k_vClearColor);
            SYS_GUI.Begin_Render();
            upMainPanel->Render();

            SYS_GUI.End_Render();

            SYS_CORE.Present();

            /* FIXED DT*/
            // pMainApp->Fixed_Update(FIXED_DT);

            fTimeAcc = 0.f;
        }
    }

    if (upMainPanel)
        upMainPanel.reset();

    SYS_GUI.DestroyInstance();
    SYS_CORE.DestroyInstance();

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    /* TODO NOTE WARN */
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    Client::g_iWinSizeX = SCAST(_uint, GetSystemMetrics(SM_CXSCREEN));
    Client::g_iWinSizeY = SCAST(_uint, GetSystemMetrics(SM_CYSCREEN));

    RECT rc = { 0, 0, SCAST(LONG, Client::g_iWinSizeX), SCAST(LONG, Client::g_iWinSizeY) };
    DWORD dwStyle = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rc, dwStyle, FALSE);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, dwStyle,
        CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    g_hWnd = hWnd;

    return TRUE;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return 0;


    switch (message)
    {
    case WM_SIZE:
    {
        const UINT w = LOWORD(lParam);
        const UINT h = HIWORD(lParam);

        if (wParam == SIZE_MINIMIZED)
            break;
        RESIZE_EVENT_DATA eData(w, h);
        SYS_EVENT.Trigger(eData);
    }
    break;
    case WM_CLOSE:
    {
        FreeConsole();
        DestroyWindow(hWnd);
        
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 메뉴 선택을 구문 분석합니다:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
