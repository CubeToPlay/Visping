#pragma once

#include "resource.h"
#include "Ping.h"

#define MAX_LOADSTRING 100

#define MENU_HEIGHT 27

#define DRAW_WIDTH 500
#define DRAW_HEIGHT 200

#define CLIENT_WIDTH 500
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

private:
    static DWORD WINAPI UpdateThreadLoop(LPVOID lpParam);

    std::wstring getDisplayText();

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

    static LRESULT CALLBACK Server(
        HWND hwnd, 
        UINT message, 
        WPARAM wParam, 
        LPARAM lParam
    );

    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
    WCHAR szServerAddress[MAX_LOADSTRING];          // The server address string to ping
    NOTIFYICONDATA nid;

    HWND hwnd;

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