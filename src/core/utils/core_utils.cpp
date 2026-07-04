#include <iostream>
#include <filesystem>
#include <map>
#include <png.hpp>

#include "settings.h"
#include "core_utils.h"

using PairOfInts = std::pair<int32_t, int32_t>;
using PairOfStrings = std::pair<std::string, std::string>;
using PairOfUint8Arrays = std::pair<std::unique_ptr<uint8_t[]>, std::unique_ptr<uint8_t[]>>;
using VectorOfStringPairs = std::vector<std::pair<std::string, std::string>>;
using namespace std;
using namespace VisoGui::Core::Models;

using namespace VisoGui::Core::Utils;

// Threading

void CoreUtils::NewThread(function<void()> callable)
{
    auto data = std::make_unique<ThreadData>(callable);
    CreateThread(nullptr, 0, CoreUtils::ThreadTask, data.release(), 0, 0);
}

DWORD WINAPI CoreUtils::ThreadTask(LPVOID lpParameter)
{
    std::unique_ptr<ThreadData> data(static_cast<ThreadData *>(lpParameter));
    data->callable();
    return 0;
}

// Strings

bool CoreUtils::StartsWith(const std::string &str, const std::string &prefix)
{
    return str.compare(0, prefix.length(), prefix) == 0;
}

std::unique_ptr<WCHAR[]> CoreUtils::GetWchar(const std::string &string)
{
    auto c_string = string.c_str();
    int size_needed = MultiByteToWideChar(CP_ACP, 0, c_string, -1, nullptr, 0);
    std::unique_ptr<WCHAR[]> wstring(new WCHAR[size_needed]);
    MultiByteToWideChar(CP_ACP, 0, c_string, -1, wstring.get(), size_needed);
    return wstring;
}

// Console

bool CoreUtils::CreateNewConsole(int16_t minLength)
{
    bool result = false;

    // Release any current console and redirect IO to NUL
    ReleaseConsole();

    // Attempt to create new console
    if (AllocConsole())
    {
        AdjustConsoleBuffer(minLength);
        result = RedirectConsoleIO();
    }

    return result;
}

bool CoreUtils::AttachParentConsole(int16_t minLength)
{
    bool result = false;

    // Release any current console and redirect IO to NUL
    ReleaseConsole();

    // Attempt to attach to parent process's console
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        AdjustConsoleBuffer(minLength);
        result = RedirectConsoleIO();
    }

    return result;
}

bool CoreUtils::ReleaseConsole()
{
    bool result = true;
    FILE *fp;

    // Just to be safe, redirect standard IO to NUL before releasing.

    // Redirect STDIN to NUL
    if (freopen_s(&fp, "NUL:", "r", stdin) != 0)
        result = false;
    else
        setvbuf(stdin, nullptr, _IONBF, 0);

    // Redirect STDOUT to NUL
    if (freopen_s(&fp, "NUL:", "w", stdout) != 0)
        result = false;
    else
        setvbuf(stdout, nullptr, _IONBF, 0);

    // Redirect STDERR to NUL
    if (freopen_s(&fp, "NUL:", "w", stderr) != 0)
        result = false;
    else
        setvbuf(stderr, nullptr, _IONBF, 0);

    // Detach from console
    if (!FreeConsole())
        result = false;

    return result;
}

bool CoreUtils::RedirectConsoleIO()
{
    bool result = true;
    FILE *fp;

    // Redirect STDIN if the console has an input handle
    if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONIN$", "r", stdin) != 0)
            result = false;
        else
            setvbuf(stdin, nullptr, _IONBF, 0);

    // Redirect STDOUT if the console has an output handle
    if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
            result = false;
        else
            setvbuf(stdout, nullptr, _IONBF, 0);

    // Redirect STDERR if the console has an error handle
    if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
        if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
            result = false;
        else
            setvbuf(stderr, nullptr, _IONBF, 0);

    // Make C++ standard streams point to console as well.
    ios::sync_with_stdio(true);

    // Clear the error state for each of the C++ standard streams.
    wcout.clear();
    cout.clear();
    wcerr.clear();
    cerr.clear();
    wcin.clear();
    cin.clear();

    return result;
}

void CoreUtils::AdjustConsoleBuffer(int16_t minLength)
{
    // Set the screen buffer to be big enough to scroll some text
    CONSOLE_SCREEN_BUFFER_INFO conInfo;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
    if (conInfo.dwSize.Y < minLength)
        conInfo.dwSize.Y = minLength;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
}

void CoreUtils::PrintInfo(const string &info)
{
    if (Settings::Core::isConsoleEnabled)
    {
        cout << info << endl;
    }
}

void CoreUtils::PrintError(const string &error)
{
    if (Settings::Core::isConsoleEnabled)
    {
        cerr << error << endl;
    }
}

// Processing

