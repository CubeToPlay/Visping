#ifndef UNICODE
#define UNICODE
#endif 

#define WIDTH 500
#define HEIGHT 200

#include <windows.h>
#include <wingdi.h>

#include "ping.h"

//Defines the way the user interacts with the window.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//Pinging Thread
DWORD WINAPI PingingThread(LPVOID lpParam);

//Like Main, but for windows.
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow){
    //Creates the Window Class (Windows Class is not the same as c++ classes)
    const wchar_t CLASS_NAME[] = L"Sample Window Class";//An array of characters.
    // const int WIDTH = 

    if (hInstance == NULL){
        hInstance = (HINSTANCE)GetModuleHandle(NULL);
    }

    WCHAR szExePath[MAX_PATH];
    GetModuleFileName(NULL, szExePath, MAX_PATH);

    HICON hIcon = NULL;
    //If hIcon is Null, it uses the first one found in the exe.
    if (hIcon == NULL){
        hIcon = ExtractIcon(hInstance, szExePath, 0);
    }

    //Registers the Window Class.
    WNDCLASS wc;
    wc.style         = CS_DBLCLKS;
    wc.lpfnWndProc   = WindowProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = hIcon;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = CLASS_NAME;

    if(!RegisterClass(&wc)){
        DWORD dwError = GetLastError();
        if (dwError != ERROR_CLASS_ALREADY_EXISTS){
            return HRESULT_FROM_WIN32(dwError);
        }
    }

    //Creating the Window Instance
    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Visping",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );


    //If creating the Window Fails, it quits the program
    if (hwnd == NULL){
        return 0;
    }

    DWORD PingingThreadID;
    HANDLE PingingThreadHandle = CreateThread(0, 0, PingingThread, 0, 0, &PingingThreadID);
    CloseHandle(PingingThreadHandle);

    std::cout << "Pinging Thread ID: " << PingingThreadID << std::endl;

    ShowWindow(hwnd, nCmdShow);

    //Message Loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0 ){
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    };

    return 0;
}

//Window Proc Function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
    switch (uMsg)
    {
    case WM_DESTROY: 
        PostQuitMessage(0);
        return 0;
    
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // All painting occurs here (Between BeginPaint and EndPaint)
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));

            
            
            ping::draw(hdc, WIDTH, HEIGHT);

            EndPaint(hwnd, &ps);
        }
        return 0;
        
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam); // Runs the default action for the message (Varies baised on message.)
};

//Pinging Loop Function
DWORD WINAPI PingingThread(LPVOID lpParam){
    const char server[] = "google.com";
    
    while (true) {
        ping::insert(ping::once(server));
        ping::display();
    };

    return 0;
};