#pragma once

#include "resource.h"
#include "Ping.h"

#define WM_ICONNOTIFY (WM_APP + 1)

#define MAX_LOADSTRING 100

#define TITLE_BAR_HEIGHT 31
#define MENU_HEIGHT 27

#define DRAW_WIDTH 500
#define DRAW_HEIGHT 200

#define CLIENT_WIDTH DRAW_WIDTH
#define CLIENT_HEIGHT 300 + MENU_HEIGHT*2

#define ARRAY_LENGTH 20

#define UPDATE_LOOP_INTERVAL 30

class Visping {
public:
    Visping();
    ~Visping();

    // Register the window class and call methods for instantiating drawing resources
    HRESULT Initialize();

    // Process and dispatch messages
    void RunMessageLoop();

    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

private:
    static DWORD WINAPI UpdateThreadLoop(LPVOID lpParam);

    std::wstring getDisplayText();

    void updateWindowCentered(int px, int py);
    void saveWindowPosition();

    HANDLE hUpdateThread;

    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    void DiscardDeviceResources();

    // Draw content.
    HRESULT OnRender();

    // Resize the render target.
    void OnResize(
        UINT width,
        UINT height
    );

    // The windows procedure.
    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    static INT_PTR CALLBACK Server(
        HWND hwnd, 
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam
    );

    WCHAR szServerAddress[MAX_LOADSTRING];          // The server address string to ping
    WCHAR szServerAddressDefault[MAX_LOADSTRING];          // The server address string to ping
    WCHAR szPath[MAX_PATH];
    WCHAR szSettingsFilePath[MAX_PATH];
    
    HWND hwnd;
    HMENU hMenu;
    HMENU hNotifyIconMenu;

    RECT desktop;

    NOTIFYICONDATA nid;
    MENUITEMINFO showOnStartupMenuItemInfo;
    MENUITEMINFO centerWindowMenuItemInfo;

    Ping* pingingServer;

    // Delcare Pointers
    ID2D1Factory*                   pFactory;
    ID2D1HwndRenderTarget*          pHwndRenderTarget;

    ID2D1SolidColorBrush*           pBlackBrush;
    ID2D1SolidColorBrush*           pWhiteBrush;
    ID2D1SolidColorBrush*           pCyanBrush;
    ID2D1SolidColorBrush*           pYellowBrush;
    ID2D1SolidColorBrush*           pMagentaBrush;
    ID2D1LinearGradientBrush*       pLinearGradientBrush;

    IDWriteFactory*                 pWriteFactory;
    IDWriteTextFormat*              pWriteTextFormat;

    ID2D1GradientStopCollection*    pGradentStopCollection;
};