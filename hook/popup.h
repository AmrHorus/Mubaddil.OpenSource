#pragma once

/**
 * @file popup.h
 * @brief "Did You Mean?" popup window with Fluent Design
 * 
 * Displays a modern, non-intrusive popup suggesting corrections
 * for words typed with the wrong keyboard layout.
 */

#ifndef POPUP_H
#define POPUP_H

#include <windows.h>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

namespace mubaddil {

/**
 * @enum PopupButton
 * @brief Buttons available in the popup
 */
enum class PopupButton {
    None,
    Yes,
    No
};

/**
 * @struct PopupConfig
 * @brief Configuration options for the popup
 */
struct PopupConfig {
    int width;                    ///< Popup width in pixels
    int height;                   ///< Popup height in pixels
    int cornerRadius;             ///< Corner radius for rounded corners
    int showDelayMs;              ///< Delay before showing (ms)
    int autoHideMs;               ///< Auto-hide timeout (0 = disabled)
    bool enableBlur;              ///< Enable glassmorphism blur effect
    bool enableShadow;            ///< Enable drop shadow
    bool stayOnTop;               ///< Keep popup above other windows
    
    PopupConfig()
        : width(320)
        , height(140)
        , cornerRadius(8)
        , showDelayMs(50)
        , autoHideMs(0)
        , enableBlur(true)
        , enableShadow(true)
        , stayOnTop(true) {}
};

/**
 * @class CorrectionPopup
 * @brief Fluent Design "Did You Mean?" popup window
 * 
 * Displays a modern, lightweight popup showing the original word
 * and suggested correction. Supports keyboard navigation and
 * automatic positioning near the cursor.
 */
class CorrectionPopup {
public:
    /**
     * @brief Callback type for button clicks
     * @param button The button that was clicked
     */
    using ButtonCallback = std::function<void(PopupButton button)>;
    
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static CorrectionPopup& Instance();
    
    /**
     * @brief Initialize the popup system
     * @param hInstance Application instance handle
     * @return true if initialization was successful
     */
    bool Initialize(HINSTANCE hInstance);
    
    /**
     * @brief Shutdown and cleanup resources
     */
    void Shutdown();
    
    /**
     * @brief Show the popup with a correction suggestion
     * @param originalWord The incorrectly typed word
     * @param suggestedWord The suggested correction
     * @param x X position for popup (screen coordinates)
     * @param y Y position for popup (screen coordinates)
     * @param callback Function to call when user selects a button
     * @return true if popup was shown successfully
     */
    bool Show(const std::wstring& originalWord,
              const std::wstring& suggestedWord,
              int x, int y,
              ButtonCallback callback = nullptr);
    
    /**
     * @brief Hide the popup
     * @return true if popup was hidden
     */
    bool Hide();
    
    /**
     * @brief Check if popup is currently visible
     * @return true if popup is visible
     */
    bool IsVisible() const;
    
    /**
     * @brief Set the configuration options
     * @param config New configuration
     */
    void SetConfig(const PopupConfig& config);
    
    /**
     * @brief Get the current configuration
     * @return Current configuration
     */
    PopupConfig GetConfig() const;
    
    /**
     * @brief Simulate a button click programmatically
     * @param button Button to simulate clicking
     */
    void SimulateButtonClick(PopupButton button);
    
    /**
     * @brief Process keyboard input for popup navigation
     * @param vkCode Virtual key code of pressed key
     * @return true if key was handled by popup
     */
    bool HandleKeyPress(int vkCode);
    
    /**
     * @brief Get the HWND of the popup window
     * @return Window handle or nullptr
     */
    HWND GetWindowHandle() const;
    
    /**
     * @brief Update popup position
     * @param x New X position
     * @param y New Y position
     */
    void Reposition(int x, int y);
    
    // Delete copy/move operations
    CorrectionPopup(const CorrectionPopup&) = delete;
    CorrectionPopup& operator=(const CorrectionPopup&) = delete;
    CorrectionPopup(CorrectionPopup&&) = delete;
    CorrectionPopup& operator=(CorrectionPopup&&) = delete;
    
private:
    CorrectionPopup();
    ~CorrectionPopup();
    
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void Render();
    void CreateButtons();
    void OnYesClicked();
    void OnNoClicked();
    void ApplyFluentStyle();
    
    HINSTANCE m_hInstance;           ///< Application instance
    HWND m_hwnd;                     ///< Popup window handle
    HWND m_hwndOriginal;             ///< Original word label
    HWND m_hwndArrow;                ///< Arrow symbol
    HWND m_hwndSuggested;            ///< Suggested word label
    HWND m_hwndYesBtn;               ///< Yes button
    HWND m_hwndNoBtn;                ///< No button
    HWND m_hwndFocusedBtn;           ///< Currently focused button
    
    PopupConfig m_config;            ///< Popup configuration
    ButtonCallback m_callback;       ///< Button click callback
    
    std::wstring m_originalWord;     ///< Original word text
    std::wstring m_suggestedWord;    ///< Suggested word text
    
    mutable std::mutex m_mutex;      ///< Thread synchronization
    std::atomic<bool> m_visible;     ///< Visibility state
    std::atomic<int> m_focusedIndex; ///< Currently focused element (0=Yes, 1=No)
};

} // namespace mubaddil

#endif // POPUP_H
