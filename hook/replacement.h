#pragma once

/**
 * @file replacement.h
 * @brief Text replacement engine using keyboard simulation
 * 
 * Provides safe text replacement by simulating backspace and typing,
 * with cursor position preservation.
 */

#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include <windows.h>
#include <string>
#include <mutex>

namespace mubaddil {

/**
 * @class ReplacementEngine
 * @brief Handles text replacement via keyboard simulation
 * 
 * Safely replaces text by simulating backspace keystrokes to delete
 * the original word, then typing the replacement. Preserves clipboard
 * and cursor position.
 */
class ReplacementEngine {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static ReplacementEngine& Instance();
    
    /**
     * @brief Replace a word with another in the active window
     * @param originalWord The word to replace
     * @param replacementWord The replacement word
     * @param hwnd Target window handle (optional)
     * @return true if replacement was successful
     */
    bool ReplaceWord(const std::wstring& originalWord,
                     const std::wstring& replacementWord,
                     HWND hwnd = nullptr);
    
    /**
     * @brief Type text using keyboard simulation
     * @param text Text to type
     * @return true if successful
     */
    bool TypeText(const std::wstring& text);
    
    /**
     * @brief Send backspace keystrokes to delete characters
     * @param count Number of backspaces to send
     * @return true if successful
     */
    bool SendBackspaces(size_t count);
    
    /**
     * @brief Get the foreground window handle
     * @return Window handle or nullptr
     */
    HWND GetForegroundWindow() const;
    
    // Delete copy/move operations
    ReplacementEngine(const ReplacementEngine&) = delete;
    ReplacementEngine& operator=(const ReplacementEngine&) = delete;
    ReplacementEngine(ReplacementEngine&&) = delete;
    ReplacementEngine& operator=(ReplacementEngine&&) = delete;
    
private:
    ReplacementEngine() = default;
    ~ReplacementEngine() = default;
    
    /**
     * @brief Send a single keystroke
     * @param vkCode Virtual key code
     * @param scanCode Scan code
     * @param keyUp true for key-up event
     */
    void SendKey(int vkCode, int scanCode, bool keyUp = false);
    
    mutable std::mutex m_mutex;        ///< Thread synchronization
};

} // namespace mubaddil

#endif // REPLACEMENT_H
