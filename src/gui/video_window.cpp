#ifndef WINVER        // Allow use of features specific to Windows XP or later.
#define WINVER 0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT        // Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501 // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINDOWS        // Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE        // Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600 // Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#pragma comment(lib, "d2d1.lib")

#include "core_utils.h"
#include "video_window.h"

using namespace VisoGui::Core::Utils;
using namespace VisoGui::Gui::Models;

using namespace VisoGui::Gui;

/******************************************************************
 *  Initialize members data                                       *
 ******************************************************************/

VideoWindow::VideoWindow()
    : BaseWindow(),
      m_pRT(nullptr),
      m_pD2DFactory(nullptr),
      m_pIWICFactory(nullptr),
      m_pConvertedSourceBitmap(nullptr),
      m_pD2DBitmap(nullptr),
      m_renderedImage(CoreUtils::GetWchar("")),
      m_newRendering(FALSE),
      m_paintFunction()
{
}

/******************************************************************
 *  Tear down resources                                            *
 ******************************************************************/

VideoWindow::~VideoWindow()
{
    // Call base destructor
    BaseWindow::~BaseWindow();

    // Release D2D/IWIC resources
    if (this->m_pD2DBitmap)
        CoreUtils::SafeRelease(this->m_pD2DBitmap);
    if (this->m_pConvertedSourceBitmap)
        CoreUtils::SafeRelease(this->m_pConvertedSourceBitmap);
    if (this->m_pIWICFactory)
        CoreUtils::SafeRelease(this->m_pIWICFactory);
    if (this->m_pD2DFactory)
        CoreUtils::SafeRelease(this->m_pD2DFactory);
    if (this->m_pRT)
        CoreUtils::SafeRelease(this->m_pRT);
}

/******************************************************************
 *  Create application window and resources                        *
 ******************************************************************/

HRESULT VideoWindow::Initialize(HINSTANCE hInstance, char *m_szClassName, char *m_winTitle, WindowLocation location,
                                std::unique_ptr<PaintFunction> m_paintFunction)
{
    // Set custom paint function
    if (m_paintFunction)
    {
        this->m_paintFunction = std::move(m_paintFunction);
    }

    // Create WIC factory
    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&this->m_pIWICFactory));

    if (!SUCCEEDED(hr))
    {
        MessageBox(nullptr, "WIC Factory creation failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);

        return E_FAIL;
    }

    // Create D2D factory
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &this->m_pD2DFactory);

    if (!SUCCEEDED(hr))
    {
        MessageBox(nullptr, "D2D Factory creation failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);

        return E_FAIL;
    }

    // Initialize underlying base window
    return BaseWindow::Initialize(
        hInstance,
        m_szClassName,
        m_winTitle,
        location);
}

/******************************************************************
 *  Internal Window message handler                                *
 ******************************************************************/

LRESULT VideoWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
    {
        D2D1_SIZE_U size = D2D1::SizeU(LOWORD(lParam), HIWORD(lParam));

        if (this->m_pRT)
        {
            // If we couldn't resize, release the device and we'll recreate it
            // during the next render pass.
            if (FAILED(m_pRT->Resize(size)))
            {
                CoreUtils::SafeRelease(this->m_pRT);
                CoreUtils::SafeRelease(this->m_pD2DBitmap);
            }
        }
        break;
    }
    case WM_ERASEBKGND:
        // Never perform erase background (to keep rendered image visible)
        return 0;
        break;
    default:
        return BaseWindow::WndProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/******************************************************************
 *  Direct2D target image rendering function                      *
 ******************************************************************/

VOID VideoWindow::OnPaint(HWND hWnd)
{
    if (this->m_paintFunction)
    {
        // Use paint function to read target image to render
        auto imageFileName = CoreUtils::GetWchar(this->m_paintFunction->Target());

        // If it's the first image of a series flag new rendering
        if (wcscmp(this->m_renderedImage.get(), L"") == 0)
        {
            this->m_newRendering = TRUE;
        }

        // If the new image is not empty, render it
        this->m_renderedImage = std::move(imageFileName);
        if (wcscmp(this->m_renderedImage.get(), L"") != 0)
        {
            this->DisplayImage(hWnd, this->m_renderedImage.get());
        }
    }
}

/******************************************************************
 *  Display image file at location fname							  *
 *  1. Use CreateD2DBitmapFromFile to generate the D2D Rendering	  *
 *     Bitmap from the image file fname							  *
 *  2. Use RenderD2D to display the image on the Window hWnd		  *
 ******************************************************************/

BOOL VideoWindow::DisplayImage(HWND hWnd, WCHAR *fname)
{
    if (SUCCEEDED(CreateD2DBitmapFromFile(hWnd, fname)))
    {
        RenderD2D(hWnd);
        return TRUE;
    }
    else
    {
        MessageBox(hWnd, "Failed to load image.", "Application Error", MB_ICONEXCLAMATION | MB_OK);
        return FALSE;
    }
}

/******************************************************************
 *  This method creates resources which are bound to a particular  *
 *  D2D device. It's all centralized here, in case the resources   *
 *  need to be recreated in the event of D2D device loss           *
 * (e.g. display change, remoting, removal of video card, etc).    *
 ******************************************************************/

