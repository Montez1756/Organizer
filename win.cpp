#define RAYGUI_IMPLEMENTATION
#include <windows.h>
#include <shellapi.h>
#include "win.h"
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1

// Global variables for tray management
static NOTIFYICONDATA nid = {0};
static bool windowMinimizedToTray = false;
static HWND raylibWindowHandle = NULL;

// Forward declarations
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MinimizeToTray(void);
void RestoreFromTray(void);
void RemoveTrayIcon(void);

void tool(void* hWnd)
{
    // Initialize Raylib window
    
    // Get the native window handle
    raylibWindowHandle = (HWND)hWnd;
    
    // Set up our custom window procedure
    SetWindowLongPtr(raylibWindowHandle, GWLP_WNDPROC, (LONG_PTR)WindowProc);
    
    // Initialize tray icon structure
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = raylibWindowHandle;
    nid.uID = ID_TRAYICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    strcpy(nid.szTip, "Raylib Application");
    
    // Main game loop
    
    // Clean up
    // RemoveTrayIcon();
    
}

// Custom window procedure to handle tray messages
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_TRAYICON:
            if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
            {
                RestoreFromTray();
            }
            break;
            
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED && !windowMinimizedToTray)
            {
                MinimizeToTray();
            }
            break;
            
        case WM_CLOSE:
            RemoveTrayIcon();
            break;
            
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void MinimizeToTray(void)
{
    Shell_NotifyIcon(NIM_ADD, &nid);
    windowMinimizedToTray = true;
    ShowWindow(raylibWindowHandle, SW_HIDE);
}

void RestoreFromTray(void)
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
    windowMinimizedToTray = false;
    ShowWindow(raylibWindowHandle, SW_RESTORE);
    SetForegroundWindow(raylibWindowHandle);
}

void RemoveTrayIcon(void)
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
}