// Visping.cpp : Defines the entry point for the application.
//
#pragma comment(lib, "d2d1")
#pragma comment(lib, "Dwrite")

#ifndef UNICODE
#define UNICODE
#endif 

#include <string>

#include "framework.h"
#include "Visping.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND main_hwnd;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
//ATOM                MyRegisterClass(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
DWORD WINAPI        PingingThread(LPVOID lpParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.


    Direct2DApp App;

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VISPING, szWindowClass, MAX_LOADSTRING);

    // Perform application initialization:
    if (!App.InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VISPING));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: PingingThread()
//
//  PURPOSE: 
//
DWORD WINAPI PingingThread(LPVOID lpParam) {
    std::string server = "8.8.8.8";

    while (true) {
        ping_server(server);
        InvalidateRect(main_hwnd, NULL, TRUE);
    };

    return 0;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM Direct2DApp::MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = Direct2DApp::WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VISPING));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VISPING);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}



//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL Direct2DApp::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    // Flag Variable
    HRESULT hr(S_OK);

    hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr))
    {
        const DWORD DW_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

        RECT wr, desktop;
        wr.top = 0;
        wr.left = 0;
        wr.bottom = CLIENT_HEIGHT;
        wr.right = CLIENT_WIDTH;

        hInst = hInstance; // Store instance handle in our global variable

        MyRegisterClass(hInstance);

        m_hwnd = CreateWindowW(
            szWindowClass,
            szTitle,
            DW_STYLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            wr.right - wr.left,
            wr.bottom - wr.top,
            nullptr,
            nullptr,
            hInstance,
            nullptr
        );

        if (!m_hwnd)
        {
            return FALSE;
        }

        main_hwnd = m_hwnd;

        //AdjustWindowRect(&wr, DW_STYLE, FALSE);

        //GetClientRect(GetDesktopWindow(), &desktop);

        //SetWindowPos(m_hwnd, 0, (desktop.right - CLIENT_WIDTH) / 2, wr.top + 1, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);

        //LPVOID lparam(nullptr);

        //PingingThread(lparam);
        
        //DWORD PingingThreadID;
        //HANDLE PingingThreadHandle = CreateThread(0, 0, PingingThread, 0, 0, &PingingThreadID);
        //CloseHandle(PingingThreadHandle);
    }

    return TRUE;
}



//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK Direct2DApp::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

        Direct2DApp* pDirect2DApp = (Direct2DApp*)pcs->lpCreateParams;

        ::SetWindowLongPtr(
            hWnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pDirect2DApp)
        );

        result = 1;
    }
    else
    {
        
        Direct2DApp* pDirect2DApp = reinterpret_cast<Direct2DApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtr(
                hWnd,
                GWLP_USERDATA
            )));
         

        //Direct2DApp* pDirect2DApp(new Direct2DApp);

        bool wasHandled = false;

        if (pDirect2DApp)
        {
            switch (message)
            {

                case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pDirect2DApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hWnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_COMMAND:
                {
                    int wmId = LOWORD(wParam);
                    // Parse the menu selections:
                    switch (wmId)
                    {
                    case IDM_ABOUT:
                        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                        break;
                    case IDM_EXIT:
                        DestroyWindow(hWnd);
                        break;
                    default:
                        return DefWindowProc(hWnd, message, wParam, lParam);
                    }
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_PAINT:
                {
                    pDirect2DApp->OnRender();
                    ValidateRect(hWnd, NULL);
                }
                result = 0;
                wasHandled = true;
                break;

                case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;

            }
        }

        if (!wasHandled)
        {
            result = DefWindowProc(hWnd, message, wParam, lParam);
        }
    }

    return result;
}


// Message handler for about box.
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
