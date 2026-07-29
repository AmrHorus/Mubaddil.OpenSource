/**
 * @file popup.cpp
 * @brief Implementation of "Did You Mean?" popup window
 */

#include "popup.h"
#include <dwmapi.h>
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>
#include <thread>
#include <chrono>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "uxtheme.lib")

namespace mubaddil {

// Window class name
static const wchar_t* POPUP_CLASS_NAME = L"MubaddilCorrectionPopup";

CorrectionPopup::CorrectionPopup()
    : m_hInstance(nullptr)
    , m_hwnd(nullptr)
    , m_hwndOriginal(nullptr)
    , m_hwndArrow(nullptr)
    , m_hwndSuggested(nullptr)
    , m_hwndYesBtn(nullptr)
    , m_hwndNoBtn(nullptr)
    , m_hwndFocusedBtn(nullptr)
    , m_visible(false)
    , m_focusedIndex(0)
{
}

CorrectionPopup::~CorrectionPopup() {
    Shutdown();
}

CorrectionPopup& CorrectionPopup::Instance() {
    static CorrectionPopup instance;
    return instance;
}

bool CorrectionPopup::Initialize(HINSTANCE hInstance) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_hwnd) {
        return true; // Already initialized
    }
    
    m_hInstance = hInstance;
    
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
    wc.lpszClassName = POPUP_CLASS_NAME;
    
    ATOM atom = RegisterClassExW(&wc);
    if (!atom) {
        return false;
    }
    
    return true;
}

void CorrectionPopup::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    Hide();
    
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    
    UnregisterClassW(POPUP_CLASS_NAME, m_hInstance);
}

bool CorrectionPopup::Show(const std::wstring& originalWord,
                            const std::wstring& suggestedWord,
                            int x, int y,
                            ButtonCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_visible) {
        Hide();
    }
    
    m_originalWord = originalWord;
    m_suggestedWord = suggestedWord;
    m_callback = callback;
    m_focusedIndex = 0; // Focus Yes button by default
    
    // Create popup window if not exists
    if (!m_hwnd) {
        m_hwnd = CreateWindowExW(
            WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            POPUP_CLASS_NAME,
            L"Did you mean?",
            WS_POPUP | WS_BORDER,
            x, y, m_config.width, m_config.height,
            nullptr, nullptr, m_hInstance, nullptr
        );
        
        if (!m_hwnd) {
            return false;
        }
        
        ApplyFluentStyle();
        CreateButtons();
    }
    
    // Position the popup
    Reposition(x, y);
    
    // Update content
    SetWindowTextW(m_hwndOriginal, originalWord.c_str());
    SetWindowTextW(m_hwndSuggested, suggestedWord.c_str());
    
    // Show the window
    ShowWindow(m_hwnd, SW_SHOWNA);
    UpdateWindow(m_hwnd);
    
    m_visible = true;
    
    return true;
}

bool CorrectionPopup::Hide() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_visible || !m_hwnd) {
        return false;
    }
    
    ShowWindow(m_hwnd, SW_HIDE);
    m_visible = false;
    
    m_callback = nullptr;
    m_originalWord.clear();
    m_suggestedWord.clear();
    
    return true;
}

bool CorrectionPopup::IsVisible() const {
    return m_visible.load();
}

void CorrectionPopup::SetConfig(const PopupConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config = config;
}

PopupConfig CorrectionPopup::GetConfig() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_config;
}

void CorrectionPopup::SimulateButtonClick(PopupButton button) {
    if (button == PopupButton::Yes) {
        OnYesClicked();
    } else if (button == PopupButton::No) {
        OnNoClicked();
    }
}

bool CorrectionPopup::HandleKeyPress(int vkCode) {
    if (!m_visible) {
        return false;
    }
    
    switch (vkCode) {
        case VK_RETURN: // Enter - activate focused button
            if (m_focusedIndex == 0) {
                OnYesClicked();
            } else {
                OnNoClicked();
            }
            return true;
            
        case VK_ESCAPE: // Escape - No
            OnNoClicked();
            return true;
            
        case VK_LEFT:
        case VK_UP:
            m_focusedIndex = 0;
            SetFocus(m_hwndYesBtn);
            return true;
            
        case VK_RIGHT:
        case VK_DOWN:
            m_focusedIndex = 1;
            SetFocus(m_hwndNoBtn);
            return true;
            
        case VK_TAB:
            m_focusedIndex = (m_focusedIndex + 1) % 2;
            if (m_focusedIndex == 0) {
                SetFocus(m_hwndYesBtn);
            } else {
                SetFocus(m_hwndNoBtn);
            }
            return true;
    }
    
    return false;
}

