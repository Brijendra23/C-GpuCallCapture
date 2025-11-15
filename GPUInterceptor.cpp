// GPUInterceptor.cpp : Modern rounded UI with owner-drawn buttons and up/down spinner.
// Paste the four parts in order to form the full file.
// Requires framework.h to include <windows.h> and resource identifiers used (IDI_GPUINTERCEPTOR, IDC_GPUINTERCEPTOR, IDI_SMALL, IDD_ABOUTBOX).
// Compile with comctl32.lib (pragma included).

#include "framework.h"
#include "GPUInterceptor.h"

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "gdi32.lib")

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

// Handles for controls (children are direct children of main window)
HWND hBtnStart = NULL, hBtnStop = NULL;
HWND hEditConcurrency = NULL, hSpinConcurrency = NULL;
HWND hBtnEnableTracing = NULL, hBtnExportTrace = NULL;
HWND hBtnPing = NULL, hBtnReset = NULL, hBtnClearCache = NULL, hBtnOpenLogs = NULL;
HWND hLblTitle = NULL, hLblConcurrency = NULL, hLblInst = NULL, hLblQA = NULL, hLblMetrics = NULL, hLblBandwidth = NULL, hLblPending = NULL;

HFONT hFontTitle = NULL;
HFONT hFontNormal = NULL;
HFONT hFontBtn = NULL;

// Panel metrics (slightly increased width)
const int PANEL_W = 520;   // increased from 450
const int PANEL_H = 620;
const int PANEL_RADIUS = 25; // rounded corners

