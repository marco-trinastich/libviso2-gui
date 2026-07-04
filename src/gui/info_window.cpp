#ifndef WINVER		  // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS		  // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE		 // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600 // Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#pragma comment(lib, "Gdiplus.lib")

#include "info_window.h"

using namespace Gdiplus;
using namespace VisoGui::Gui::Models;

using namespace VisoGui::Gui;

/******************************************************************
 *  Initialize members data                                       *
 ******************************************************************/

InfoWindow::InfoWindow()
	: BaseWindow(),
	  m_gdiplusStartupInput(),
	  m_gdiplusToken(),
	  m_paintFunction()
{
}

/******************************************************************
 *  Tear down resources                                            *
 ******************************************************************/

InfoWindow::~InfoWindow()
{
	// Call base destructor
	BaseWindow::~BaseWindow();

	// Release GDI+ resources
	GdiplusShutdown(this->m_gdiplusToken);
}

/******************************************************************
 *  Create application window and resources                        *
 ******************************************************************/

HRESULT InfoWindow::Initialize(HINSTANCE hInstance, char *m_szClassName, char *m_winTitle, WindowLocation location,
							   std::unique_ptr<PaintFunction> m_paintFunction)
{
	// Set custom paint function
	if (m_paintFunction)
	{
		this->m_paintFunction = std::move(m_paintFunction);
	}

	// Initialize GDI+
	if (GdiplusStartup(&this->m_gdiplusToken, &this->m_gdiplusStartupInput, nullptr) != Gdiplus::Ok)
	{
		MessageBox(nullptr, "GDI+ startup failed!", "Error!",
				   MB_ICONEXCLAMATION | MB_OK);
		return E_FAIL;
	}

	// Initialize underlying base window
	return BaseWindow::Initialize(
		hInstance,
		m_szClassName,
		m_winTitle,
		location);
}

/******************************************************************
 *  Internal Window message handler                                *
 ******************************************************************/

LRESULT InfoWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ERASEBKGND:
	{
		// Clear window to background color if requested

		HDC hdc = (HDC)wParam;
		HPEN erasePen = CreatePen(PS_SOLID, 1, this->m_backgroundColor);
		HBRUSH eraseBrush = CreateSolidBrush(this->m_backgroundColor);

		SelectObject(hdc, erasePen);
		SelectObject(hdc, eraseBrush);

		RECT rect;
		GetClientRect(hWnd, &rect);
		Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

		DeleteObject(eraseBrush);
		DeleteObject(erasePen);
		break;
	}
	default:
		return BaseWindow::WndProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

/******************************************************************
 *  Double buffered GDI+ paint function                           *
 ******************************************************************/

VOID InfoWindow::OnPaint(HWND hWnd)
{
	if (this->m_paintFunction)
	{
		// Double buffering paint

		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);

		RECT rect;
		GetClientRect(this->m_hWnd, &rect);

		// Create an off-screen bitmap and a memory device context
		HDC memDC = CreateCompatibleDC(hdc);
		HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right - rect.left, rect.bottom - rect.top);
		HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

		// Create a Graphics object from the memory device context
		Graphics graphics(memDC);

		// Clear the background
		Color backgroundColor;
		backgroundColor.SetFromCOLORREF(this->m_backgroundColor);
		graphics.Clear(backgroundColor);

		// Draw content on bitmap using the Graphics object
		this->m_paintFunction->Draw(graphics);

		// Copy the off-screen bitmap to the screen
		BitBlt(hdc, 0, 0, rect.right - rect.left, rect.bottom - rect.top, memDC, 0, 0, SRCCOPY);

		// Clean up
		SelectObject(memDC, oldBitmap);
		DeleteObject(memBitmap);
		DeleteDC(memDC);

		EndPaint(hWnd, &ps);
	}
}
