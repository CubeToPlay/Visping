// Visping.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Visping.h"

Visping::Visping() : 
    pFactory(NULL),
    pHwndRenderTarget(NULL),
    pBlackBrush(NULL),
    pWhiteBrush(NULL),
    pCyanBrush(NULL),
    pYellowBrush(NULL),
    pMagentaBrush(NULL),
    pLinearGradientBrush(NULL),
    pWriteFactory(NULL),
    pWriteTextFormat(NULL),
    pGradentStopCollection(NULL)
{}

Visping::~Visping() {
    CloseHandle(hUpdateThread);

    SafeRelease(&pFactory);
    DiscardDeviceResources();
}

void Visping::DiscardDeviceResources()
{
    SafeRelease(&pHwndRenderTarget);
    SafeRelease(&pBlackBrush);
    SafeRelease(&pWhiteBrush);
    SafeRelease(&pCyanBrush);
    SafeRelease(&pYellowBrush);
    SafeRelease(&pMagentaBrush);
    SafeRelease(&pLinearGradientBrush);
    SafeRelease(&pWriteFactory);
    SafeRelease(&pWriteTextFormat);
    SafeRelease(&pGradentStopCollection);
}


DWORD WINAPI Visping::UpdateThreadLoop(LPVOID lpParam) {
    HWND* hwnd = static_cast<HWND*>(lpParam);
    while (true) {
        InvalidateRect(*hwnd, NULL, TRUE);

        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_LOOP_INTERVAL));
    }

    return 0;
}

HRESULT Visping::Initialize()
{
    HRESULT hr;

    // Initialize strings
    LoadStringW(HINST_THISCOMPONENT, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(HINST_THISCOMPONENT, IDC_VISPING, szWindowClass, MAX_LOADSTRING);
    LoadStringW(HINST_THISCOMPONENT, IDS_SERVER_IPV4, szServerIPv4, MAX_LOADSTRING);


    // Run winsocket
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        // Handle error - Failed to initialize Winsock
    }

    pingingServer = new Ping(szServerIPv4, ARRAY_LENGTH);

    pingingServer->start();

    // Initialize device-independent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr))
    {
        // Register the window class.
        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = Visping::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = HINST_THISCOMPONENT;
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_VISPING);
        wcex.hCursor = LoadCursor(NULL, IDI_APPLICATION);
        wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_VISPING));
        wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
        wcex.lpszClassName = szWindowClass;
        wcex.cbSize = sizeof(WNDCLASSEX);

        //wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);

        RegisterClassEx(&wcex);

        // In terms of using the correct DPI, to create a window at a specific size
        // like this, the procedure is to first create the window hidden. Then we get
        // the actual DPI from the HWND (which will be assigned by whichever monitor
        // the window is created on). Then we use SetWindowPos to resize it to the
        // correct DPI-scaled size, then we use ShowWindow to show it.

        RECT wr, desktop;
        wr.top = 0;
        wr.left = 0;
        wr.bottom = CLIENT_HEIGHT;
        wr.right = CLIENT_WIDTH;

        GetClientRect(GetDesktopWindow(), &desktop);

        hwnd = CreateWindowW(
            szWindowClass,
            szTitle,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this);

        if (hwnd)
        {
            // Run update thread
            hUpdateThread = CreateThread(NULL, 0, UpdateThreadLoop, &hwnd, 0, NULL);

            // Because the SetWindowPos function takes its size in pixels, we
            // obtain the window's DPI, and use it to scale the window size.
            int dpi = GetDpiForWindow(hwnd);

            SetWindowPos(
                hwnd,
                NULL,
                NULL,
                NULL,
                wr.right - wr.left,
                wr.bottom - wr.top,
                /*
                static_cast<int>(ceil(640.f * dpi / 96.f)),
                static_cast<int>(ceil(480.f * dpi / 96.f)),*/
                SWP_NOMOVE);
            ShowWindow(hwnd, SW_SHOWNORMAL);
            UpdateWindow(hwnd);
        }
    }

    return hr;
}

void Visping::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT Visping::CreateDeviceResources() {
    HRESULT hr = S_OK;

    if (!pHwndRenderTarget)
    {
        RECT rc;
        GetClientRect(hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);

        // Create a Direct2D render target.
        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hwnd, size),
            &pHwndRenderTarget);

        // Create Factory
        if (SUCCEEDED(hr)) {
            hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(&pWriteFactory)
            );
        }

        // Create gradent stops
        if (SUCCEEDED(hr)) {
            D2D1_GRADIENT_STOP gradent_stops[2];
            gradent_stops[0].position = 0;
            gradent_stops[0].color = D2D1::ColorF(D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f));
            gradent_stops[1].position = 1;
            gradent_stops[1].color = D2D1::ColorF(D2D1::ColorF(0.0f, 1.0f, 0.0f, 1.0f));

            hr = pHwndRenderTarget->CreateGradientStopCollection(
                gradent_stops,
                2,
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &pGradentStopCollection
            );
        }

        // Create Brushes
        if (SUCCEEDED(hr)) {
            // Create linear gradent brush.
            hr = pHwndRenderTarget->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(DRAW_WIDTH / 2, 0),
                    D2D1::Point2F(DRAW_WIDTH / 2, DRAW_HEIGHT)),
                pGradentStopCollection,
                &pLinearGradientBrush
            );
        }


        if (SUCCEEDED(hr)) {
            // Create a black brush.
            hr = pHwndRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(0.0f, 0.0f, 0.0f)),
                &pBlackBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a white brush.
            hr = pHwndRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 1.0f, 1.0f)),
                &pWhiteBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a cyan brush.
            hr = pHwndRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(0.0f, 1.0f, 1.0f)),
                &pCyanBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a yellow brush.
            hr = pHwndRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 1.0f, 0.0f)),
                &pYellowBrush
            );
        }

        if (SUCCEEDED(hr)) {
            // Create a magenta brush.
            hr = pHwndRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF(1.0f, 0.0f, 1.0f)),
                &pMagentaBrush
            );
        }


        if (SUCCEEDED(hr)) {
            // Create the Text Format.
            hr = pWriteFactory->CreateTextFormat(
                L"Consolas",
                NULL,
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                15.0f,
                L"en-us",
                &pWriteTextFormat
            );
        }
    }

    return hr;
}

