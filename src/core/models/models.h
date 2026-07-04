#ifndef MODELS_H
#define MODELS_H

/******************************************************************
 *                                                                 *
 *  Models                                                         *
 *                                                                 *
 ******************************************************************/

#include <functional>

namespace VisoGui
{
    namespace Core
    {
        namespace Models
        {
            struct ThreadData
            {
                std::function<void()> callable;
                ThreadData(std::function<void()> callable) : callable(callable) {}
            };

            struct ProcessingParams
            {
                std::string imagesPath;
                std::string imagesPrefixLeft;
                std::string imagesPrefixRight;
                std::vector<std::pair<std::string, std::string>> imageFiles;
                std::pair<int32_t, int32_t> imageDimension;
            };

            enum class CommandLineParseStatus
            {
                MultiplePathFound,
                NoPathFound,
                ShowHelp,
                Success
            };

            enum class GetImageFilesStatus
            {
                InvalidPath,
                NoImageFound,
                ImagesNumberMismatch,
                Success
            };
        }
    }
}

#endif // MODELS_H
