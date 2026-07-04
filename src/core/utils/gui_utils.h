#ifndef GUI_UTILS_H
#define GUI_UTILS_H

/******************************************************************
 *                                                                 *
 *  GUI Utils                                                      *
 *                                                                 *
 ******************************************************************/

#include <objidl.h>
#include <gdiplus.h>

namespace VisoGui
{
    namespace Core
    {
        namespace Utils
        {
            class GuiUtils
            {
            public:
                static void DrawLine(Gdiplus::Graphics &graphics, int x1, int y1, int x2, int y2);
                static void DrawRectangle(Gdiplus::Graphics &graphics, int x1, int y1, int width, int height);
                static void DrawString(Gdiplus::Graphics &graphics, const std::string &text, int x, int y);
                static void ClearArea(Gdiplus::Graphics &graphics, int x1, int y1, int width, int height, COLORREF color);

            private:
                static std::unique_ptr<Gdiplus::Pen> CreateDefaultPen();
                static std::unique_ptr<Gdiplus::Pen> CreateLinePen();
                static std::unique_ptr<Gdiplus::SolidBrush> CreateDefaultBrush();
                static std::unique_ptr<Gdiplus::SolidBrush> CreateBrushFromColor(COLORREF color);
                static std::unique_ptr<Gdiplus::Font> CreateDefaultFont();
            };
        }
    }
}

#endif // GUI_UTILS_H
