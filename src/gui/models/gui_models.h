#ifndef GUI_MODELS_H
#define GUI_MODELS_H

/******************************************************************
 *                                                                 *
 *  Gui Models                                                     *
 *                                                                 *
 ******************************************************************/

#include <string>
#include <functional>
#include <objidl.h>
#include <gdiPlus.h>

namespace VisoGui
{
    namespace Gui
    {
        namespace Models
        {
            // Paint function
            // (receives a GDI+ graphics object to paint on window or returns an image file name to render with D2D)
            class PaintFunction
            {
            public:
                PaintFunction(std::function<void(Gdiplus::Graphics &)> m_paintDrawFunction = nullptr,
                              std::function<std::string()> m_paintTargetFunction = nullptr)
                    : m_paintDrawFunction(m_paintDrawFunction),
                      m_paintTargetFunction(m_paintTargetFunction)
                {
                }

                ~PaintFunction() = default;

                void Draw(Gdiplus::Graphics &graphics)
                {
                    if (this->m_paintDrawFunction)
                    {
                        this->m_paintDrawFunction(graphics);
                    }
                }

                std::string Target()
                {
                    if (this->m_paintTargetFunction)
                    {
                        return this->m_paintTargetFunction();
                    }

                    return "";
                }

            private:
                std::function<void(Gdiplus::Graphics &)> m_paintDrawFunction;
                std::function<std::string()> m_paintTargetFunction;
            };

            class PaintDrawFunction : public PaintFunction
            {
            public:
                PaintDrawFunction(std::function<void(Gdiplus::Graphics &)> m_paintDrawFunction)
                    : PaintFunction(m_paintDrawFunction, nullptr)
                {
                }
            };

            class PaintTargetFunction : public PaintFunction
            {
            public:
                PaintTargetFunction(std::function<std::string()> m_paintTargetFunction)
                    : PaintFunction(nullptr, m_paintTargetFunction)
                {
                }
            };

            struct WindowLocation
            {
                int x;
                int y;
                int width;
                int height;

                WindowLocation(int x, int y, int width, int height)
                {
                    this->x = x;
                    this->y = y;
                    this->width = width;
                    this->height = height;
                }

                WindowLocation()
                {
                    this->x = 0;
                    this->y = 0;
                    this->width = 0;
                    this->height = 0;
                }
            };
        }
    }
}

#endif // GUI_MODELS_H