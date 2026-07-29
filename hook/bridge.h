#pragma once

/**
 * @file bridge.h
 * @brief Python-C++ communication bridge
 * 
 * Provides the interface for Python to interact with the C++ core,
 * exposing initialization, hook control, and replacement functions.
 */

#ifndef BRIDGE_H
#define BRIDGE_H

#include <windows.h>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>

namespace mubaddil {

/**
 * @struct Statistics
 * @brief Runtime statistics for the engine
 */
struct Statistics {
    uint64_t totalCorrections;      ///< Total corrections made
    uint64_t enToArCorrections;     ///< English to Arabic corrections
    uint64_t arToEnCorrections;     ///< Arabic to English corrections
    uint64_t suggestionsShown;      ///< Number of suggestions displayed
    uint64_t suggestionsAccepted;   ///< Number of accepted suggestions
    uint64_t suggestionsRejected;   ///< Number of rejected suggestions
    uint64_t keysProcessed;         ///< Total keys processed
    double avgDetectionTimeMs;      ///< Average detection time in ms
    double avgReplacementTimeMs;    ///< Average replacement time in ms
    
    Statistics()
        : totalCorrections(0), enToArCorrections(0), arToEnCorrections(0)
        , suggestionsShown(0), suggestionsAccepted(0), suggestionsRejected(0)
        , keysProcessed(0), avgDetectionTimeMs(0.0), avgReplacementTimeMs(0.0) {}
};

/**
 * @struct CorrectionEvent
 * @brief Event data for correction notifications
 */
struct CorrectionEvent {
    std::wstring original;          ///< Original typed word
    std::wstring suggested;         ///< Suggested correction
    bool isAutoReplace;             ///< Whether this is auto-replace
    int targetHwnd;                 ///< Target window handle
    double confidence;              ///< Confidence score
};

/**
 * @class Bridge
 * @brief Main bridge between C++ core and Python UI
 * 
 * Manages the lifecycle of all core components and provides
 * a clean API for Python interaction via ctypes or pybind11.
 */
class Bridge {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static Bridge& Instance();
    
    /**
     * @brief Initialize the bridge and all subsystems
     * @param logPath Path to log file (optional)
     * @return true if successful
     */
    bool Initialize(const std::wstring& logPath = L"");
    
    /**
     * @brief Shutdown all subsystems
     */
    void Shutdown();
    
    /**
     * @brief Start the keyboard hook
     * @return true if successful
     */
    bool StartHook();
    
    /**
     * @brief Stop the keyboard hook
     */
    void StopHook();
    
    /**
     * @brief Check if hook is running
     * @return true if hook is active
     */
    bool IsHookActive() const noexcept;
    
    /**
     * @brief Enable monitoring
     */
    void Enable();
    
    /**
     * @brief Disable monitoring
     */
    void Disable();
    
    /**
     * @brief Check if monitoring is enabled
     * @return true if enabled
     */
    bool IsEnabled() const noexcept;
    
    /**
     * @brief Replace a word in the active window
     * @param original Original word
     * @param replacement Replacement word
     * @param hwnd Target window handle
     * @return true if successful
     */
    bool ReplaceWord(const std::wstring& original,
                     const std::wstring& replacement,
                     HWND hwnd);
    
    /**
     * @brief Get current version string
     * @return Version as string
     */
    static const wchar_t* GetVersion() noexcept;
    
    /**
     * @brief Get current statistics
     * @return Statistics structure
     */
    Statistics GetStatistics() const;
    
    /**
     * @brief Set callback for correction events
     * @param callback Function to call on correction events
     */
    void SetCorrectionCallback(std::function<void(const CorrectionEvent&)> callback);
    
    /**
     * @brief Record an accepted suggestion
     * @param isEnToAr true if English to Arabic conversion
     */
    void RecordAcceptance(bool isEnToAr);
    
    /**
     * @brief Record a rejected suggestion
     */
    void RecordRejection();
    
    /**
     * @brief Internal key event handler (called from hook thread)
     * @param vkCode Virtual key code
     * @param scanCode Scan code
     * @param flags Key flags
     * @param time Timestamp
     */
    void OnKeyEvent(int vkCode, int scanCode, int flags, DWORD time);
    
    // Delete copy/move operations
    Bridge(const Bridge&) = delete;
    Bridge& operator=(const Bridge&) = delete;
    Bridge(Bridge&&) = delete;
    Bridge& operator=(Bridge&&) = delete;
    
private:
    Bridge() = default;
    ~Bridge();
    
    mutable std::mutex m_mutex;                      ///< Thread synchronization
    std::atomic<bool> m_enabled{false};              ///< Monitoring enabled flag
    std::atomic<bool> m_initialized{false};          ///< Initialization flag
    Statistics m_statistics;                         ///< Runtime statistics
    std::function<void(const CorrectionEvent&)> m_correctionCallback;  ///< Event callback
};

} // namespace mubaddil

// C-style exports for ctypes compatibility
extern "C" {
    __declspec(dllexport) bool Mubaddil_Initialize(const wchar_t* logPath);
    __declspec(dllexport) void Mubaddil_Shutdown();
    __declspec(dllexport) bool Mubaddil_StartHook();
    __declspec(dllexport) void Mubaddil_StopHook();
    __declspec(dllexport) bool Mubaddil_IsHookActive();
    __declspec(dllexport) void Mubaddil_Enable();
    __declspec(dllexport) void Mubaddil_Disable();
    __declspec(dllexport) bool Mubaddil_IsEnabled();
    __declspec(dllexport) bool Mubaddil_ReplaceWord(const wchar_t* original, 
                                                     const wchar_t* replacement,
                                                     HWND hwnd);
    __declspec(dllexport) const wchar_t* Mubaddil_GetVersion();
    __declspec(dllexport) uint64_t Mubaddil_GetTotalCorrections();
    __declspec(dllexport) uint64_t Mubaddil_GetEnToArCorrections();
    __declspec(dllexport) uint64_t Mubaddil_GetArToEnCorrections();
}

#endif // BRIDGE_H
