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
#include "resource.h"
#include <shlwapi.h> 
#pragma comment(lib, "Shlwapi.lib")

using namespace Microsoft::WRL;

// Global Variables:
HINSTANCE   hInst;
static const char* windowClass = "LSESender";

// Overridable commandline arguments
static std::wstring wv_initialUrl = L"https://www.google.com";
static std::string senderName = "Webview Sender";


// SPOUT
spoutDX                            sender;                 // Spout sender object
HWND                               g_hWnd = nullptr;       // Main window handle
HWND                               g_hWebView = nullptr;   // WebView2 child window handle
ComPtr<ICoreWebView2Controller>    g_webviewController;    // WebView2 controller
ComPtr<ICoreWebView2>              g_webViewWindow;        // CoreWebView2

unsigned char* g_pixelBuffer = nullptr; // Sending pixel buffer (RGBA)
unsigned int   g_SenderWidth = 640;    // Sender width
unsigned int   g_SenderHeight = 360;   // Sender height



// Forward declarations
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void                Render();


