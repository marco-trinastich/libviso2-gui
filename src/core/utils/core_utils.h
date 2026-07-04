#ifndef CORE_UTILS_H
#define CORE_UTILS_H

/******************************************************************
 *                                                                 *
 *  CoreUtils                                                          *
 *                                                                 *
 ******************************************************************/

#include <string>
#include <memory>
#include <functional>
#include <wtypes.h>

#include "models.h"

namespace VisoGui
{
    namespace Core
    {
        namespace Utils
        {
            class CoreUtils
            {
                using string = std::string;
                using PairOfInts = std::pair<int32_t, int32_t>;
                using PairOfStrings = std::pair<std::string, std::string>;
                using PairOfUint8Arrays = std::pair<std::unique_ptr<uint8_t[]>, std::unique_ptr<uint8_t[]>>;
                using VectorOfStringPairs = std::vector<std::pair<std::string, std::string>>;
                using CommandLineParseStatus = VisoGui::Core::Models::CommandLineParseStatus;
                using GetImageFilesStatus = VisoGui::Core::Models::GetImageFilesStatus;
                using ProcessingParams = VisoGui::Core::Models::ProcessingParams;

            public:
                static void NewThread(std::function<void()> callable);
                template <typename T>
                static inline void SafeRelease(T *&p)
                {
                    if (nullptr != p)
                    {
                        p->Release();
                        p = nullptr;
                    }
                }

                template <typename... Args>
                static string Format(const string &format, Args... args)
                {
                    auto format_c_str = format.c_str();

                    // Calculated formatted size; take into account extra space for '\0'
                    size_t size = snprintf(nullptr, 0, format_c_str, args...) + 1;
                    if (size <= 0)
                    {
                        throw std::runtime_error("Error during formatting.");
                    }
                    std::unique_ptr<char[]> buf(new char[size]);
                    snprintf(buf.get(), size, format_c_str, args...);

                    // Construct string from the buffer
                    return string(buf.get());
                }
                static bool StartsWith(const string &str, const string &prefix);
                static std::unique_ptr<WCHAR[]> GetWchar(const string &string);

                static bool CreateNewConsole(int16_t minLength);
                static bool AttachParentConsole(int16_t minLength);
                static bool ReleaseConsole();
                static void PrintInfo(const string &info);
                static void PrintError(const string &error);

                static CommandLineParseStatus ParseCommandLine(const string &lpCmdLine, ProcessingParams &params);
                static std::vector<string> GetTokensInString(const string &tokensString);
                static GetImageFilesStatus GetImageFiles(const ProcessingParams &params, VectorOfStringPairs &imageFiles);
                static bool GetImageDimension(const VectorOfStringPairs &imageFiles, PairOfInts &imageDimension);
                static bool GetImageData(const ProcessingParams &processingParams, const PairOfStrings &imagePair, PairOfUint8Arrays &imageData);

            private:
                static DWORD WINAPI ThreadTask(LPVOID lpParameter);
                static bool RedirectConsoleIO();
                static void AdjustConsoleBuffer(int16_t minLength);
            };
        }
    }
}

#endif // CORE_UTILS_H
