#include "framework.h"
#include "Game.h"
#include "MainApp.h"

#include "Core_System.h"
#include "Event_System.h"
#include "WindowResize_Event.h"
#include "GameInstance.h"
#include "CRender_System.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HWND g_hWnd;
HINSTANCE g_hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
FILE* debug;

constexpr float FRAME_DT = 1.f / 60.f;
constexpr float FIXED_DT = 0.02f;

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
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    std::unique_ptr<Client::CMainApp> pMainApp = { nullptr };

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GAME, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GAME));

    MSG msg;

    ENGINE_DESC EngineDesc{};
    EngineDesc.eWinMode = WINMODE::WIN;
    EngineDesc.hWnd = g_hWnd;
    EngineDesc.hInst = g_hInst;
    EngineDesc.iViewportSize = {Client::g_iWinSizeX, Client::g_iWinSizeY }; /* 고정 해상도 1920 * 1080 */

    pMainApp = Client::CMainApp::Create(EngineDesc);
    if (nullptr == pMainApp)
        return FALSE;

    if (!CCore_System::GetInstancePtr())
    {
        MessageBox(nullptr, L"SYS_CORE가 없습니다!", L"에러", MB_OK);
        return FALSE;
    }

    /* 렌더러는 항상 Scene RT에 그린다. 에디터의 ScenePanel이 하던 준비를 여기서 한다 (고정 해상도). */
    {
        const _uint iW = Client::g_iWinSizeX, iH = Client::g_iWinSizeY;

        /* 에디터는 Scene 이미지를 ImGui 패널에 띄우며 tSceneView(화면상 위치 · 크기)를 채운다. Game은 창 전체가 SceneView */
        UI_GLOBAL tUI = SYS_RENDER.Get_UI_Global();
        tUI.vViewport = { (_float)iW, (_float)iH };
        tUI.tSceneView.vSize = { (_float)iW, (_float)iH };
        Math::Store(tUI.matView, XMMatrixIdentity());
        Math::Store(tUI.matProj, XMMatrixOrthographicOffCenterLH(0.f, (_float)iW, (_float)iH, 0.f, 0.f, 1.f));
        SYS_RENDER.Set_UI_Global(tUI);

        SYS_CORE.Ready_SceneRenderTarget(iW, iH);
        SYS_CORE.Ready_DeferredRenderTargets(iW, iH);
    }

    if (FAILED(GAME_INSTANCE.SetUp_Game()))
    {
        MessageBox(nullptr, L"시작 씬을 열지 못했습니다. Game/Bin/Config/GameConfig.json을 확인하세요.", L"에러", MB_OK);
        return FALSE;
    }

    _bool bStaticShadowCached = false;

    _float      fTimeAcc = {};
    _float      fFixedAcc = {};

    // 기본 메시지 루프입니다. 
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
            continue;
        }

        const _float sysDT = SYS_CORE.Compute_SystemDT();
        fTimeAcc += sysDT;

        if (fTimeAcc >= FRAME_DT)
        {
            {   /* 마우스 좌표 변환용 : 창이 움직여도 클라이언트 영역의 화면 위치를 따라간다 */
                POINT ptOrigin{ 0, 0 };
                ClientToScreen(g_hWnd, &ptOrigin);
                UI_GLOBAL tUI = SYS_RENDER.Get_UI_Global();
                tUI.tSceneView.vScreenPos = { (_float)ptOrigin.x, (_float)ptOrigin.y };
                SYS_RENDER.Set_UI_Global(tUI);
            }

            const _float fDT = SYS_CORE.Compute_FrameDT();
            pMainApp->Update(fDT, APP_MODE::GAME_PLAY);

            /* --- Render : Scene RT에 그린 뒤 백버퍼로 복사 --- */
            SYS_CORE.Bind_SceneRTV();

            _float4 vClearColor = { 0.1f, 0.8f, 0.8f, 1.0f };
            SYS_CORE.Clear_Scene_Buffers(&vClearColor);

            pMainApp->Render();

            SYS_CORE.Copy_Scene_To_BackBuffer();
            SYS_CORE.Present();

            /* 정적 그림자는 첫 프레임에 한 번 그린 뒤 캐시 (에디터는 Play 시점에 같은 처리) */
            if (!bStaticShadowCached)
            {
                SYS_RENDER.Rendered_StaticShadow();
                bStaticShadowCached = true;
            }

            /* FIXED DT
            fFixedAcc += fDT;
            if (fFixedAcc >= FIXED_DT)
            {
                pMainApp->Fixed_Update(FIXED_DT);
                fFixedAcc -= FIXED_DT;
            }
            */

            fTimeAcc = 0.f; /* TODO : fTimeAcc -= FRAME_DT 가 프레임 드랍에는 더 강해서 전환 고려  */
        }
    }

    pMainApp.reset(); /* 디바이스 참조 해제 + Core 파괴 */

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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GAME);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    /* 창 생성 전에 DPI 인식을 정한다 (에디터와 동일). 안 하면 고해상도 모니터에서 창이 확대 · 흐려진다. */
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

    /* 고정 해상도 : 크기 조절 불가. 백버퍼 = Scene RT 크기여야 Copy_Scene_To_BackBuffer가 동작한다 */
    const DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    RECT rc = { 0, 0, SCAST(LONG, Client::g_iWinSizeX), SCAST(LONG, Client::g_iWinSizeY) };
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    /* WM_SIZE는 전달하지 않는다 : 백버퍼를 고정 해상도로 유지 */
    case WM_CREATE:
    {
        AllocConsole();

        _tfreopen_s(&debug, _T("CONOUT$"), _T("w"), stdout);
        _tfreopen_s(&debug, _T("CONOUT$"), _T("r"), stdin);
        _tfreopen_s(&debug, _T("CONOUT$"), _T("w"), stderr);

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
