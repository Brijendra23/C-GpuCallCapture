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
#define IDC_SPIN_CONCURRENCY      3004
#define IDC_BTN_ENABLE_TRACING    3005
#define IDC_BTN_EXPORT_TRACE      3006
#define IDC_BTN_PING_GPU          3007
#define IDC_BTN_RESET_SESSION     3008
#define IDC_BTN_CLEAR_CACHE       3009
#define IDC_BTN_OPEN_LOGS         3010

// Globals
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Prototypes
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

// Each control handle stored for repositioning
HWND hLblTitle;
HWND hBtnStart, hBtnStop;

HWND hLblConcurrency, hEditConcurrency, hSpinConcurrency;

HWND hLblInst, hBtnEnableTracing, hBtnExportTrace;

HWND hLblQA, hBtnPing, hBtnReset, hBtnClearCache, hBtnOpenLogs;

HWND hLblMetrics, hLblBandwidth, hLblPending;


// ------------------------------------------------------------------
// WINMAIN
// ------------------------------------------------------------------
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GPUINTERCEPTOR, szWindowClass, MAX_LOADSTRING);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    MSG msg;
    HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GPUINTERCEPTOR));

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccel, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}


// ------------------------------------------------------------------
// REGISTER WINDOW CLASS
// ------------------------------------------------------------------
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GPUINTERCEPTOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GPUINTERCEPTOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


// ------------------------------------------------------------------
// INIT INSTANCE
// ------------------------------------------------------------------
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    // Enable Up/Down
    INITCOMMONCONTROLSEX ic{};
    ic.dwSize = sizeof(ic);
    ic.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&ic);

    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1000, 900,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}


// ------------------------------------------------------------------
// CENTERING FUNCTION
// ------------------------------------------------------------------
void CenterControl(HWND parent, HWND ctrl, int width)
{
    RECT rc;
    GetClientRect(parent, &rc);
    int x = (rc.right - width) / 2;

    RECT c;
    GetWindowRect(ctrl, &c);
    int h = c.bottom - c.top;

    SetWindowPos(ctrl, NULL, x, 0, width, h, SWP_NOSIZE | SWP_NOZORDER | SWP_NOMOVE);
}


// ------------------------------------------------------------------
// RESIZE ALL CONTROLS DYNAMICALLY
// ------------------------------------------------------------------
void RecenterAll(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    int xCenter = rc.right / 2 - 150;

    int y = 20;

    // Each control moved
    SetWindowPos(hLblTitle, NULL, xCenter, y, 300, 30, 0);
    y += 40;

    SetWindowPos(hBtnStart, NULL, xCenter, y, 140, 40, 0);
    SetWindowPos(hBtnStop, NULL, xCenter + 150, y, 140, 40, 0);
    y += 60;

    SetWindowPos(hLblConcurrency, NULL, xCenter, y, 300, 20, 0);
    y += 25;

    SetWindowPos(hEditConcurrency, NULL, xCenter, y, 260, 25, 0);
    SetWindowPos(hSpinConcurrency, NULL, xCenter + 260, y, 40, 25, 0);
    y += 50;

    SetWindowPos(hLblInst, NULL, xCenter, y, 300, 20, 0);
    y += 25;

    SetWindowPos(hBtnEnableTracing, NULL, xCenter, y, 140, 35, 0);
    SetWindowPos(hBtnExportTrace, NULL, xCenter + 150, y, 140, 35, 0);
    y += 55;

    SetWindowPos(hLblQA, NULL, xCenter, y, 300, 20, 0);
    y += 25;

    SetWindowPos(hBtnPing, NULL, xCenter, y, 140, 35, 0);
    SetWindowPos(hBtnReset, NULL, xCenter + 150, y, 140, 35, 0);
    y += 45;

    SetWindowPos(hBtnClearCache, NULL, xCenter, y, 140, 35, 0);
    SetWindowPos(hBtnOpenLogs, NULL, xCenter + 150, y, 140, 35, 0);
    y += 50;

    SetWindowPos(hLblMetrics, NULL, xCenter, y, 300, 20, 0);
    y += 25;

    SetWindowPos(hLblBandwidth, NULL, xCenter, y, 300, 20, 0);
    y += 20;

    SetWindowPos(hLblPending, NULL, xCenter, y, 300, 20, 0);
}