HRESULT VideoWindow::CreateDeviceResources(HWND hWnd)
{
    HRESULT hr = S_OK;

    if (!this->m_pRT)
    {
        RECT rc;
        hr = GetClientRect(hWnd, &rc) ? S_OK : E_FAIL;

        if (SUCCEEDED(hr))
        {
            // Create a D2D render target properties
            D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties();

            // Set the DPI to be the default system DPI to allow direct mapping
            // between image pixels and desktop pixels in different system DPI settings
            renderTargetProperties.dpiX = DEFAULT_DPI;
            renderTargetProperties.dpiY = DEFAULT_DPI;

            // Create a D2D render target
            D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

            hr = this->m_pD2DFactory->CreateHwndRenderTarget(
                renderTargetProperties,
                D2D1::HwndRenderTargetProperties(hWnd, size),
                &this->m_pRT);
        }
    }

    return hr;
}

/******************************************************************
 *  Load an image file and create an D2DBitmap                     *
 ******************************************************************/

HRESULT VideoWindow::CreateD2DBitmapFromFile(HWND hWnd, WCHAR *fname)
{
    HRESULT hr = S_OK;

    // Step 1: Decode the source image

    // Create a decoder
    IWICBitmapDecoder *pDecoder = nullptr;

    hr = this->m_pIWICFactory->CreateDecoderFromFilename(
        fname,                          // Image to be decoded
        nullptr,                        // Do not prefer a particular vendor
        GENERIC_READ,                   // Desired read access to the file
        WICDecodeMetadataCacheOnDemand, // Cache metadata when needed
        &pDecoder                       // Pointer to the decoder
    );

    // Retrieve the first frame of the image from the decoder
    IWICBitmapFrameDecode *pFrame = nullptr;

    if (SUCCEEDED(hr))
    {
        hr = pDecoder->GetFrame(0, &pFrame);
    }

    // Only at first rendering, get Frame and Screen Dimensions,
    // and Move/Resize Window
    if (this->m_newRendering)
    {
        UINT FrameX, FrameY;
        pFrame->GetSize(&FrameX, &FrameY);
        RECT working_area;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &working_area, 0);

        SetWindowPos(hWnd, 0, working_area.right - FrameX, working_area.bottom - FrameY,
                     FrameX, FrameY, SWP_NOZORDER | SWP_NOACTIVATE);

        this->m_newRendering = FALSE;
    }

    // Step 2: Format convert the frame to 32bppPBGRA
    if (SUCCEEDED(hr))
    {
        CoreUtils::SafeRelease(this->m_pConvertedSourceBitmap);
        hr = this->m_pIWICFactory->CreateFormatConverter(&this->m_pConvertedSourceBitmap);
    }

    if (SUCCEEDED(hr))
    {
        hr = this->m_pConvertedSourceBitmap->Initialize(
            pFrame,                        // Input bitmap to convert
            GUID_WICPixelFormat32bppPBGRA, // Destination pixel format
            WICBitmapDitherTypeNone,       // Specified dither pattern
            nullptr,                       // Specify a particular palette
            0.f,                           // Alpha threshold
            WICBitmapPaletteTypeCustom     // Palette translation type
        );
    }

    // Step 3: Create render target and D2D bitmap from IWICBitmapSource
    if (SUCCEEDED(hr))
    {
        hr = CreateDeviceResources(hWnd);
    }

    if (SUCCEEDED(hr))
    {
        // Need to release the previous D2DBitmap if there is one
        CoreUtils::SafeRelease(this->m_pD2DBitmap);
        hr = this->m_pRT->CreateBitmapFromWicBitmap(this->m_pConvertedSourceBitmap, nullptr, &this->m_pD2DBitmap);
    }

    CoreUtils::SafeRelease(pDecoder);
    CoreUtils::SafeRelease(pFrame);

    return hr;
}

/******************************************************************
 * Rendering routine using D2D                                     *
 ******************************************************************/
LRESULT VideoWindow::RenderD2D(HWND hWnd)
{
    HRESULT hr = S_OK;
    PAINTSTRUCT ps;

    if (BeginPaint(hWnd, &ps))
    {
        // Create render target if not yet created
        hr = CreateDeviceResources(hWnd);

        if (SUCCEEDED(hr) && !(this->m_pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            this->m_pRT->BeginDraw();

            this->m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

            // Clear the background
            this->m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));

            D2D1_SIZE_F rtSize = this->m_pRT->GetSize();

            // Create a rectangle same size of current window
            D2D1_RECT_F rectangle = D2D1::RectF(0.0f, 0.0f, rtSize.width, rtSize.height);

            // D2DBitmap may have been released due to device loss.
            // If so, re-create it from the source bitmap
            if (this->m_pConvertedSourceBitmap && !this->m_pD2DBitmap)
            {
                this->m_pRT->CreateBitmapFromWicBitmap(this->m_pConvertedSourceBitmap, nullptr, &this->m_pD2DBitmap);
            }

            // Draws an image and scales it to the current window size
            if (this->m_pD2DBitmap)
            {
                this->m_pRT->DrawBitmap(this->m_pD2DBitmap, rectangle);
            }

            hr = this->m_pRT->EndDraw();

            // In case of device loss, discard D2D render target and D2DBitmap
            // They will be re-created in the next rendering pass
            if (hr == D2DERR_RECREATE_TARGET)
            {
                CoreUtils::SafeRelease(this->m_pD2DBitmap);
                CoreUtils::SafeRelease(this->m_pRT);
                // Force a re-render
                hr = InvalidateRect(hWnd, nullptr, TRUE) ? S_OK : E_FAIL;
            }
        }

        EndPaint(hWnd, &ps);
    }

    return SUCCEEDED(hr) ? 0 : 1;
}
