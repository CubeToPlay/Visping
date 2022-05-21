#ifndef UNICODE
#define UNICODE
#endif 

#include "application.h"
#include "visping.h"

#define DRAW_WIDTH 500
#define DRAW_HEIGHT 200

#define CLIENT_WIDTH 500
#define CLIENT_HEIGHT 300

HWND MainHWND;

// Translates and dispatches messages
void App::RunMessageLoop(){
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

// Creates the window, shows it, and calls the App::CreateDeviceIndependentResources method
HRESULT App::Initialize(){
    const DWORD DW_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME;
    
    const wchar_t CLASS_NAME[] = L"Visping Window Class";

    RECT wr;
    wr.top = 0;
    wr.left = 0;
    wr.bottom = CLIENT_HEIGHT;
    wr.right = CLIENT_WIDTH;

    AdjustWindowRect(&wr, DW_STYLE, FALSE);

    HRESULT hr;
    // Initialize device-indpendent resources, such as the Direct2D factory.
    hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr)){
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style         = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc   = App::WndProc;
        wcex.cbClsExtra    = 0;
        wcex.cbWndExtra    = sizeof(LONG_PTR);
        wcex.hInstance     = HINST_THISCOMPONENT;
        wcex.hbrBackground = NULL;
        wcex.lpszMenuName  = NULL;
        wcex.hCursor       = LoadCursor(NULL, IDI_APPLICATION);
        wcex.lpszClassName = CLASS_NAME;

        RegisterClassEx(&wcex);


        // Because the CreateWindow function takes its size in pixels, obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;

        // The factory returns the current system DPI. This is also the value it will use to create its own windows.
        m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


        // Create the window.
        m_hwnd = CreateWindow(
            CLASS_NAME,
            L"Visping",
            DW_STYLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            wr.right - wr.left,
            wr.bottom - wr.top,
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this
        );

        MainHWND = m_hwnd;

        hr = m_hwnd ? S_OK : E_FAIL;
        
        if (SUCCEEDED(hr)){
            // Remove all window styles, check MSDN for details
            // SetWindowLong(m_hwnd, GWL_STYLE, 0); 
            // Display window
            ShowWindow(m_hwnd, SW_SHOW);
            UpdateWindow(m_hwnd);
        }

        DWORD PingingThreadID;
        HANDLE PingingThreadHandle = CreateThread(0, 0, App::PingingThread, 0, 0, &PingingThreadID);
        CloseHandle(PingingThreadHandle);
    }

    return hr;
}

// Serves as the application entry point. Initialize an instance of the App class and begin its message loop.
INT WINAPI WinMain(
    HINSTANCE /* hInstance */,
    HINSTANCE /* hPrevInstance */,
    LPSTR /* lpCmdLine */,
    int /* nCmdShow */
) {
    // Use HeapSetInformation to specify that the process should terminate if the heap manager detects an error in any heap used by the process.
    // The return value is ignored, because we want to continue running in the unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL))) {
        {
            App app;

            if (SUCCEEDED(app.Initialize())) {
                // AdjustWindowRect()
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}

// Create an ID2D1Factory, a device-independent resource, for creating other Direct2D resources. Use the m_pDirect2DdFactory class member to store the factory.
HRESULT App::CreateDeviceIndependentResources(){
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT App::CreateDeviceResources(){
    HRESULT hr = S_OK;

    if (!m_pRenderTarget){
        // Creates the window's device-dependent resources, a render target, and two brushes.
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
        );

        // Create a Direct2D render target.
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
        );

        if (SUCCEEDED(hr)) {
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&m_pWriteTarget)
            );
        }

        // Create gradent stops
        D2D1_GRADIENT_STOP gradentStops[2];
        gradentStops[0].position = 0;
        gradentStops[0].color = D2D1::ColorF(D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f));
        gradentStops[1].position = 1;
        gradentStops[1].color = D2D1::ColorF(D2D1::ColorF(0.0f, 1.0f, 0.0f, 1.0f));

        hr = m_pRenderTarget->CreateGradientStopCollection(
            gradentStops,
            2,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &m_pGradentStops
        );

        // Create Brushes
        if (SUCCEEDED(hr)){
            // Create linear gradent brush.
            hr = m_pRenderTarget->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(DRAW_WIDTH/2, 0),
                    D2D1::Point2F(DRAW_WIDTH/2, DRAW_HEIGHT)),
                m_pGradentStops,
                &m_pLinearGradientBrush
            );
        }

        if (SUCCEEDED(hr)){
            // Create a black brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(0.0f, 0.0f, 0.0f)),
                &m_pBlackBrush
            );
        }

        if (SUCCEEDED(hr)){
            // Create a white brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 1.0f, 1.0f)),
                &m_pWhiteBrush
            );
        }

        if (SUCCEEDED(hr)){
            // Create a cyan brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(0.0f, 1.0f, 1.0f)),
                &m_pCyanBrush
            );
        }

        if (SUCCEEDED(hr)){
            // Create a yellow brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 1.0f, 0.0f)),
                &m_pYellowBrush
            );
        }

        if (SUCCEEDED(hr)){
            // Create a magenta brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 0.0f, 1.0f)),
                &m_pMagentaBrush
            );
        }

        if (SUCCEEDED(hr)){
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

// Release the render target and the two brushes you created.
void App::DiscardDeviceResources(){
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pLinearGradientBrush);
    
    SafeRelease(&m_pBlackBrush);
    SafeRelease(&m_pWhiteBrush);
    SafeRelease(&m_pCyanBrush);
    SafeRelease(&m_pYellowBrush);
    SafeRelease(&m_pMagentaBrush);

    SafeRelease(&m_pWriteTarget);
    SafeRelease(&m_pTextFormat);
}

