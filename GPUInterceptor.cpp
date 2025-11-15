// GPUInterceptor.cpp : Defines the entry point for the application.
//
#include "framework.h"
#include "GPUInterceptor.h"
// Enable Up-Down Control
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#define MAX_LOADSTRING 100

// Control IDs
#define IDC_BTN_START_CAPTURE     3001
#define IDC_BTN_STOP_CAPTURE      3002
#define IDC_EDIT_CONCURRENCY      3003
#define IDC_BTN_ENABLE_TRACING    3004
#define IDC_BTN_EXPORT_TRACE      3005
#define IDC_BTN_PING_GPU          3006
#define IDC_BTN_RESET_SESSION     3007
#define IDC_BTN_CLEAR_CACHE       3008
#define IDC_BTN_OPEN_LOGS         3009

// Global Variables
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Function Prototypes
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GPUINTERCEPTOR, szWindowClass, MAX_LOADSTRING);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GPUINTERCEPTOR));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GPUINTERCEPTOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GPUINTERCEPTOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    //  Required for Up-Down Control 
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_CREATE:
    {
        RECT rc;
        GetClientRect(hWnd, &rc);

        int centerX = (rc.right - rc.left) / 2 - 150;
        int y = 20;

        CreateWindowW(L"STATIC", L"Runtime Controls",
            WS_VISIBLE | WS_CHILD,
            centerX, y, 300, 30,
            hWnd, NULL, NULL, NULL);
        y += 40;

        CreateWindowW(L"BUTTON", L"Start Capture",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX, y, 140, 40,
            hWnd, (HMENU)IDC_BTN_START_CAPTURE, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Stop Capture",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX + 150, y, 140, 40,
            hWnd, (HMENU)IDC_BTN_STOP_CAPTURE, NULL, NULL);
        y += 60;

        // -------------------------------
        // Concurrency
        // -------------------------------
        CreateWindowW(L"STATIC", L"Concurrency (max in-flight requests)",
            WS_VISIBLE | WS_CHILD,
            centerX, y, 300, 20,
            hWnd, NULL, NULL, NULL);
        y += 25;

        // EDIT + UpDown
        HWND hConcurrency = CreateWindowW(
            L"EDIT", L"4",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
            centerX, y, 260, 25,
            hWnd, (HMENU)IDC_EDIT_CONCURRENCY, NULL, NULL);

        HWND hSpin = CreateWindowW(
            UPDOWN_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
            centerX + 260, y, 40, 25,
            hWnd, NULL, NULL, NULL);

        SendMessage(hSpin, UDM_SETBUDDY, (WPARAM)hConcurrency, 0);
        SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(100, 1));
        SendMessage(hSpin, UDM_SETPOS, 0, MAKELPARAM(4, 0));

        y += 50;

        // Instrumentation Section
        CreateWindowW(L"STATIC", L"Instrumentation",
            WS_VISIBLE | WS_CHILD,
            centerX, y, 300, 20,
            hWnd, NULL, NULL, NULL);
        y += 25;

        CreateWindowW(L"BUTTON", L"Enable Tracing",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX, y, 140, 35,
            hWnd, (HMENU)IDC_BTN_ENABLE_TRACING, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Export Trace",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX + 150, y, 140, 35,
            hWnd, (HMENU)IDC_BTN_EXPORT_TRACE, NULL, NULL);
        y += 55;

        // Quick Actions Section
        CreateWindowW(L"STATIC", L"Quick Actions",
            WS_VISIBLE | WS_CHILD,
            centerX, y, 300, 20,
            hWnd, NULL, NULL, NULL);
        y += 25;

        CreateWindowW(L"BUTTON", L"Ping GPU",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX, y, 140, 35,
            hWnd, (HMENU)IDC_BTN_PING_GPU, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Reset Remote Session",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX + 150, y, 140, 35,
            hWnd, (HMENU)IDC_BTN_RESET_SESSION, NULL, NULL);
        y += 45;

        CreateWindowW(L"BUTTON", L"Clear Cache",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX, y, 140, 35,
            hWnd, (HMENU)IDC_BTN_CLEAR_CACHE, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Open Logs",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            centerX + 150, y, 140, 35,
            hWnd, (HMENU)IDC_BTN_OPEN_LOGS, NULL, NULL);

        y += 50;

        CreateWindowW(L"STATIC", L"Live Metrics",
            WS_VISIBLE | WS_CHILD,
            centerX, y, 300, 20,
            hWnd, NULL, NULL, NULL);
        y += 25;

        CreateWindowW(L"STATIC", L"Bandwidth: 0 MB/s",
            WS_VISIBLE | WS_CHILD,
            centerX, y, 300, 20,
            hWnd, NULL, NULL, NULL);
        y += 20;

        CreateWindowW(L"STATIC", L"Pending Requests: 0",
            WS_VISIBLE | WS_CHILD,
            centerX, y, 300, 20,
            hWnd, NULL, NULL, NULL);
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START_CAPTURE:
            MessageBox(hWnd, L"Starting Capture...", L"Info", MB_OK);
            break;

        case IDC_BTN_STOP_CAPTURE:
            MessageBox(hWnd, L"Stopping Capture...", L"Info", MB_OK);
            break;

        case IDC_BTN_ENABLE_TRACING:
            MessageBox(hWnd, L"Tracing Enabled", L"Instrumentation", MB_OK);
            break;

        case IDC_BTN_EXPORT_TRACE:
            MessageBox(hWnd, L"Trace Exported", L"Instrumentation", MB_OK);
            break;

        case IDC_BTN_PING_GPU:
            MessageBox(hWnd, L"Pinging GPU...", L"Quick Action", MB_OK);
            break;

        case IDC_BTN_RESET_SESSION:
            MessageBox(hWnd, L"Session Reset", L"Quick Action", MB_OK);
            break;

        case IDC_BTN_CLEAR_CACHE:
            MessageBox(hWnd, L"Cache Cleared", L"Quick Action", MB_OK);
            break;

        case IDC_BTN_OPEN_LOGS:
            MessageBox(hWnd, L"Opening Logs...", L"Quick Action", MB_OK);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
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
