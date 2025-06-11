#pragma once

#include <SDKDDKVer.h>


// SPOUT
#include "SpoutDX.h"

// WEBVIEW2
#include "WebView2.h"

// WINDOWS
#include <vector>
#include <windows.h>
#include <wrl.h>
#include <shellapi.h>
#include <string>

using namespace Microsoft::WRL;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE                         hInst;                  // current instance
WCHAR                             szTitle[MAX_LOADSTRING];      // The title bar text
WCHAR                             szWindowClass[MAX_LOADSTRING];// the main window class name

// SPOUT
spoutDX                            sender;                 // Spout sender object
HWND                               g_hWnd = nullptr;       // Main window handle
HWND                               g_hWebView = nullptr;   // WebView2 child window handle
ComPtr<ICoreWebView2Controller>    g_webviewController;    // WebView2 controller
ComPtr<ICoreWebView2>              g_webViewWindow;        // CoreWebView2

unsigned char* g_pixelBuffer = nullptr; // Sending pixel buffer (RGBA)
unsigned int   g_SenderWidth = 640;    // Sender width
unsigned int   g_SenderHeight = 360;   // Sender height

// Now a mutable std::wstring so we can override it from argv[1]
static std::wstring wv_initialUrl = L"https://www.google.com";

// Forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                Render();