// Implement the windows procedure, the OnRender method that paints content, and the OnResize method that adjusts the size of the render target when the window is resized.
LRESULT CALLBACK App::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    LRESULT result = 0;

    if (message == WM_CREATE) {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        App *pApp = (App *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pApp)
        );

        result = 1;
    } else {
        App *pApp = reinterpret_cast<App *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
        )));

        bool wasHandled = false;

        if (pApp) {
            switch (message) {
            case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pApp->OnResize(width, height);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
                {
                    pApp->OnRender();
                    ValidateRect(hwnd, NULL);
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

        if (!wasHandled){
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

HRESULT App::OnRender()
{
    HRESULT hr = S_OK;

    hr = CreateDeviceResources();

    // Verify that the CreateDeviceResource method succeeded. If it didn't, don't perform any drawing.
    if (SUCCEEDED(hr)) {
        // std::cout << "Running" << std::endl;
        // Initiate drawing by calling the render target's BeginDraw method. Set the render target's transform to the identity matrix, and clear the window.
        m_pRenderTarget->BeginDraw();
                
        m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));

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
        float lineSpacing = DRAW_WIDTH/arrayLength;
        float amplification = 1;
        float offset = DRAW_HEIGHT/4;

        // Ping Properties
        int averagePing = vpg::average() * amplification;
        int highestPing = vpg::highest() * amplification;
        int lowestPing = vpg::lowest() * amplification;

        for(int i = arrayLength; i >= 1; i--){
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(DRAW_WIDTH - (i-1) * lineSpacing, DRAW_HEIGHT - vpg::list[i-1] * amplification),
                D2D1::Point2F(DRAW_WIDTH - (i) * lineSpacing, DRAW_HEIGHT - vpg::list[i] * amplification),
                m_pBlackBrush,
                1.0f
            );
        }

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(0, DRAW_HEIGHT - averagePing),
            D2D1::Point2F(DRAW_WIDTH, DRAW_HEIGHT - averagePing),
            m_pCyanBrush,
            1.5f
        );

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(0, DRAW_HEIGHT - highestPing),
            D2D1::Point2F(DRAW_WIDTH, DRAW_HEIGHT - highestPing),
            m_pYellowBrush,
            1.5f
        );

        // m_pRenderTarget->DrawLine(
        //     D2D1::Point2F(0, DRAW_HEIGHT - lowestPing),
        //     D2D1::Point2F(DRAW_WIDTH, DRAW_HEIGHT - lowestPing),
        //     m_pMagentaBrush,
        //     1.5f
        // );

        D2D1_RECT_F textRect = D2D1::RectF(
            0,
            DRAW_HEIGHT,
            DRAW_WIDTH,
            CLIENT_HEIGHT
        );

        std::string pingString = vpg::string();
        UINT stringSize = pingString.length();

        std::wstring pingWstring(pingString.begin(), pingString.end());

        const wchar_t* pingWchar = pingWstring.c_str();
        // const wchar_t* pingWchar = L"Average: ";

        // std::cout << pingString << std::endl;

        m_pRenderTarget->DrawTextW(
            pingWchar,
            stringSize,
            m_pTextFormat,
            &textRect,
            m_pWhiteBrush,
            D2D1_DRAW_TEXT_OPTIONS_NO_SNAP,
            DWRITE_MEASURING_MODE_GDI_CLASSIC
        );

        // Call the render target's EndDraw method.
        hr = m_pRenderTarget->EndDraw();
    }

    // Check the HRESULT returned by EndDraw. If it indicates that the render target needs to be recreated, call the App::DiscardDeviceResources method to release it
    if (hr == D2DERR_RECREATE_TARGET) {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}

// Resizes the render target to the new size of the window.
void App::OnResize(UINT width, UINT height){
    if (m_pRenderTarget){
        // Note: This method can fail, but it's okay to ignore the error here, because the error will be returned again the next time EndDraw is called.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

// Pinging Loop Function
DWORD WINAPI App::PingingThread(LPVOID lpParam){
    const char server[] = "8.8.8.8";
    
    while (true) {
        vpg::insert(vpg::once(server));
        // vpg::display();
        InvalidateRect(MainHWND, NULL, TRUE);
    };

    return 0;
};