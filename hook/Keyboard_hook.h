#pragma once

/**
 * @file Keyboard_hook.h
 * @brief Low-level Windows keyboard hook implementation
 * 
 * Provides system-wide keyboard monitoring using WH_KEYBOARD_LL hook.
 * Thread-safe design with atomic state management.
 */

#ifndef KEYBOARD_HOOK_H
#define KEYBOARD_HOOK_H

#include <windows.h>
#include <functional>
#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace mubaddil {

/**
 * @brief Callback type for key events
 * @param vkCode Virtual key code
 * @param scanCode Scan code
 * @param flags Key state flags
 * @param time Timestamp
 */
using KeyEventCallback = std::function<void(int vkCode, int scanCode, int flags, DWORD time)>;

/**
 * @class KeyboardHook
 * @brief Manages low-level keyboard hook installation and event processing
 * 
 * Singleton pattern ensures only one hook instance exists per process.
 * Uses RAII for automatic cleanup on destruction.
 */
class KeyboardHook {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static KeyboardHook& Instance();

    /**
     * @brief Install the keyboard hook
     * @param callback Function to call on key events
     * @return true if successful, false otherwise
     */
    bool Install(KeyEventCallback callback);

    /**
     * @brief Remove the keyboard hook
     */
    void Uninstall();

    /**
     * @brief Check if hook is currently installed
     * @return true if installed, false otherwise
     */
    bool IsInstalled() const noexcept;

    /**
     * @brief Process a key event (called from hook thread)
     * @param vkCode Virtual key code
     * @param scanCode Scan code
     * @param flags Key flags
     * @param time Event timestamp
     */
    void ProcessKeyEvent(int vkCode, int scanCode, int flags, DWORD time);

    /**
     * @brief Set the foreground window handle for context
     * @param hwnd Window handle
     */
    void SetForegroundWindow(HWND hwnd) noexcept;

    /**
     * @brief Get current foreground window handle
     * @return Window handle or nullptr
     */
    HWND GetForegroundWindow() const noexcept;

    // Delete copy/move operations for singleton safety
    KeyboardHook(const KeyboardHook&) = delete;
    KeyboardHook& operator=(const KeyboardHook&) = delete;
    KeyboardHook(KeyboardHook&&) = delete;
    KeyboardHook& operator=(KeyboardHook&&) = delete;

private:
    KeyboardHook() = default;
    ~KeyboardHook();

    /**
     * @brief Static hook procedure (Windows API callback)
     */
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

    HHOOK m_hook{nullptr};                          ///< Hook handle
    KeyEventCallback m_callback;                     ///< User callback
    mutable std::atomic<bool> m_installed{false};   ///< Installation state
    mutable std::mutex m_mutex;                      ///< Thread synchronization
    HWND m_foregroundHwnd{nullptr};                  ///< Current foreground window
};

} // namespace mubaddil

#endif // KEYBOARD_HOOK_H