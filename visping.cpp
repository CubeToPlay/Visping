// Visping.cpp : Defines the entry point for the application.

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
{
    LoadStringW(HINST_THISCOMPONENT, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(HINST_THISCOMPONENT, IDC_VISPING, szWindowClass, MAX_LOADSTRING);
    LoadStringW(HINST_THISCOMPONENT, IDC_SETTINGS_FILE, szSettingsFileName, MAX_LOADSTRING);
}

Visping::~Visping() {
    running.store(false);
    CloseHandle(hUpdateThread);
    DestroyMenu(hNotifyIconMenu);
    SafeRelease(&pFactory);
    DiscardDeviceResources();
    pingingServer->stop();
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
    while (app->running.load()) {
        InvalidateRect(app->hwnd, NULL, TRUE);

        std::wstring displayText = app->getDisplayText();

        if (app->pingingServer->getPing() == DISCONNECT_VALUE)
            app->nid.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_VISPING_DISCONNECT));
        else if (app->pingingServer->getInstability() > 100)
            app->nid.hIcon = LoadIcon(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDI_VISPING_UNSTABLE));
        else if (app->pingingServer->getAverage() > 100)
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
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

    // Initialize strings;
    LoadStringW(HINST_THISCOMPONENT, IDS_SERVER_ADDRESS, szServerAddressDefault, MAX_LOADSTRING);
    GetModuleFileNameW(NULL, szPath, MAX_PATH);

    // Initalize Other
    hNotifyIconMenu = CreatePopupMenu();
    AppendMenu(hNotifyIconMenu, MF_STRING, IDM_EXIT, L"Exit");

    GetClientRect(GetDesktopWindow(), &desktop);

    // Create Settings File
    DWORD bufferSize = MAX_PATH;
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId());
    HANDLE tokenHandle;
    OpenProcessToken(processHandle, TOKEN_QUERY, &tokenHandle);
    GetUserProfileDirectoryW(tokenHandle, szUserPath, &bufferSize);

    wcscpy_s(szPathStartup, (wcslen(szPath) + 1) * 2, szPath);
    wcscat_s(szPathStartup, L" --startup");

    wcscpy_s(szPathSettingsFile, szUserPath);
    wcscat_s(szPathSettingsFile, L"\\.visping\\");

    CreateDirectory(szPathSettingsFile, NULL);

    wcscat_s(szPathSettingsFile, szSettingsFileName);
    
    CloseHandle(CreateFile(
        szPathSettingsFile,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL)
    );

    // Add to Startup
    HKEY hkey = NULL;
    RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
    RegSetValueEx(hkey, L"Visping", 0, REG_SZ, (BYTE*)szPathStartup, (wcslen(szPathStartup) + 1) * 2);

    // Run winsocket
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    GetPrivateProfileStringW(L"Server", L"address", szServerAddressDefault, szServerAddress, MAX_LOADSTRING, szPathSettingsFile);
    GetPrivateProfileIntW(L"Window", L"center-window", 0, szPathSettingsFile);

    running.store(true);

    pingingServer.reset(new Ping(szServerAddress, ARRAY_LENGTH));
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

        hwnd = CreateWindowW(
            szWindowClass,
            szTitle,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CLIENT_WIDTH,
            CLIENT_HEIGHT,
            NULL,
            NULL,
            HINST_THISCOMPONENT,
            this);

        if (hwnd)
        {
            hMenu = GetMenu(hwnd);

            GetMenuItemInfo(hMenu, IDM_SHOW_STARTUP, FALSE, &showOnStartupMenuItemInfo);
            GetMenuItemInfo(hMenu, IDM_CENTER_WINDOW, FALSE, &centerWindowMenuItemInfo);

            showOnStartupMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
            showOnStartupMenuItemInfo.fMask = MIIM_STATE;
            showOnStartupMenuItemInfo.fState = (GetPrivateProfileIntW(L"Window", L"show-on-startup", 0, szPathSettingsFile) == 1) ? MFS_CHECKED : MFS_UNCHECKED;
            SetMenuItemInfo(hMenu, IDM_SHOW_STARTUP, FALSE, &showOnStartupMenuItemInfo);

            centerWindowMenuItemInfo.cbSize = sizeof(MENUITEMINFO);
            centerWindowMenuItemInfo.fMask = MIIM_STATE;
            centerWindowMenuItemInfo.fState = (GetPrivateProfileIntW(L"Window", L"center-window", 1, szPathSettingsFile) == 1) ? MFS_CHECKED : MFS_UNCHECKED;
            SetMenuItemInfo(hMenu, IDM_CENTER_WINDOW, FALSE, &centerWindowMenuItemInfo);

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

            updateWindowCentered(
                GetPrivateProfileIntW(L"Window", L"position-x", (desktop.right - CLIENT_WIDTH) / 2, szPathSettingsFile),
                GetPrivateProfileIntW(L"Window", L"position-y", 0, szPathSettingsFile)
            );
            
            // The process was started by the user or show on startup is enabled.
            bool startup = false;

            for (int i = 1; i < __argc && !startup; i++)
                if (__argv[i] == std::string("--startup"))
                    startup = true;

            if (!startup || showOnStartupMenuItemInfo.fState == MFS_CHECKED)
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
                switch (wParam)
                {
                case IDM_SERVER:
                    DialogBoxParam(HINST_THISCOMPONENT, MAKEINTRESOURCE(IDD_SERVER_BOX), hwnd, Server, (LPARAM)pApp);
                    break;
                case IDM_EXIT:
                    SendMessage(hwnd, WM_DESTROY, NULL, NULL);
                    break;
                case IDM_SHOW_STARTUP:
                    pApp->showOnStartupMenuItemInfo.fState = (pApp->showOnStartupMenuItemInfo.fState == MFS_CHECKED) ? MFS_UNCHECKED : MFS_CHECKED;
                    SetMenuItemInfo(pApp->hMenu, IDM_SHOW_STARTUP, FALSE, &(pApp->showOnStartupMenuItemInfo));

                    WritePrivateProfileStringW(L"Window", L"show-on-startup", (pApp->showOnStartupMenuItemInfo.fState == MFS_CHECKED) ? L"1" : L"0", pApp->szPathSettingsFile);

                    break;
                case IDM_CENTER_WINDOW:
                    pApp->centerWindowMenuItemInfo.fState = (pApp->centerWindowMenuItemInfo.fState == MFS_CHECKED) ? MFS_UNCHECKED : MFS_CHECKED;
                    SetMenuItemInfo(pApp->hMenu, IDM_CENTER_WINDOW, FALSE, &(pApp->centerWindowMenuItemInfo));

                    WritePrivateProfileStringW(L"Window", L"center-window", (pApp->centerWindowMenuItemInfo.fState == MFS_CHECKED) ? L"1" : L"0", pApp->szPathSettingsFile);

                    if (pApp->centerWindowMenuItemInfo.fState == MFS_CHECKED) pApp->saveWindowPosition();

                    pApp->updateWindowCentered(
                        GetPrivateProfileIntW(L"Window", L"position-x", (pApp->desktop.right - CLIENT_WIDTH) / 2, pApp->szPathSettingsFile),
                        GetPrivateProfileIntW(L"Window", L"position-y", 0, pApp->szPathSettingsFile)
                    );
                    
                    break;
                case IDM_HIDE:
                    SendMessage(hwnd, WM_CLOSE, NULL, NULL);
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
                if (pApp->centerWindowMenuItemInfo.fState == MFS_UNCHECKED) pApp->saveWindowPosition();
                Shell_NotifyIcon(NIM_DELETE, &(pApp->nid));
                DestroyWindow(pApp->hwnd);
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
                    SetForegroundWindow(hwnd);
                    SetFocus(hwnd);

                    break;
                case WM_RBUTTONUP:
                    POINT pt;
                    GetCursorPos(&pt);
                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(pApp->hNotifyIconMenu, 0, pt.x, pt.y, 0, hwnd, NULL);
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

INT_PTR CALLBACK Visping::Server(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT_PTR result = 0;
    bool wasHandled = false;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        Visping* pApp = (Visping*)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, lParam);

        SetDlgItemText(hwnd, IDC_ADDRESS, pApp->szServerAddress);
    }
    result = 1;
    wasHandled = true;
    break;


    case WM_COMMAND:
    {
        Visping* pApp = (Visping*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        switch (wParam)
        {
        case ID_OK:
            WCHAR szBuffer[MAX_LOADSTRING];
            GetDlgItemText(hwnd, IDC_ADDRESS, szBuffer, MAX_LOADSTRING);

            WritePrivateProfileStringW(L"Server", L"address", szBuffer, pApp->szPathSettingsFile);
            wcscpy_s(pApp->szServerAddress, szBuffer);
            pApp->pingingServer->setAddress(szBuffer);

            
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

    }
    if (!wasHandled) result = DefWindowProc(hwnd, message, wParam, lParam);

    return result;
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
            D2D1::Point2F(0.0f, (float)(DRAW_HEIGHT - average)),
            D2D1::Point2F((float)DRAW_WIDTH, (float)(DRAW_HEIGHT - average)),
            pCyanBrush,
            1.5f
        );

        pHwndRenderTarget->DrawLine(
            D2D1::Point2F(0, (float)(DRAW_HEIGHT - maximum)),
            D2D1::Point2F((float)DRAW_WIDTH, (float)(DRAW_HEIGHT - maximum)),
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

        if (pingingServer->isRunning())
        {
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
        }

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
        displayText.append(L" Lost Connection!");
    else {
        displayText.append(L" Average: " + std::to_wstring((int)pingingServer->getAverage()));
        displayText.append(L"\n Highest: " + std::to_wstring(pingingServer->getMax()));
        displayText.append(L"\n Instability: " + std::to_wstring((int)pingingServer->getInstability()));
    }

    return displayText;
}

void Visping::updateWindowCentered(int px, int py) {
    if (centerWindowMenuItemInfo.fState == MFS_CHECKED)
        SetWindowPos(
            hwnd,
            NULL,
            (desktop.right - CLIENT_WIDTH) / 2,
            -TITLE_BAR_HEIGHT,
            CLIENT_WIDTH,
            CLIENT_HEIGHT,
            SWP_NOSIZE);
    else
        SetWindowPos(
            hwnd,
            NULL,
            px,
           py,
            CLIENT_WIDTH,
            CLIENT_HEIGHT,
            SWP_NOSIZE);
}

void Visping::saveWindowPosition() {
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    WritePrivateProfileStringW(L"Window", L"position-x", std::to_wstring(windowRect.left).c_str(), szPathSettingsFile);
    WritePrivateProfileStringW(L"Window", L"position-y", std::to_wstring(windowRect.top).c_str(), szPathSettingsFile);
}

int WINAPI WinMain(
    HINSTANCE  hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
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
        Visping app;

        HANDLE hMutex = CreateMutexW(NULL, FALSE, L"VispingMutex");

        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            HWND hwnd = FindWindowW(app.szWindowClass, app.szTitle);

            if (IsWindowVisible(hwnd))
                SetForegroundWindow(hwnd);
            else
                ShowWindow(hwnd, SW_SHOWNORMAL);
        }

        else if (SUCCEEDED(app.Initialize())) 
            app.RunMessageLoop();

        CloseHandle(hMutex);
        
        CoUninitialize();
    }

    return 0;
}