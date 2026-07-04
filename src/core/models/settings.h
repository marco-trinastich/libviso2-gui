#ifndef SETTINGS_H
#define SETTINGS_H

/******************************************************************
 *                                                                 *
 *  Settings                                                       *
 *                                                                 *
 ******************************************************************/

#include <string>

namespace VisoGui
{
    namespace Core
    {
        namespace Models
        {
            class Settings
            {
            public:
                class Core
                {
                public:
                    static const bool isConsoleEnabled = true;
                    static const int maxConsoleLines = 1024;
                    static const inline std::string imagesExtension = ".png";
                    static const inline std::string helpParam = "/h";
                    static const inline std::string leftImagesPrefixParam = "/p1=";
                    static const inline std::string leftImagesDefaultPrefix = "I1_";
                    static const inline std::string rightImagesPrefixParam = "/p2=";
                    static const inline std::string rightImagesDefaultPrefix = "I2_";
                };

                class Gui
                {
                public:
                    // Graph drawing
                    static const int xPadding = 20;
                    static const int xOffset = 205 + xPadding;
                    static const int zOffset = 240;
                    static const int fOffset = 35 + xPadding;
                    static const int yOffset = 500;
                    static const int pointThickness = 3;

                    // Info text
                    static const int xInfoText = 430 + xPadding;
                    static const int textHeight = 15;
                };
            };
        }
    }
}

#endif // SETTINGS_H
