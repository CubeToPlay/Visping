#include "visping.h"

int main() {
    // Create Window
    HWND hwnd = CreateWindow(
        "STATIC",
        "Visping",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        HINSTANCE(GetModuleHandle(NULL)),
        NULL
    );

    // Show Window
    ShowWindow(hwnd, SW_SHOWNORMAL);

    // Wait for user to close the window
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Destroy the window
    DestroyWindow(hwnd);

    return 0;
}