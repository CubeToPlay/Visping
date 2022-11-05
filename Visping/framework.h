// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#include "Visping.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <wincodec.h>

#define DRAW_WIDTH 500
#define DRAW_HEIGHT 200

#define CLIENT_WIDTH 500
#define CLIENT_HEIGHT 300



template<class Interface>
inline void SafeRelease(
    Interface** ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != NULL)
    {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = NULL;
    }
}

#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif


class Direct2DApp
{
public:
    Direct2DApp();
    ~Direct2DApp();

    ATOM MyRegisterClass(HINSTANCE hInstance);

    BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

private:
    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    void DiscardDeviceResources();

    // Draw content.
    HRESULT OnRender();

    // Resize the render target.
    void OnResize(
        UINT width,
        UINT height
    );

    // The windows procedure.
    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    // Delcare Pointers
    HWND                            m_hwnd;

    ID2D1Factory*                   m_pDirect2dFactory;
    ID2D1HwndRenderTarget*          m_pRenderTarget;

    ID2D1SolidColorBrush*           m_pBlackBrush;
    ID2D1SolidColorBrush*           m_pWhiteBrush;
    ID2D1SolidColorBrush*           m_pCyanBrush;
    ID2D1SolidColorBrush*           m_pYellowBrush;
    ID2D1SolidColorBrush*           m_pMagentaBrush;
    ID2D1LinearGradientBrush*       m_pLinearGradientBrush;

    IDWriteFactory*                 m_pWriteTarget;
    IDWriteTextFormat*              m_pTextFormat;

    ID2D1GradientStopCollection*    m_pGradentStops;
};


//Direct2DApp::

Direct2DApp::Direct2DApp() :
    m_hwnd(NULL),

    m_pDirect2dFactory(NULL),
    m_pRenderTarget(NULL),

    m_pBlackBrush(NULL),
    m_pWhiteBrush(NULL),
    m_pCyanBrush(NULL),
    m_pYellowBrush(NULL),
    m_pMagentaBrush(NULL),
    m_pLinearGradientBrush(NULL),

    m_pWriteTarget(NULL),
    m_pTextFormat(NULL),

    m_pGradentStops(NULL)
{}

Direct2DApp::~Direct2DApp()
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


void Direct2DApp::DiscardDeviceResources()
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

void Direct2DApp::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here, because the error will be returned again
        // the next time EndDraw is called.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

HRESULT Direct2DApp::CreateDeviceIndependentResources()
{
    // Flag Varable
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT Direct2DApp::CreateDeviceResources()
{
    // Flag Varable
    HRESULT hr = S_OK;

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

    return hr;
}

HRESULT Direct2DApp::OnRender()
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
        float line_spacing = DRAW_WIDTH / ARRAY_LENGTH;
        float amplification = 1;
        float offset = DRAW_HEIGHT / 4;

        // Ping Properties
        int average_ping, highest_ping, lowest_ping, instability_ping;
        ping_stats(highest_ping, lowest_ping, average_ping, instability_ping);

        // Draw
        for (int i(ARRAY_LENGTH - 1); i > 0; i--) {
            m_pRenderTarget->DrawLine(
                D2D1::Point2F(DRAW_WIDTH - (i - 1) * line_spacing, DRAW_HEIGHT - ping_array[i - 1] * amplification),
                D2D1::Point2F(DRAW_WIDTH - (i)*line_spacing, DRAW_HEIGHT - ping_array[i] * amplification),
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