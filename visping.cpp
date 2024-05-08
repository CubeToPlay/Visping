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
    Visping* app = static_cast<Visping*>(lpParam);
    while (true) {
        InvalidateRect(app->hwnd, NULL, TRUE);

        std::wstring displayText = app->getDisplayText();

        if (app->pingingServer->getPing() == DISCONNECT_VALUE)
            app->nid.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_VISPING_DISCONNECT));
        else if (app->pingingServer->getInstability() > 100)
            app->nid.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_VISPING_UNSTABLE));
        else if (app->pingingServer->getPing() > 100)
            app->nid.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_VISPING_SLOW));
        else
            app->nid.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_VISPING));

        wcscpy_s(app->nid.szTip, displayText.length() + 1, displayText.c_str());

        Shell_NotifyIcon(NIM_MODIFY, &(app->nid));

        std::this_thread::sleep_for(std::chrono::milliseconds(UPDATE_LOOP_INTERVAL));
    }

    return 0;
}

HRESULT Visping::Initialize()
{
    HRESULT hr;

    // Add to Startup
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);

    HKEY hkey = NULL;
    RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
    RegSetValueEx(hkey, L"Visping", 0, REG_SZ, (BYTE*)path, (wcslen(path) + 1) * 2);

    // Initialize strings
    LoadStringW(HINST_THISCOMPONENT, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(HINST_THISCOMPONENT, IDC_VISPING, szWindowClass, MAX_LOADSTRING);
    LoadStringW(HINST_THISCOMPONENT, IDS_SERVER_ADDRESS, szServerAddress, MAX_LOADSTRING);

    // Run winsocket
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        // Handle error - Failed to initialize Winsock
    }

    pingingServer = new Ping(szServerAddress, ARRAY_LENGTH);

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
            // Create a NOTIFYICONDATA structure.
            nid = { sizeof(NOTIFYICONDATA) };
            nid.cbSize = sizeof(nid);
            nid.hWnd = hwnd;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_VISPING));
            nid.uCallbackMessage = WM_ICONNOTIFY;

            Shell_NotifyIcon(NIM_ADD, &nid);

            // Run update thread
            hUpdateThread = CreateThread(NULL, 0, UpdateThreadLoop, this, 0, NULL);

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
            case WM_SYSCOMMAND:
            {
                switch (wParam)
                {
                case SC_MINIMIZE:
                    SendMessage(hwnd, WM_CLOSE, NULL, NULL);
                    wasHandled = true;
                    break;
                }
            }
            result = 0;
            break;

            case WM_COMMAND:
            {
                // Parse the menu selections:
                switch (LOWORD(wParam))
                {
                case IDM_SERVER:
                    DialogBox(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDD_SERVER_BOX), hwnd, Server);
                    break;
                case IDM_EXIT:
                    SendMessage(hwnd, WM_DESTROY, NULL, NULL);
                    break;
                default:
                    return DefWindowProc(hwnd, message, wParam, lParam);
                }
            }
            result = 0;
            wasHandled = true;
            break;

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

            case WM_CLOSE:
            {
                ShowWindow(hwnd, SW_HIDE);
            }
            result = 1;
            wasHandled = true;
            break;

            case WM_ICONNOTIFY:
            {
                switch (lParam)
                {
                case WM_LBUTTONUP:
                    ShowWindow(hwnd, SW_SHOWNORMAL);
                    break;
                }
            }
            result = 1;
            wasHandled = true;
            break;
            }
        }

        if (!wasHandled) result = DefWindowProc(hwnd, message, wParam, lParam);
    }

    return result;
}

LRESULT CALLBACK Visping::Server(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    bool wasHandled = false;

    switch (message)
    {
    case WM_COMMAND:
    {
        switch (wParam)
        {
        case ID_OK:
            size_t i;

            //TCHAR szBuf[BUFF_LENGTH];
            //GetDlgItemText(hwnd, IDC_IPADDRESS_SERVER, szBuf, BUFF_LENGTH - 1);

            //wcstombs_s(&i, server_ip, szBuf, BUFF_LENGTH - 1);

            EndDialog(hwnd, wParam);
            break;

        case ID_CANCEL:
            EndDialog(hwnd, wParam);
            break;
        }
    }
    result = 0;
    wasHandled = true;
    break;

    case WM_INITDIALOG:
    {
        SetDlgItemText(hwnd, IDC_ADDRESS, L" ");
    }
    result = 0;
    wasHandled = true;
    break;
    }

    if (!wasHandled) result = DefWindowProc(hwnd, message, wParam, lParam);

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
        average = (int)pingingServer->getAverage();
        maximum = pingingServer->getMax();
        minimum = pingingServer->getMin();
        instability = (int)pingingServer->getInstability();

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

        std::wstring displayText = getDisplayText();
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

std::wstring Visping::getDisplayText() {
    // Display string
    std::wstring displayText;

    if (pingingServer->getPing() == DISCONNECT_VALUE)
        displayText.append(L"Lost Connection!");
    else {
        displayText.append(L" Average: " + std::to_wstring((int)pingingServer->getAverage()));
        displayText.append(L"\n Highest: " + std::to_wstring(pingingServer->getMax()));
        displayText.append(L"\n Instability: " + std::to_wstring((int)pingingServer->getInstability()));
    }

    return displayText;
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