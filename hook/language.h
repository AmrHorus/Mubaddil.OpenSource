#pragma once

/**
 * @file language.h
 * @brief Language utilities and keyboard layout management
 * 
 * Provides language detection helpers, keyboard layout switching,
 * and locale-specific operations for Arabic and English.
 */

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <windows.h>
#include <string>
#include <mutex>
#include <atomic>

namespace mubaddil {

/**
 * @enum KeyboardLayoutId
 * @brief Common keyboard layout identifiers
 */
enum class KeyboardLayoutId : unsigned long {
    EnglishUS = 0x0409,
    Arabic = 0x0401,
    Unknown = 0x0000
};

/**
 * @struct LayoutInfo
 * @brief Information about a keyboard layout
 */
struct LayoutInfo {
    void* handle;                 ///< HKL handle
    std::wstring name;            ///< Layout name
    KeyboardLayoutId languageId;  ///< Language identifier
    bool isActive;                ///< Whether this is the active layout
    
    LayoutInfo()
        : handle(nullptr)
        , languageId(KeyboardLayoutId::Unknown)
        , isActive(false) {}
};

/**
 * @class LanguageManager
 * @brief Manages keyboard layouts and language switching
 * 
 * Handles loading, activating, and monitoring keyboard layouts
 * for seamless language switching during text replacement.
 */
class LanguageManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static LanguageManager& Instance();
    
    /**
     * @brief Initialize the language manager
     * @return true if initialization was successful
     */
    bool Initialize();
    
    /**
     * @brief Shutdown and cleanup resources
     */
    void Shutdown();
    
    /**
     * @brief Get the current active keyboard layout
     * @param hwnd Optional window handle (uses foreground window if null)
     * @return LayoutInfo for current layout
     */
    LayoutInfo GetCurrentLayout(HWND hwnd = nullptr);
    
    /**
     * @brief Set the active keyboard layout
     * @param layoutId Target layout identifier
     * @param hwnd Optional window handle
     * @return true if successful
     */
    bool SetLayout(KeyboardLayoutId layoutId, HWND hwnd = nullptr);
    
    /**
     * @brief Switch to English (US) keyboard layout
     * @param hwnd Optional window handle
     * @return true if successful
     */
    bool SwitchToEnglish(HWND hwnd = nullptr);
    
    /**
     * @brief Switch to Arabic keyboard layout
     * @param hwnd Optional window handle
     * @return true if successful
     */
    bool SwitchToArabic(HWND hwnd = nullptr);
    
    /**
     * @brief Check if English layout is currently active
     * @param hwnd Optional window handle
     * @return true if English is active
     */
    bool IsEnglishActive(HWND hwnd = nullptr);
    
    /**
     * @brief Check if Arabic layout is currently active
     * @param hwnd Optional window handle
     * @return true if Arabic is active
     */
    bool IsArabicActive(HWND hwnd = nullptr);
    
    /**
     * @brief Get the language ID of the current layout
     * @param hwnd Optional window handle
     * @return Language ID (0x0409 for English, 0x0401 for Arabic)
     */
    unsigned long GetCurrentLanguageId(HWND hwnd = nullptr);
    
    /**
     * @brief Load a keyboard layout by ID
     * @param layoutId Layout identifier to load
     * @return true if layout was loaded successfully
     */
    bool LoadLayout(KeyboardLayoutId layoutId);
    
    /**
     * @brief Unload a previously loaded keyboard layout
     * @param layoutId Layout identifier to unload
     * @return true if successful
     */
    bool UnloadLayout(KeyboardLayoutId layoutId);
    
    /**
     * @brief Check if a layout is available on the system
     * @param layoutId Layout identifier to check
     * @return true if layout is available
     */
    bool IsLayoutAvailable(KeyboardLayoutId layoutId);
    
    /**
     * @brief Get count of layout switches performed
     * @return Number of switches
     */
    size_t GetSwitchCount() const;
    
    /**
     * @brief Reset switch counter
     */
    void ResetSwitchCount();
    
    // Delete copy/move operations
    LanguageManager(const LanguageManager&) = delete;
    LanguageManager& operator=(const LanguageManager&) = delete;
    LanguageManager(LanguageManager&&) = delete;
    LanguageManager& operator=(LanguageManager&&) = delete;
    
private:
    LanguageManager();
    ~LanguageManager();
    
    void* GetLayoutHandle(KeyboardLayoutId layoutId);
    HWND GetTargetWindow(HWND hwnd);
    
    mutable std::mutex m_mutex;              ///< Thread synchronization
    void* m_hklEnglish;                      ///< English layout handle
    void* m_hklArabic;                       ///< Arabic layout handle
    std::atomic<size_t> m_switchCount;       ///< Number of layout switches
    bool m_initialized;                      ///< Initialization state
};

} // namespace mubaddil

#endif // LANGUAGE_H