// ------------------------------------------------------------------
// WINDOW PROCEDURE
// ------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        int x = 50, y = 20;

        // Title
        hLblTitle = CreateWindowW(L"STATIC", L"Runtime Controls",
            WS_VISIBLE | WS_CHILD, x, y, 300, 30, hWnd, NULL, NULL, NULL);
        y += 40;

        // Buttons
        hBtnStart = CreateWindowW(L"BUTTON", L"Start Capture",
            WS_VISIBLE | WS_CHILD, x, y, 140, 40, hWnd, (HMENU)IDC_BTN_START_CAPTURE, NULL, NULL);
        hBtnStop = CreateWindowW(L"BUTTON", L"Stop Capture",
            WS_VISIBLE | WS_CHILD, x + 150, y, 140, 40, hWnd, (HMENU)IDC_BTN_STOP_CAPTURE, NULL, NULL);
        y += 60;

        // Concurrency
        hLblConcurrency = CreateWindowW(L"STATIC", L"Concurrency (max in-flight requests)",
            WS_VISIBLE | WS_CHILD, x, y, 300, 20, hWnd, NULL, NULL, NULL);
        y += 25;

        hEditConcurrency = CreateWindowW(L"EDIT", L"4",
            WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
            x, y, 260, 25, hWnd, (HMENU)IDC_EDIT_CONCURRENCY, NULL, NULL);

        hSpinConcurrency = CreateWindowW(
            UPDOWN_CLASS, NULL,
            WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
            x + 260, y, 40, 25,
            hWnd, (HMENU)IDC_SPIN_CONCURRENCY, NULL, NULL);

        SendMessage(hSpinConcurrency, UDM_SETBUDDY, (WPARAM)hEditConcurrency, 0);
        SendMessage(hSpinConcurrency, UDM_SETRANGE, 0, MAKELPARAM(100, 1));
        SendMessage(hSpinConcurrency, UDM_SETPOS, 0, MAKELPARAM(4, 0));

        y += 50;

        // Instrumentation
        hLblInst = CreateWindowW(L"STATIC", L"Instrumentation",
            WS_VISIBLE | WS_CHILD, x, y, 300, 20, hWnd, NULL, NULL, NULL);
        y += 25;

        hBtnEnableTracing = CreateWindowW(L"BUTTON", L"Enable Tracing",
            WS_VISIBLE | WS_CHILD, x, y, 140, 35, hWnd, (HMENU)IDC_BTN_ENABLE_TRACING, NULL, NULL);
        hBtnExportTrace = CreateWindowW(L"BUTTON", L"Export Trace",
            WS_VISIBLE | WS_CHILD, x + 150, y, 140, 35, hWnd, (HMENU)IDC_BTN_EXPORT_TRACE, NULL, NULL);
        y += 55;

        // Quick Actions
        hLblQA = CreateWindowW(L"STATIC", L"Quick Actions",
            WS_VISIBLE | WS_CHILD, x, y, 300, 20, hWnd, NULL, NULL, NULL);
        y += 25;

        hBtnPing = CreateWindowW(L"BUTTON", L"Ping GPU",
            WS_VISIBLE | WS_CHILD, x, y, 140, 35, hWnd, (HMENU)IDC_BTN_PING_GPU, NULL, NULL);
        hBtnReset = CreateWindowW(L"BUTTON", L"Reset Remote Session",
            WS_VISIBLE | WS_CHILD, x + 150, y, 140, 35, hWnd, (HMENU)IDC_BTN_RESET_SESSION, NULL, NULL);
        y += 45;

        hBtnClearCache = CreateWindowW(L"BUTTON", L"Clear Cache",
            WS_VISIBLE | WS_CHILD, x, y, 140, 35, hWnd, (HMENU)IDC_BTN_CLEAR_CACHE, NULL, NULL);
        hBtnOpenLogs = CreateWindowW(L"BUTTON", L"Open Logs",
            WS_VISIBLE | WS_CHILD, x + 150, y, 140, 35, hWnd, (HMENU)IDC_BTN_OPEN_LOGS, NULL, NULL);
        y += 50;

        // Metrics
        hLblMetrics = CreateWindowW(L"STATIC", L"Live Metrics",
            WS_VISIBLE | WS_CHILD, x, y, 300, 20, hWnd, NULL, NULL, NULL);
        y += 25;

        hLblBandwidth = CreateWindowW(L"STATIC", L"Bandwidth: 0 MB/s",
            WS_VISIBLE | WS_CHILD, x, y, 300, 20, hWnd, NULL, NULL, NULL);
        y += 20;

        hLblPending = CreateWindowW(L"STATIC", L"Pending Requests: 0",
            WS_VISIBLE | WS_CHILD, x, y, 300, 20, hWnd, NULL, NULL, NULL);

        // Center everything once initially
        RecenterAll(hWnd);
    }
    break;

    case WM_SIZE:
        RecenterAll(hWnd);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START_CAPTURE: MessageBox(hWnd, L"Starting...", L"Info", 0); break;
        case IDC_BTN_STOP_CAPTURE: MessageBox(hWnd, L"Stopping...", L"Info", 0); break;
        case IDC_BTN_ENABLE_TRACING: MessageBox(hWnd, L"Tracing Enabled", L"Info", 0); break;
        case IDC_BTN_EXPORT_TRACE: MessageBox(hWnd, L"Trace Exported", L"Info", 0); break;
        case IDC_BTN_PING_GPU: MessageBox(hWnd, L"Pinging GPU", L"Info", 0); break;
        case IDC_BTN_RESET_SESSION: MessageBox(hWnd, L"Session Reset", L"Info", 0); break;
        case IDC_BTN_CLEAR_CACHE: MessageBox(hWnd, L"Cache Cleared", L"Info", 0); break;
        case IDC_BTN_OPEN_LOGS: MessageBox(hWnd, L"Opening Logs", L"Info", 0); break;
        case IDM_EXIT: DestroyWindow(hWnd); break;
        case IDM_ABOUT: DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About); break;
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


// About dialog
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    if (message == WM_COMMAND)
    {
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
    }
    return FALSE;
}