// Prototypes
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void RepositionAll(HWND hWnd);
void DrawRoundedPanel(HDC hdc, RECT rcClient, RECT panelRect);
void FillRoundRect(HDC hdc, RECT r, int radius, COLORREF color);
void PaintRoundedButton(DRAWITEMSTRUCT* dis);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
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
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GPUINTERCEPTOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    // Use a neutral background for the window — panel is drawn manually.
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GPUINTERCEPTOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    // init common controls required for Up-Down spinner
    INITCOMMONCONTROLSEX ic{};
    ic.dwSize = sizeof(ic);
    ic.dwICC = ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&ic);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1200, 820, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return FALSE;

    // Create fonts (Segoe UI look)
    LOGFONTW lf{};
    lf.lfHeight = -18; // normal text ~18px
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    wcscpy_s(lf.lfFaceName, L"Segoe UI");
    hFontNormal = CreateFontIndirectW(&lf);

    lf.lfHeight = -24; lf.lfWeight = FW_BOLD;
    hFontTitle = CreateFontIndirectW(&lf);

    lf.lfHeight = -16; lf.lfWeight = FW_SEMIBOLD;
    hFontBtn = CreateFontIndirectW(&lf);

    // Create controls (children of main window). We'll position them later in RepositionAll.
    hLblTitle = CreateWindowW(L"STATIC", L"Runtime Controls", WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
    SendMessage(hLblTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

    // Owner-drawn buttons to get a modern look (we'll paint them in WM_DRAWITEM)
    hBtnStart = CreateWindowW(L"BUTTON", L"Start Capture", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 160, 44, hWnd, (HMENU)IDC_BTN_START_CAPTURE, hInst, NULL);
    SendMessage(hBtnStart, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hBtnStop = CreateWindowW(L"BUTTON", L"Stop Capture", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 160, 44, hWnd, (HMENU)IDC_BTN_STOP_CAPTURE, hInst, NULL);
    SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hLblConcurrency = CreateWindowW(L"STATIC", L"Concurrency (max in-flight requests)", WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
    SendMessage(hLblConcurrency, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    // Wider edit to avoid intersection with panel border
    hEditConcurrency = CreateWindowW(L"EDIT", L"4", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER | ES_LEFT,
        0, 0, 340, 36, hWnd, (HMENU)IDC_EDIT_CONCURRENCY, hInst, NULL);
    SendMessage(hEditConcurrency, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    // Up-Down spinner (we make it a bit taller to match new edit)
    hSpinConcurrency = CreateWindowExW(0, UPDOWN_CLASS, NULL,
        WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
        0, 0, 44, 36, hWnd, (HMENU)IDC_SPIN_CONCURRENCY, hInst, NULL);

    // Set buddy and range
    SendMessage(hSpinConcurrency, UDM_SETBUDDY, (WPARAM)hEditConcurrency, 0);
    SendMessage(hSpinConcurrency, UDM_SETRANGE, 0, MAKELPARAM(100, 1));
    SendMessage(hSpinConcurrency, UDM_SETPOS, 0, MAKELPARAM(4, 0));

    hLblInst = CreateWindowW(L"STATIC", L"Instrumentation", WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
    SendMessage(hLblInst, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    // Increase the width of Enable Tracing / Export Trace so they don't collide with border
    hBtnEnableTracing = CreateWindowW(L"BUTTON", L"Enable Tracing", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 200, 44, hWnd, (HMENU)IDC_BTN_ENABLE_TRACING, hInst, NULL);
    SendMessage(hBtnEnableTracing, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hBtnExportTrace = CreateWindowW(L"BUTTON", L"Export Trace", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 200, 44, hWnd, (HMENU)IDC_BTN_EXPORT_TRACE, hInst, NULL);
    SendMessage(hBtnExportTrace, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hLblQA = CreateWindowW(L"STATIC", L"Quick Actions", WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 0, 0, hWnd, NULL, hInst, NULL);
    SendMessage(hLblQA, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    hBtnPing = CreateWindowW(L"BUTTON", L"Ping GPU", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 200, 44, hWnd, (HMENU)IDC_BTN_PING_GPU, hInst, NULL);
    SendMessage(hBtnPing, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hBtnReset = CreateWindowW(L"BUTTON", L"Reset Remote Session", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 200, 44, hWnd, (HMENU)IDC_BTN_RESET_SESSION, hInst, NULL);
    SendMessage(hBtnReset, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hBtnClearCache = CreateWindowW(L"BUTTON", L"Clear Cache", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 200, 44, hWnd, (HMENU)IDC_BTN_CLEAR_CACHE, hInst, NULL);
    SendMessage(hBtnClearCache, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hBtnOpenLogs = CreateWindowW(L"BUTTON", L"Open Logs", WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 200, 44, hWnd, (HMENU)IDC_BTN_OPEN_LOGS, hInst, NULL);
    SendMessage(hBtnOpenLogs, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

    hLblMetrics = CreateWindowW(L"STATIC", L"Live Metrics", WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0, hWnd, NULL, hInst, NULL);
    SendMessage(hLblMetrics, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    hLblBandwidth = CreateWindowW(L"STATIC", L"Bandwidth: 0 MB/s", WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0, hWnd, NULL, hInst, NULL);
    SendMessage(hLblBandwidth, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    hLblPending = CreateWindowW(L"STATIC", L"Pending Requests: 0", WS_CHILD | WS_VISIBLE | SS_LEFT,
        0, 0, 0, 0, hWnd, NULL, hInst, NULL);
    SendMessage(hLblPending, WM_SETFONT, (WPARAM)hFontNormal, TRUE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Position now
    RepositionAll(hWnd);
    return TRUE;
}

// Helper to draw rounded rectangle filled
void FillRoundRect(HDC hdc, RECT r, int radius, COLORREF color)
{
    HBRUSH hBrush = CreateSolidBrush(color);
    HRGN rgn = CreateRoundRectRgn(r.left, r.top, r.right, r.bottom, radius, radius);
    SelectClipRgn(hdc, rgn);
    FillRect(hdc, &r, hBrush);
    SelectClipRgn(hdc, NULL);
    DeleteObject(rgn);
    DeleteObject(hBrush);
}

// Draw panel only — no section background bars (removed per your request)
void DrawRoundedPanel(HDC hdc, RECT rcClient, RECT panelRect)
{
    // Background shadow
    RECT shadow = { panelRect.left + 6, panelRect.top + 8,
                    panelRect.right + 6, panelRect.bottom + 8 };
    FillRoundRect(hdc, shadow, PANEL_RADIUS, RGB(220, 220, 220));

    // Panel background
    FillRoundRect(hdc, panelRect, PANEL_RADIUS, RGB(250, 250, 250));

    // Border
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, panelRect.left, panelRect.top, panelRect.right, panelRect.bottom,
        PANEL_RADIUS, PANEL_RADIUS);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

// Reposition all controls centered inside rounded panel
void RepositionAll(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);

    int px = (rc.right - PANEL_W) / 2;
    if (px < 10) px = 10;
    int py = 60;

    int x = px + 30;
    int y = py + 20;

    // Title
    SetWindowPos(hLblTitle, NULL, x, y, PANEL_W - 60, 30, SWP_NOZORDER);
    y += 50;

    // Start / Stop
    SetWindowPos(hBtnStart, NULL, x, y, 230, 44, SWP_NOZORDER);
    SetWindowPos(hBtnStop, NULL, x + 240, y, 230, 44, SWP_NOZORDER);
    y += 60;

    // Concurrency
    SetWindowPos(hLblConcurrency, NULL, x, y, PANEL_W - 60, 26, SWP_NOZORDER);
    y += 32;
    SetWindowPos(hEditConcurrency, NULL, x, y, 360, 40, SWP_NOZORDER);
    SetWindowPos(hSpinConcurrency, NULL, x + 360, y, 40, 40, SWP_NOZORDER);
    y += 55;

    // Instrumentation
    SetWindowPos(hLblInst, NULL, x, y, PANEL_W - 60, 26, SWP_NOZORDER);
    y += 32;
    SetWindowPos(hBtnEnableTracing, NULL, x, y, 230, 44, SWP_NOZORDER);
    SetWindowPos(hBtnExportTrace, NULL, x + 240, y, 230, 44, SWP_NOZORDER);
    y += 60;

    // Quick Actions
    SetWindowPos(hLblQA, NULL, x, y, PANEL_W - 60, 26, SWP_NOZORDER);
    y += 32;
    SetWindowPos(hBtnPing, NULL, x, y, 230, 44, SWP_NOZORDER);
    SetWindowPos(hBtnReset, NULL, x + 240, y, 230, 44, SWP_NOZORDER);
    y += 55;

    SetWindowPos(hBtnClearCache, NULL, x, y, 230, 44, SWP_NOZORDER);
    SetWindowPos(hBtnOpenLogs, NULL, x + 240, y, 230, 44, SWP_NOZORDER);
    y += 60;

    // Live Metrics
    SetWindowPos(hLblMetrics, NULL, x, y, PANEL_W - 60, 26, SWP_NOZORDER);
    y += 32;
    SetWindowPos(hLblBandwidth, NULL, x, y, PANEL_W - 60, 26, SWP_NOZORDER);
    y += 28;
    SetWindowPos(hLblPending, NULL, x, y, PANEL_W - 60, 26, SWP_NOZORDER);

    InvalidateRect(hWnd, NULL, TRUE);
}

// Draw rounded modern buttons
void PaintRoundedButton(DRAWITEMSTRUCT* dis)
{
    HDC hdc = dis->hDC;
    RECT rc = dis->rcItem;

    int id = dis->CtlID;

    BOOL isPressed = (dis->itemState & ODS_SELECTED) != 0;
    BOOL isDisabled = (dis->itemState & ODS_DISABLED) != 0;

    // Colors
    COLORREF bg, fg, border;

    if (id == IDC_BTN_START_CAPTURE)
    {
        bg = isPressed ? RGB(22, 130, 60) : RGB(30, 170, 95);
        fg = RGB(255, 255, 255);
        border = RGB(18, 120, 50);
    }
    else if (id == IDC_BTN_STOP_CAPTURE)
    {
        bg = isPressed ? RGB(185, 25, 25) : RGB(230, 40, 40);
        fg = RGB(255, 255, 255);
        border = RGB(170, 20, 20);
    }
    else
    {
        // For secondary buttons
        bg = isPressed ? RGB(235, 235, 235) : RGB(250, 250, 250);
        fg = RGB(30, 30, 30);
        border = RGB(210, 210, 210);
    }

    if (isDisabled)
    {
        fg = RGB(170, 170, 170);
        bg = RGB(240, 240, 240);
    }

    // Fill shape
    HBRUSH hBrush = CreateSolidBrush(bg);
    HRGN rgn = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, 10, 10);
    FillRgn(hdc, rgn, hBrush);

    // Border
    HPEN hPen = CreatePen(PS_SOLID, 1, border);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 10, 10);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);

    // Text
    TCHAR textBuf[200];
    GetWindowText(dis->hwndItem, textBuf, 200);

    HFONT hFont = (HFONT)SendMessage(dis->hwndItem, WM_GETFONT, 0, 0);
    HFONT oldFont = (HFONT)SelectObject(hdc, hFont);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, fg);

    DrawText(hdc, textBuf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);

    DeleteObject(hBrush);
    DeleteObject(rgn);
    DeleteObject(hPen);
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        RepositionAll(hWnd);
        return 0;

    case WM_SIZE:
        RepositionAll(hWnd);
        return 0;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;

        switch (dis->CtlID)
        {
        case IDC_BTN_START_CAPTURE:
        case IDC_BTN_STOP_CAPTURE:
        case IDC_BTN_ENABLE_TRACING:
        case IDC_BTN_EXPORT_TRACE:
        case IDC_BTN_PING_GPU:
        case IDC_BTN_RESET_SESSION:
        case IDC_BTN_CLEAR_CACHE:
        case IDC_BTN_OPEN_LOGS:
            PaintRoundedButton(dis);
            return TRUE;
        }
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BTN_START_CAPTURE:
            MessageBox(hWnd, L"Start Capture clicked", L"Info", MB_OK);
            break;

        case IDC_BTN_STOP_CAPTURE:
            MessageBox(hWnd, L"Stop Capture clicked", L"Info", MB_OK);
            break;

        case IDC_BTN_ENABLE_TRACING:
            MessageBox(hWnd, L"Enable Tracing clicked", L"Info", MB_OK);
            break;

        case IDC_BTN_EXPORT_TRACE:
            MessageBox(hWnd, L"Export Trace clicked", L"Info", MB_OK);
            break;

        case IDC_BTN_PING_GPU:
            MessageBox(hWnd, L"Ping GPU clicked", L"Info", MB_OK);
            break;

        case IDC_BTN_RESET_SESSION:
            MessageBox(hWnd, L"Reset Session clicked", L"Info", MB_OK);
            break;

        case IDC_BTN_CLEAR_CACHE:
            MessageBox(hWnd, L"Cache Cleared", L"Info", MB_OK);
            break;

        case IDC_BTN_OPEN_LOGS:
            MessageBox(hWnd, L"Opening Logs...", L"Info", MB_OK);
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        }
        return 0;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        // Panel rect centered
        int px = (rcClient.right - PANEL_W) / 2;
        if (px < 20) px = 20;

        int py = 40;

        RECT panelRect = { px, py, px + PANEL_W, py + PANEL_H };

        // ---- Draw main white rounded panel --------
        // Shadow
        RECT shadow = { panelRect.left + 6, panelRect.top + 6,
                        panelRect.right + 6, panelRect.bottom + 6 };

        FillRoundRect(hdc, shadow, PANEL_RADIUS, RGB(220, 220, 220));
        FillRoundRect(hdc, panelRect, PANEL_RADIUS, RGB(255, 255, 255));

        // Border
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        HPEN penOld = (HPEN)SelectObject(hdc, pen);
        HBRUSH brOld = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        RoundRect(hdc,
            panelRect.left, panelRect.top,
            panelRect.right, panelRect.bottom,
            PANEL_RADIUS, PANEL_RADIUS);

        SelectObject(hdc, penOld);
        SelectObject(hdc, brOld);
        DeleteObject(pen);

        // **IMPORTANT**  
        // You requested to REMOVE small background bars behind:
        // Runtime Controls, Concurrency, Instrumentation,
        // Quick Actions, Live Metrics, Bandwidth, Pending
        //
        // So we REMOVE all FillRect header backgrounds.
        // No header bars are drawn here.

        EndPaint(hWnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (hFontNormal) DeleteObject(hFontNormal);
        if (hFontTitle) DeleteObject(hFontTitle);
        if (hFontBtn) DeleteObject(hFontBtn);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// ---------------------- ABOUT DIALOG ----------------------
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
