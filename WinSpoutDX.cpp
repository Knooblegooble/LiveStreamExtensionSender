// WebView2SpoutDX.cpp
#include "WinSpoutDX.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    // Initialize COM for WebView2 (STA)
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        MessageBox(nullptr, "Failed to initialize COM", "LSESender", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // --- NEW: parse command line for a URL override ---
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        if (argc > 1 && argv[1][0] != L'\0') {
            wv_initialUrl = argv[1];
        }
        LocalFree(argv);
    }
    // ---------------------------------------------------

    // Initialize global strings
    // LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    // LoadStringW(hInstance, IDC_WINSPOUTDX, szWindowClass, MAX_LOADSTRING);
	wcscpy_s(szTitle, MAX_LOADSTRING, L"LSEStreamMonitor");
	wcscpy_s(szWindowClass, MAX_LOADSTRING, L"STREAMMONITOR");

    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    // Determine initial client size for buffer
    RECT rcClient;
    GetClientRect(g_hWnd, &rcClient);
    g_SenderWidth = rcClient.right - rcClient.left;
    g_SenderHeight = rcClient.bottom - rcClient.top;
    g_pixelBuffer = new unsigned char[g_SenderWidth * g_SenderHeight * 4];

    // SPOUT: Initialize DirectX 11
    if (!sender.OpenDirectX11())
        return FALSE;
    sender.SetSenderName("Webview Sender");
    SetWindowTextA(g_hWnd, sender.GetName());

    // Create WebView2 environment and controller
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [](HRESULT envResult, ICoreWebView2Environment* env) -> HRESULT {
                if (SUCCEEDED(envResult) && env) {
                    env->CreateCoreWebView2Controller(
                        g_hWnd,
                        Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [](HRESULT ctrlResult, ICoreWebView2Controller* controller) -> HRESULT {
                                if (SUCCEEDED(ctrlResult) && controller) {
                                    g_webviewController = controller;
                                    g_webviewController->get_CoreWebView2(&g_webViewWindow);
                                    RECT bounds;
                                    GetClientRect(g_hWnd, &bounds);
                                    g_webviewController->put_Bounds(bounds);
                                    g_webviewController->get_ParentWindow(&g_hWebView);
                                    // Use the (possibly overridden) URL here
                                    g_webViewWindow->Navigate(wv_initialUrl.c_str());
                                }
                                return S_OK;
                            }).Get());
                }
                return S_OK;
            }).Get());

    // Main message loop:
    MSG msg = { 0 };
    while (WM_QUIT != msg.message) {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            Render();
        }
    }

    // Clean up
    delete[] g_pixelBuffer;
    sender.ReleaseSender();
    sender.CloseDirectX11();
    CoUninitialize();

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
	wcex.hIcon   = LoadIcon(nullptr, IDI_APPLICATION);
	wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = CreateHatchBrush(HS_DIAGCROSS, RGB(192, 192, 192));
    wcex.lpszClassName = szWindowClass;

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    RECT rc = { 0, 0, (LONG)g_SenderWidth, (LONG)g_SenderHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

    HWND hWnd = CreateWindowW(
        szWindowClass, szTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
        return FALSE;

    // Center window
    RECT winRect, workArea;
    GetWindowRect(hWnd, &winRect);
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    int x = workArea.left + (workArea.right - workArea.left - (winRect.right - winRect.left)) / 2;
    int y = workArea.top + (workArea.bottom - workArea.top - (winRect.bottom - winRect.top)) / 2;
    MoveWindow(hWnd, x, y, winRect.right - winRect.left, winRect.bottom - winRect.top, FALSE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    g_hWnd = hWnd;
    return TRUE;
}

//
//  FUNCTION: Render()
//  PURPOSE:  Captures the WebView2 content into our pixel buffer and sends via Spout
//
void Render()
{
    InvalidateRect(g_hWnd, NULL, FALSE);
    UpdateWindow(g_hWnd);

    sender.spoutcopy.ClearAlpha(g_pixelBuffer, g_SenderWidth, g_SenderHeight, 255);
    sender.SendImage(g_pixelBuffer, g_SenderWidth, g_SenderHeight);
    sender.HoldFps(60);
}

//
//  FUNCTION: WndProc(...)
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        if (!g_hWebView) break;
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            RECT rcClient; GetClientRect(hWnd, &rcClient);
            UINT width = rcClient.right - rcClient.left;
            UINT height = rcClient.bottom - rcClient.top;
            if (width != g_SenderWidth || height != g_SenderHeight) {
                g_SenderWidth = width; g_SenderHeight = height;
                delete[] g_pixelBuffer;
                g_pixelBuffer = new unsigned char[width * height * 4];
                g_webviewController->put_Bounds(rcClient);
            }
            HDC hdcMem = CreateCompatibleDC(hdc);
            HBITMAP hBmp = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP hOld = (HBITMAP)SelectObject(hdcMem, hBmp);
            PrintWindow(g_hWebView, hdcMem, PW_RENDERFULLCONTENT);
            GetBitmapBits(hBmp, width * height * 4, g_pixelBuffer);
            SelectObject(hdcMem, hOld);
            DeleteObject(hBmp);
            DeleteDC(hdcMem);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_WINDOWPOSCHANGING:
    {
        WINDOWPOS* p = reinterpret_cast<WINDOWPOS*>(lParam);
        p->hwndInsertAfter = HWND_BOTTOM;
        p->flags |= SWP_NOACTIVATE;
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
