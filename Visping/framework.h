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

// Delcare Pointers
HWND                            m_hwnd(NULL);

ID2D1Factory* m_pDirect2dFactory(NULL);
ID2D1HwndRenderTarget* m_pRenderTarget(NULL);

ID2D1SolidColorBrush* m_pBlackBrush(NULL);
ID2D1SolidColorBrush* m_pWhiteBrush(NULL);
ID2D1SolidColorBrush* m_pCyanBrush(NULL);
ID2D1SolidColorBrush* m_pYellowBrush(NULL);
ID2D1SolidColorBrush* m_pMagentaBrush(NULL);
ID2D1LinearGradientBrush* m_pLinearGradientBrush(NULL);

IDWriteFactory* m_pWriteTarget(NULL);
IDWriteTextFormat* m_pTextFormat(NULL);

ID2D1GradientStopCollection* m_pGradentStops(NULL);