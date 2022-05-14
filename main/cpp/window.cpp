#ifndef UNICODE
#define UNICODE
#endif 

#include <iostream>

#include <windows.h>
#include <thread>
// #include <string>

#include "ping.h"

//Defines the way the user interacts with the window.
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//Like Main, but for windows.
INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow){
   //Creates the Window Class (Windows Class is not the same as c++ classes)
   const wchar_t CLASS_NAME[] = L"Sample Window Class";//An array of characters.

   WNDCLASS wc = {};

   wc.lpfnWndProc   = WindowProc;
   wc.hInstance     = hInstance;
   wc.lpszClassName = CLASS_NAME;

   //Registers the Window Class.
   RegisterClass(&wc);



   //Creating the Window Instance
   HWND hwnd = CreateWindowEx(
      0,                              // Optional window styles.
      CLASS_NAME,                     // Window class
      L"Windows Application",    // Window text
      WS_OVERLAPPEDWINDOW,            // Window style

      // Size and position
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

      NULL,       // Parent window    
      NULL,       // Menu
      hInstance,  // Instance handle
      NULL        // Additional application data
   );

   //If creating the Window Fails, it quits the program
   if (hwnd == NULL){
      return 0;
   }

   ShowWindow(hwnd, nCmdShow);

   //Message Loop
   MSG msg = { };
   while (GetMessage(&msg, NULL, 0, 0) > 0 ){
       TranslateMessage(&msg);
       DispatchMessage(&msg);
   };

   //Ping Thread
   const char server[] = "google.com";
   std::thread pinging (ping::server, server);

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
            // const char server[] = "google.com";
            // std::cout << ping::server(server) << std::endl;

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));

            EndPaint(hwnd, &ps);
        }
        return 0;
        
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam); // Runs the default action for the message (Varies baised on message.)
};