CommandLineParseStatus CoreUtils::ParseCommandLine(const string &lpCmdLine, ProcessingParams &params)
{
    // Split lpCmdLine by spaces
    vector<string> tokens = GetTokensInString(lpCmdLine);

    bool imagesPathFound = false;

    size_t leftPrefixLength = Settings::Core::leftImagesPrefixParam.length();
    size_t rightPrefixLength = Settings::Core::rightImagesPrefixParam.length();

    // Parse tokens
    for (const auto &t : tokens)
    {
        if (CoreUtils::StartsWith(t, Settings::Core::leftImagesPrefixParam))
        {
            params.imagesPrefixLeft = t.substr(leftPrefixLength);
        }
        else if (CoreUtils::StartsWith(t, Settings::Core::rightImagesPrefixParam))
        {
            params.imagesPrefixRight = t.substr(rightPrefixLength);
        }
        else if (CoreUtils::StartsWith(t, Settings::Core::helpParam))
        {
            return CommandLineParseStatus::ShowHelp;
        }
        else
        {
            if (!imagesPathFound)
            {
                params.imagesPath = t;
                imagesPathFound = true;
            }
            else
            {
                return CommandLineParseStatus::MultiplePathFound;
            }
        }
    }

    // Check if images path is found
    if (!imagesPathFound)
    {
        return CommandLineParseStatus::NoPathFound;
    }

    // Check if prefixes have been set
    if (params.imagesPrefixLeft.empty())
    {
        params.imagesPrefixLeft = Settings::Core::leftImagesDefaultPrefix;
    }

    if (params.imagesPrefixRight.empty())
    {
        params.imagesPrefixRight = Settings::Core::rightImagesDefaultPrefix;
    }

    return CommandLineParseStatus::Success;
}

vector<string> CoreUtils::GetTokensInString(const string &tokensString)
{
    // Split string by spaces, considering quoted strings as single tokens
    vector<string> tokens;
    string token;
    bool inQuotes = false;

    for (size_t i = 0; i < tokensString.length(); ++i)
    {
        char c = tokensString[i];

        if (c == '"')
        {
            inQuotes = !inQuotes;
        }
        else if (isspace(c) && !inQuotes)
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            token += c;
        }
    }

    // Add the last token if it's not empty
    if (!token.empty())
    {
        tokens.push_back(token);
    }

    return tokens;
}

GetImageFilesStatus CoreUtils::GetImageFiles(const ProcessingParams &params, VectorOfStringPairs &imageFiles)
{
    // Map to store filenames without prefixes and their corresponding left and right image paths
    map<string, pair<string, string>> imagesMap;

    try
    {
        for (const auto &entry : filesystem::directory_iterator(params.imagesPath))
        {
            if (entry.is_regular_file() && entry.path().extension() == Settings::Core::imagesExtension)
            {
                string file = entry.path().string();
                string filename = entry.path().filename().string();

                if (StartsWith(filename, params.imagesPrefixLeft))
                {
                    string key = filename.substr(params.imagesPrefixLeft.length());
                    imagesMap[key].first = file;
                }
                else if (StartsWith(filename, params.imagesPrefixRight))
                {
                    string key = filename.substr(params.imagesPrefixRight.length());
                    imagesMap[key].second = file;
                }
            }
        }
    }
    catch (...)
    {
        return GetImageFilesStatus::InvalidPath;
    }

    // Build the vector of pairs of strings
    for (const auto &entry : imagesMap)
    {
        const auto &leftImage = entry.second.first;
        const auto &rightImage = entry.second.second;

        if (leftImage.empty() || rightImage.empty())
        {
            return GetImageFilesStatus::ImagesNumberMismatch;
        }

        imageFiles.emplace_back(leftImage, rightImage);
    }

    if (imageFiles.empty())
    {
        return GetImageFilesStatus::NoImageFound;
    }

    return GetImageFilesStatus::Success;
}

bool CoreUtils::GetImageDimension(const VectorOfStringPairs &imageFiles, PairOfInts &imageDimension)
{
    // image dimensions
    int32_t width;
    int32_t height;

    try
    {
        // load an input image
        png::image<png::gray_pixel> image(imageFiles[0].first);

        // image dimensions
        width = image.get_width();
        height = image.get_height();
    }
    catch (...)
    {
        // catch image read/write errors
        return false;
    }

    // Set the image dimensions
    imageDimension = {width, height};

    return true;
}

bool CoreUtils::GetImageData(const ProcessingParams &processingParams, const PairOfStrings &imagePair, PairOfUint8Arrays &imageData)
{

    // catch image read/write errors here
    try
    {
        // load left and right input image
        png::image<png::gray_pixel> leftImage(imagePair.first);
        png::image<png::gray_pixel> rightImage(imagePair.second);

        int width = processingParams.imageDimension.first;
        int height = processingParams.imageDimension.second;

        // convert input images to uint8_t buffer
        int32_t k = 0;
        for (int32_t v = 0; v < height; v++)
        {
            for (int32_t u = 0; u < width; u++)
            {
                imageData.first[k] = leftImage.get_pixel(u, v);
                imageData.second[k] = rightImage.get_pixel(u, v);
                k++;
            }
        }
    }
    catch (...)
    {
        return false;
    }

    return true;
}
