#pragma once
#ifndef VIDEO_WINDOW_H
#define VIDEO_WINDOW_H

/******************************************************************
 *                                                                 *
 *  VideoWindow                                                    *
 *                                                                 *
 ******************************************************************/

#include <memory>
#include <wincodec.h>
#include <d2d1.h>

#include "base_window.h"

namespace VisoGui
{
	namespace Gui
	{
		// Default DPI that maps image resolution directly to screen resoltuion
		const FLOAT DEFAULT_DPI = 96.f;

		class VideoWindow : public BaseWindow
		{

		public:
			VideoWindow();
			~VideoWindow() override;
			HRESULT Initialize(HINSTANCE hInstance, char *m_szClassName, char *m_winTitle, Models::WindowLocation location,
							   std::unique_ptr<Models::PaintFunction> m_paintFunction);

		protected:
			LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
			VOID OnPaint(HWND hWnd) override;

		private:
			BOOL DisplayImage(HWND hWnd, WCHAR *fname);
			HRESULT CreateDeviceResources(HWND hWnd);
			HRESULT CreateD2DBitmapFromFile(HWND hWnd, WCHAR *fname);
			LRESULT RenderD2D(HWND hWnd);

		private:
			ID2D1HwndRenderTarget *m_pRT;							// D2D Render Target
			ID2D1Factory *m_pD2DFactory;							// D2D Factory
			IWICImagingFactory *m_pIWICFactory;						// WIC Imaging Factory
			IWICFormatConverter *m_pConvertedSourceBitmap;			// WIC Format Converter
			ID2D1Bitmap *m_pD2DBitmap;								// D2D Bitmap
			std::unique_ptr<WCHAR[]> m_renderedImage;				// Current rendered image
			BOOL m_newRendering;									// Whether is a new rendering
			std::unique_ptr<Models::PaintFunction> m_paintFunction; // Current Window paint function
		};
	}
}

#endif // VIDEO_WINDOW_H
