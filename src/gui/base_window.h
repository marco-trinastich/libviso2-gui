#ifndef BASE_WINDOW_H
#define BASE_WINDOW_H

/******************************************************************
 *                                                                 *
 *  Base Window (abstract window definition)                       *
 *                                                                 *
 ******************************************************************/

#include <memory>
#include <functional>
#include "wtypes.h"

#include "gui_models.h"

namespace VisoGui
{
    namespace Gui
    {
        class BaseWindow
        {

        public:
            BaseWindow();
            virtual ~BaseWindow();
            virtual HRESULT Initialize(HINSTANCE hInstance, char *m_szClassName, char *m_winTitle, Models::WindowLocation location,
                                       std::unique_ptr<Models::PaintFunction> m_paintFunction) = 0;

        protected:
            HRESULT Initialize(HINSTANCE hInstance, char *m_szClassName, char *m_winTitle, Models::WindowLocation location);
            static LRESULT CALLBACK s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
            virtual LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

            virtual VOID OnPaint(HWND hWnd) = 0;
            VOID OnTimer();
            VOID StartTimer();
            VOID StopTimer();

        protected:
            HWND m_hWnd;                // Current Window Handle
            char *m_szClassName;        // Current instance of Window Class name
            char *m_winTitle;           // Current Window Title
            COLORREF m_backgroundColor; // Current Window Background color
        };
    }
}

#endif // BASE_WINDOW_H
