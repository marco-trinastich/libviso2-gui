#ifndef GUI_HANDLER_H
#define GUI_HANDLER_H

/******************************************************************
 *                                                                 *
 *  Gui Handler                                                    *
 *                                                                 *
 ******************************************************************/

#include "state.h"
#include "info_window.h"
#include "video_window.h"

namespace VisoGui
{
    namespace Core
    {
        namespace Handlers
        {
            class GuiHandler
            {
            public:
                GuiHandler(const VisoGui::Core::Models::State &state);
                void Initialize(HINSTANCE hInstance);

            private:
                void OnInfoPaint(Gdiplus::Graphics &graphics);
                void DrawVOParams(Gdiplus::Graphics &graphics);
                void DrawVOStats(Gdiplus::Graphics &graphics);
                void DrawVOGraph(Gdiplus::Graphics &graphics);

            private:
                const VisoGui::Core::Models::State &state;
                VisoGui::Gui::InfoWindow infoWindow;
                VisoGui::Gui::VideoWindow videoWindow;
            };
        }
    }
}

#endif // GUI_HANDLER_H