LRESULT CALLBACK Visping::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        Visping* pApp = (Visping*)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pApp)
        );

        result = 1;
    }
    else
    {
        Visping* pApp = reinterpret_cast<Visping*>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
            )));

        bool wasHandled = false;

        if (pApp)
        {
            switch (message)
            {
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

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

HRESULT Visping::OnRender()
{
    HRESULT hr = S_OK;

    hr = CreateDeviceResources();

    if (SUCCEEDED(hr))
    {
        pHwndRenderTarget->BeginDraw();
        pHwndRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        pHwndRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

        // Retrieve the size of the drawing area.
        D2D1_SIZE_F rtSize = pHwndRenderTarget->GetSize();

        // Draw background rectangle.
        D2D1_RECT_F pingBackground = D2D1::RectF(
            0,
            0,
            DRAW_WIDTH,
            DRAW_HEIGHT
        );

        // Draw a filled rectangle.
        pHwndRenderTarget->FillRectangle(&pingBackground, pLinearGradientBrush);

        // Draw the ping.
        double lineSpacing = (double)DRAW_WIDTH / (ARRAY_LENGTH - 1);
        double offset = DRAW_HEIGHT / 4;

        // Ping Properties
        int average, maximum, minimum, instability;
        average = pingingServer->getAverage();
        maximum = pingingServer->getMax();
        minimum = pingingServer->getMin();
        instability = pingingServer->getInstability();

        // Draw
        for (int i(ARRAY_LENGTH - 1); i > 0; i--) {
            pHwndRenderTarget->DrawLine(
                D2D1::Point2F((float)(DRAW_WIDTH - (i - 1) * lineSpacing), (float)(DRAW_HEIGHT - pingingServer->getPing(i - 1))),
                D2D1::Point2F((float)(DRAW_WIDTH - (i) * lineSpacing), (float)(DRAW_HEIGHT - pingingServer->getPing(i))),
                pBlackBrush,
                1.0f
            );
        }

        pHwndRenderTarget->DrawLine(
            D2D1::Point2F(0, DRAW_HEIGHT - average),
            D2D1::Point2F(DRAW_WIDTH, DRAW_HEIGHT - average),
            pCyanBrush,
            1.5f
        );

        pHwndRenderTarget->DrawLine(
            D2D1::Point2F(0, DRAW_HEIGHT - maximum),
            D2D1::Point2F(DRAW_WIDTH, DRAW_HEIGHT - maximum),
            pYellowBrush,
            1.5f
        );

        D2D1_RECT_F textRect = D2D1::RectF(
            0,
            DRAW_HEIGHT,
            DRAW_WIDTH,
            CLIENT_HEIGHT
        );

        pHwndRenderTarget->FillRectangle(&textRect, pBlackBrush);

        // Display string
        std::wstring displayText;

        if (pingingServer->getPing() == DISCONNECT_VALUE)
            displayText.append(L"Lost Connection!");
        else {
            displayText.append(L" Average: " + std::to_wstring(average));
            displayText.append(L"\n Highest: " + std::to_wstring(maximum));
            displayText.append(L"\n Instability: " + std::to_wstring(instability));
        }

        pHwndRenderTarget->DrawTextW(
            displayText.c_str(),
            displayText.length(),
            pWriteTextFormat,
            &textRect,
            pWhiteBrush,
            D2D1_DRAW_TEXT_OPTIONS_NO_SNAP,
            DWRITE_MEASURING_MODE_GDI_CLASSIC
        );

        hr = pHwndRenderTarget->EndDraw();
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        hr = S_OK;
        DiscardDeviceResources();
    }

    return hr;
}

void Visping::OnResize(UINT width, UINT height)
{
    if (pHwndRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here, because the error will be returned again
        // the next time EndDraw is called.
        pHwndRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

HRESULT Visping::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

    return hr;
}

int WINAPI WinMain(
    HINSTANCE /* hInstance */,
    HINSTANCE /* hPrevInstance */,
    LPSTR /* lpCmdLine */,
    int /* nCmdShow */
)
{
    // Use HeapSetInformation to specify that the process should
    // terminate if the heap manager detects an error in any heap used
    // by the process.
    // The return value is ignored, because we want to continue running in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            Visping app;

            if (SUCCEEDED(app.Initialize())) app.RunMessageLoop();
        }
        CoUninitialize();
    }

    return 0;
}