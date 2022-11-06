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
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT OnRender();
HRESULT CreateDeviceIndependentResources();
void DiscardDeviceResources();

void OnResize(UINT width, UINT height);


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

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VISPING, szWindowClass, MAX_LOADSTRING);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
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
    static int val;

    std::string server = "8.8.8.8";

    while (true) {
        ping_server(server);
        //insert(val);
        //val++;
        InvalidateRect(m_hwnd, NULL, TRUE);
    };

    return 0;
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
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

        AdjustWindowRect(&wr, DW_STYLE, FALSE);

        GetClientRect(GetDesktopWindow(), &desktop);

        SetWindowPos(m_hwnd, 0, (desktop.right - CLIENT_WIDTH) / 2, wr.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);

        //LPVOID lparam(nullptr);

        //PingingThread(lparam);
        
        DWORD PingingThreadID;
        HANDLE PingingThreadHandle = CreateThread(0, 0, PingingThread, 0, 0, &PingingThreadID);
        CloseHandle(PingingThreadHandle);
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    switch (message)
    {

    case WM_CREATE:
    {
    }
    break;

    case WM_SIZE:
    {
        UINT width = LOWORD(lParam);
        UINT height = HIWORD(lParam);
        OnResize(width, height);
    }
    break;

    case WM_DISPLAYCHANGE:
    {
        InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_PAINT:
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;

        OnRender();
        ValidateRect(hWnd, NULL);
    }
    break;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
    }
    break;

    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VISPING));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_VISPING);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

void DiscardDeviceResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pDirect2dFactory);

    SafeRelease(&m_pBlackBrush);
    SafeRelease(&m_pWhiteBrush);
    SafeRelease(&m_pCyanBrush);
    SafeRelease(&m_pYellowBrush);
    SafeRelease(&m_pMagentaBrush);
    SafeRelease(&m_pLinearGradientBrush);

    SafeRelease(&m_pWriteTarget);
    SafeRelease(&m_pTextFormat);

    SafeRelease(&m_pGradentStops);
}

void OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here, because the error will be returned again
        // the next time EndDraw is called.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

HRESULT CreateDeviceIndependentResources()
{
    // Flag Varable
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT CreateDeviceResources()
{
    // Flag Varable
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);

        // Create a Direct2D render target.
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget);

        if (SUCCEEDED(hr)) {
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&m_pWriteTarget)
            );
        }

        if (SUCCEEDED(hr)) {
            // Create gradent stops
            D2D1_GRADIENT_STOP gradent_stops[2];
            gradent_stops[0].position = 0;
            gradent_stops[0].color = D2D1::ColorF(D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f));
            gradent_stops[1].position = 1;
            gradent_stops[1].color = D2D1::ColorF(D2D1::ColorF(0.0f, 1.0f, 0.0f, 1.0f));

            hr = m_pRenderTarget->CreateGradientStopCollection(
                gradent_stops,
                2,
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &m_pGradentStops
            );
        }

        // Create Brushes
        if (SUCCEEDED(hr)) {
            // Create linear gradent brush.
            hr = m_pRenderTarget->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(DRAW_WIDTH / 2, 0),
                    D2D1::Point2F(DRAW_WIDTH / 2, DRAW_HEIGHT)),
                m_pGradentStops,
                &m_pLinearGradientBrush
            );
        }


        if (SUCCEEDED(hr)) {
            // Create a black brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(0.0f, 0.0f, 0.0f)),
                &m_pBlackBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a white brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 1.0f, 1.0f)),
                &m_pWhiteBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a cyan brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(0.0f, 1.0f, 1.0f)),
                &m_pCyanBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a yellow brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 1.0f, 0.0f)),
                &m_pYellowBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a magenta brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 0.0f, 1.0f)),
                &m_pMagentaBrush
            );
        }


        if (SUCCEEDED(hr)) {
            // Create the Text Format.
            hr = m_pWriteTarget->CreateTextFormat(
                L"Consolas",
                NULL,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                15.0f,
                L"en-us",
                &m_pTextFormat
            );
        }
    }

    return hr;
}

HRESULT OnRender()
{
    // Flag Varable
    HRESULT hr = S_OK;

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
        m_pRenderTarget->BeginDraw();
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        // Retrieve the size of the drawing area.
        D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();

        // Draw background rectangle.
        D2D1_RECT_F pingBackground = D2D1::RectF(
            0,
            0,
            DRAW_WIDTH,
            DRAW_HEIGHT
        );

        // Draw a filled rectangle.
        m_pRenderTarget->FillRectangle(&pingBackground, m_pLinearGradientBrush);

        // Draw the ping.
        float line_spacing = (float)DRAW_WIDTH / (ARRAY_LENGTH - 1);
        float offset = DRAW_HEIGHT / 4;

        // Ping Properties
        int average_ping, highest_ping, lowest_ping, instability_ping;
        ping_stats(highest_ping, lowest_ping, average_ping, instability_ping);

        // Draw
        for (int i(ARRAY_LENGTH - 1); i > 0; i--) {
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(DRAW_WIDTH - (i - 1) * line_spacing, DRAW_HEIGHT - ping_array[i - 1]),
                D2D1::Point2F(DRAW_WIDTH - (i) * line_spacing, DRAW_HEIGHT - ping_array[i]),
                m_pBlackBrush,
                1.0f
            );
        }

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(0, DRAW_HEIGHT - average_ping),
            D2D1::Point2F(DRAW_WIDTH, DRAW_HEIGHT - average_ping),
            m_pCyanBrush,
            1.5f
        );

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(0, DRAW_HEIGHT - highest_ping),
            D2D1::Point2F(DRAW_WIDTH, DRAW_HEIGHT - highest_ping),
            m_pYellowBrush,
            1.5f
        );

        D2D1_RECT_F text_rect = D2D1::RectF(
            0,
            DRAW_HEIGHT,
            DRAW_WIDTH,
            CLIENT_HEIGHT
        );

        m_pRenderTarget->FillRectangle(&text_rect, m_pBlackBrush);

        // Display string
        std::string output_string;

        if (ping_array[0] == DISCONNECT_VALUE) {
            output_string.append("Lost Connection!");
        }
        else {
            output_string.append(" Average: " + std::to_string(average_ping));
            output_string.append("\n Highest: " + std::to_string(highest_ping));
            output_string.append("\n Instability: " + std::to_string(instability_ping));
        }

        UINT string_size = output_string.length();

        std::wstring ping_wstring(output_string.begin(), output_string.end());

        const wchar_t* ping_wchar = ping_wstring.c_str();

        m_pRenderTarget->DrawTextW(
            ping_wchar,
            string_size,
            m_pTextFormat,
            &text_rect,
            m_pWhiteBrush,
            D2D1_DRAW_TEXT_OPTIONS_NO_SNAP,
            DWRITE_MEASURING_MODE_GDI_CLASSIC
        );

        hr = m_pRenderTarget->EndDraw();
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}