#include "base_window.h"

using namespace VisoGui::Gui::Models;

using namespace VisoGui::Gui;

/******************************************************************
 *  Initialize members data                                       *
 ******************************************************************/

BaseWindow::BaseWindow()
    : m_hWnd(nullptr),
      m_szClassName(nullptr),
      m_winTitle("Default Window"),
      m_backgroundColor(0x00FFFFFF) {};

/******************************************************************
 *  Tear down resources                                            *
 ******************************************************************/

BaseWindow::~BaseWindow()
{
    this->StopTimer();
}

/******************************************************************
 *  Create application window                                      *
 ******************************************************************/

HRESULT BaseWindow::Initialize(HINSTANCE hInstance, char *m_szClassName, char *m_winTitle, WindowLocation location)
{
    // Set Window class name
    if (!m_szClassName)
    {
        MessageBox(nullptr, "Cannot create a new window without specifying a class name!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return E_FAIL;
    }
    this->m_szClassName = m_szClassName;

    // Set Window title
    if (m_winTitle)
    {
        this->m_winTitle = m_winTitle;
    }

    // Register the Window Class
    WNDCLASSEX m_wcls;
    m_wcls.cbSize = sizeof(WNDCLASSEX);
    m_wcls.style = CS_HREDRAW | CS_VREDRAW;
    m_wcls.lpfnWndProc = BaseWindow::s_WndProc;
    m_wcls.cbClsExtra = 0;
    m_wcls.cbWndExtra = sizeof(LONG_PTR);
    m_wcls.hInstance = hInstance;
    m_wcls.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    m_wcls.hCursor = LoadCursor(nullptr, IDC_ARROW);
    m_wcls.hbrBackground = CreateSolidBrush(this->m_backgroundColor);
    m_wcls.lpszMenuName = nullptr;
    m_wcls.lpszClassName = this->m_szClassName;
    m_wcls.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    if (!RegisterClassEx(&m_wcls))
    {
        MessageBox(nullptr, "Window registration failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return E_FAIL;
    }

    // Create the Window
    this->m_hWnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        this->m_szClassName,
        this->m_winTitle,
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        location.x,
        location.y,
        location.width,
        location.height,
        nullptr,
        nullptr,
        hInstance,
        this);

    if (!m_hWnd)
    {
        MessageBox(nullptr, "Window creation failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return E_FAIL;
    }

    // Show the Window
    ShowWindow(this->m_hWnd, SW_SHOW);
    if (!UpdateWindow(this->m_hWnd))
    {
        MessageBox(nullptr, "Window show failed!", "Error!",
                   MB_ICONEXCLAMATION | MB_OK);
        return E_FAIL;
    }

    // Start rendering timer
    this->StartTimer();

    return S_OK;
}

/******************************************************************
 *  Regsitered Window message handler                              *
 ******************************************************************/

LRESULT CALLBACK BaseWindow::s_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BaseWindow *pThis;
    LRESULT lRet = 0;

    if (uMsg == WM_NCCREATE)
    {
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        pThis = reinterpret_cast<BaseWindow *>(pcs->lpCreateParams);

        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    else
    {
        pThis = reinterpret_cast<BaseWindow *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (pThis)
        {
            lRet = pThis->WndProc(hWnd, uMsg, wParam, lParam);
        }
        else
        {
            lRet = DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }
    return lRet;
}

/******************************************************************
 *  Internal Window message handler                                *
 ******************************************************************/

LRESULT BaseWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_PAINT:
        this->OnPaint(hWnd);
        break;
    case WM_TIMER:
        this->OnTimer();
        break;
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

VOID BaseWindow::OnTimer()
{
    // Trigger repaint
    InvalidateRect(this->m_hWnd, nullptr, FALSE);
}

VOID BaseWindow::StartTimer()
{
    SetTimer(this->m_hWnd, 1, 16, nullptr); // 16 ms for ~60 FPS
}

VOID BaseWindow::StopTimer()
{
    KillTimer(this->m_hWnd, 1);
}

/*
// Custom background colors (win startup phase)
// (HBRUSH)GetStockObject(BLACK_BRUSH/GRAY_BRUSH/WHITE_BRUSH)
*/

/*
// Create button
        const int button_id = 1;
        HWND button = CreateWindowEx(WS_EX_CLIENTEDGE, "Button", "OK", WS_CHILD | WS_VISIBLE,
                                     10, 100, 100, 25, m_hWnd, (HMENU)button_id, nullptr, nullptr);
*/

/*
// WndProc Function to manage Button Click
    case WM_COMMAND:
        if (((HWND)lParam) && (HIWORD(wParam) == BN_CLICKED))
        {
            int iMID;
            iMID = LOWORD(wParam);
            switch (iMID)
            {
            case 1:
                MessageBox(hWnd, (LPCTSTR) "You just pushed me!", (LPCTSTR) "My Program!", MB_OK | MB_ICONEXCLAMATION);
                break;
            case 2:
                PostMessage(hWnd, WM_CLOSE, 0, 0);
                break;
            default:
                break;
            }
        }
        break;
*/

/*
// WndProc function to manage LBUTTONDOWN event (left mouse button click on window)
    case WM_LBUTTONDOWN:
        m_backgroundColor = 0x00110011;
        // Invalidates window background and launch WM_ERASEBKGND event
        InvalidateRect(hWnd, nullptr, TRUE);
        break;
*/
