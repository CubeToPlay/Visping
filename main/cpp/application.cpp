#ifndef UNICODE
#define UNICODE
#endif 

#include "application.h"
#include "visping.h"

#define WIDTH 500
#define HEIGHT 200

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
    const wchar_t CLASS_NAME[] = L"Visping Window Class";//An array of characters.

    RECT wr;
    wr.top = 0;
    wr.left = 0;
    wr.bottom = HEIGHT;
    wr.right = WIDTH;

    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME, FALSE);

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
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
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
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
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
            &pGradentStops
        );

        // Create Brushes
        if (SUCCEEDED(hr)){
            // Create linear gradent brush.
            hr = m_pRenderTarget->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(rc.right/2, 0),
                    D2D1::Point2F(rc.right/2, rc.bottom)),
                pGradentStops,
                &m_pLinearGradientBrush
            );
        }

        if (SUCCEEDED(hr)){
            // Create a black brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::Black),
                &m_pBlackBrush
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

        // Draw a background.
        int width = static_cast<int>(rtSize.width);
        int height = static_cast<int>(rtSize.height);

        // Draw background rectangle.
        D2D1_RECT_F background = D2D1::RectF(
            0,
            0,
            width,
            height
        );

        // Draw a filled rectangle.
        m_pRenderTarget->FillRectangle(&background, m_pLinearGradientBrush);

        // Draw the ping.
        float lineSpacing = width/arrayLength;

        for(int i = arrayLength; i >= 1; i--){
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(width - (i-1) * lineSpacing, height - vpg::list[i-1]),
                D2D1::Point2F(width - (i) * lineSpacing, height - vpg::list[i]),
                m_pBlackBrush,
                0.5f
            );
        }

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(0, height - vpg::average()),
            D2D1::Point2F(width, height - vpg::average()),
            m_pBlackBrush,
            0.75f
        );

        m_pRenderTarget->DrawLine(
            D2D1::Point2F(0, height - vpg::highest()),
            D2D1::Point2F(width, height - vpg::highest()),
            m_pBlackBrush,
            0.75f
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
    const char server[] = "google.com";
    
    while (true) {
        vpg::insert(vpg::once(server));
        vpg::display();
        InvalidateRect(MainHWND, NULL, TRUE);
    };

    return 0;
};