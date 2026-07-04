#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

/******************************************************************
 *                                                                 *
 *  InfoWindow                                                     *
 *                                                                 *
 ******************************************************************/

#include <memory>
#include <functional>
#include <objidl.h>
#include <gdiplus.h>

#include "base_window.h"

namespace VisoGui
{
	namespace Gui
	{
		class InfoWindow : public BaseWindow
		{

		public:
			InfoWindow();
			~InfoWindow() override;
			HRESULT Initialize(HINSTANCE hInstance, char *m_szClassName, char *m_winTitle, Models::WindowLocation location,
							   std::unique_ptr<Models::PaintFunction> m_paintFunction);

		protected:
			LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
			VOID OnPaint(HWND hWnd) override;

		private:
			Gdiplus::GdiplusStartupInput m_gdiplusStartupInput;		// Gdiplus Startup Input
			ULONG_PTR m_gdiplusToken;								// Gdiplus Token
			std::unique_ptr<Models::PaintFunction> m_paintFunction; // Current Window paint function
		};
	}
}

#endif // INFO_WINDOW_H