HWND CorrectionPopup::GetWindowHandle() const {
    return m_hwnd;
}

void CorrectionPopup::Reposition(int x, int y) {
    if (!m_hwnd) {
        return;
    }
    
    // Ensure popup stays on screen
    RECT screenRect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);
    
    int newX = x;
    int newY = y;
    
    if (x + m_config.width > screenRect.right) {
        newX = screenRect.right - m_config.width;
    }
    
    if (y + m_config.height > screenRect.bottom) {
        newY = screenRect.bottom - m_config.height;
    }
    
    SetWindowPos(m_hwnd, nullptr, newX, newY, 
                 m_config.width, m_config.height,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT CALLBACK CorrectionPopup::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    auto& popup = Instance();
    
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Double buffering for smooth rendering
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memDC, memBitmap));
            
            // Fill background
            HBRUSH bgBrush = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &rc, bgBrush);
            DeleteObject(bgBrush);
            
            // Draw title
            SetBkMode(memDC, TRANSPARENT);
            SetTextColor(memDC, RGB(64, 64, 64));
            HFONT titleFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
            HFONT oldFont = static_cast<HFONT>(SelectObject(memDC, titleFont));
            TextOutW(memDC, 16, 12, L"Did you mean?", 15);
            SelectObject(memDC, oldFont);
            DeleteObject(titleFont);
            
            EndPaint(hwnd, &ps);
            
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            return 0;
        }
        
        case WM_CLOSE:
            popup.Hide();
            return 0;
            
        case WM_NCHITTEST:
            return HTTRANSPARENT; // Let clicks pass through non-client areas
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void CorrectionPopup::CreateButtons() {
    if (!m_hwnd) return;
    
    // Calculate button positions
    int btnWidth = 80;
    int btnHeight = 32;
    int spacing = 16;
    int totalWidth = btnWidth * 2 + spacing;
    int startY = m_config.height - btnHeight - 16;
    int startX = (m_config.width - totalWidth) / 2;
    
    // Yes button
    m_hwndYesBtn = CreateWindowExW(
        0, L"BUTTON", L"✔ Yes",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX, startY, btnWidth, btnHeight,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
    
    // No button
    m_hwndNoBtn = CreateWindowExW(
        0, L"BUTTON", L"✖ No",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        startX + btnWidth + spacing, startY, btnWidth, btnHeight,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
    
    // Word labels (using static controls)
    m_hwndOriginal = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        16, 40, m_config.width - 32, 32,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
    
    m_hwndArrow = CreateWindowExW(
        0, L"STATIC", L"↓",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        m_config.width / 2 - 8, 72, 16, 24,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
    
    m_hwndSuggested = CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        16, 96, m_config.width - 32, 32,
        m_hwnd, nullptr, m_hInstance, nullptr
    );
    
    // Set fonts
    HFONT font = CreateFontW(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    SendMessage(m_hwndOriginal, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessage(m_hwndArrow, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessage(m_hwndSuggested, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessage(m_hwndYesBtn, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessage(m_hwndNoBtn, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
}

void CorrectionPopup::OnYesClicked() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_callback) {
        m_callback(PopupButton::Yes);
    }
    
    Hide();
}

void CorrectionPopup::OnNoClicked() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_callback) {
        m_callback(PopupButton::No);
    }
    
    Hide();
}

void CorrectionPopup::ApplyFluentStyle() {
    if (!m_hwnd) return;
    
    // Enable rounded corners (Windows 11)
    MARGINS margins = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(m_hwnd, &margins);
    
    // Set window shadow
    DWORD shadow = 2; // Drop shadow
    DwmSetWindowAttribute(m_hwnd, DWMWA_NCRENDERING_POLICY, &shadow, sizeof(shadow));
    
    // Make background semi-transparent for glassmorphism
    COLORREF color = RGB(255, 255, 255);
    BYTE alpha = 230; // Slightly transparent
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA};
    
    SetLayeredWindowAttributes(m_hwnd, color, alpha, LWA_ALPHA);
}

} // namespace mubaddil
