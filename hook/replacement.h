#pragma once

/**
 * @file replacement.h
 * @brief Text replacement engine for correcting keyboard layout mistakes
 * 
 * Handles safe replacement of words while preserving surrounding text,
 * punctuation, and cursor position. Also manages keyboard layout switching.
 */

#ifndef REPLACEMENT_H
#define REPLACEMENT_H

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace mubaddil {

/**
 * @struct ReplacementResult
 * @brief Result of a text replacement operation
 */
struct ReplacementResult {
    bool success;               ///< Whether replacement succeeded
    std::wstring originalWord;  ///< The word that was replaced
    std::wstring newWord;       ///< The replacement word
    size_t charsDeleted;        ///< Number of characters deleted
    size_t charsInserted;       ///< Number of characters inserted
    int caretOffset;            ///< Offset to adjust caret position
    std::wstring errorMessage;  ///< Error message if failed
    
    ReplacementResult()
        : success(false), charsDeleted(0), charsInserted(0), caretOffset(0) {}
};

/**
 * @class ReplacementEngine
 * @brief Handles safe text replacement and keyboard layout management
 * 
 * Provides atomic replacement operations that preserve surrounding text
 * and manage clipboard state. Also handles keyboard layout switching.
 */
class ReplacementEngine {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static ReplacementEngine& Instance();
    
    /**
     * @brief Replace a word at the current cursor position
     * @param originalWord The word to replace
     * @param newWord The replacement word
     * @param hwnd Target window handle (optional)
     * @return ReplacementResult with operation details
     */
    ReplacementResult ReplaceWord(const std::wstring& originalWord,
                                   const std::wstring& newWord,
                                   HWND hwnd = nullptr);
    
    /**
     * @brief Replace a word and switch keyboard layout
     * @param originalWord The word to replace
     * @param newWord The replacement word
     * @param targetLang Keyboard layout language ID (0x0409=English, 0x0401=Arabic)
     * @param hwnd Target window handle (optional)
     * @return ReplacementResult with operation details
     */
    ReplacementResult ReplaceWordAndSwitchLayout(const std::wstring& originalWord,
                                                  const std::wstring& newWord,
                                                  unsigned long targetLang,
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
    
    /**
     * @brief Get the current active keyboard layout
     * @return HKL handle to current keyboard layout
     */
    void* GetCurrentKeyboardLayout();
    
    /**
     * @brief Set the active keyboard layout
     * @param hkl Keyboard layout handle
     * @return true if successful
     */
    bool SetKeyboardLayout(void* hkl);
    
    /**
     * @brief Switch to English keyboard layout
     * @return true if successful
     */
    bool SwitchToEnglish();
    
    /**
     * @brief Switch to Arabic keyboard layout
     * @return true if successful
     */
    bool SwitchToArabic();
    
    /**
     * @brief Get the language of the current keyboard layout
     * @return 0x0409 for English, 0x0401 for Arabic, 0 for unknown
     */
    unsigned long GetCurrentLayoutLanguage();
    
    /**
     * @brief Check if current layout is Arabic
     * @return true if Arabic layout is active
     */
    bool IsArabicLayoutActive();
    
    /**
     * @brief Check if current layout is English
     * @return true if English layout is active
     */
    bool IsEnglishLayoutActive();
    
    /**
     * @brief Save current clipboard content
     * @return true if successful
     */
    bool SaveClipboard();
    
    /**
     * @brief Restore saved clipboard content
     * @return true if successful
     */
    bool RestoreClipboard();
    
    /**
     * @brief Get number of replacements performed
     * @return Total replacement count
     */
    size_t GetReplacementCount() const;
    
    /**
     * @brief Get number of successful replacements
     * @return Successful replacement count
     */
    size_t GetSuccessCount() const;
    
    /**
     * @brief Get number of failed replacements
     * @return Failed replacement count
     */
    size_t GetFailureCount() const;
    
    /**
     * @brief Reset statistics counters
     */
    void ResetStatistics();
    
    // Delete copy/move operations
    ReplacementEngine(const ReplacementEngine&) = delete;
    ReplacementEngine& operator=(const ReplacementEngine&) = delete;
    ReplacementEngine(ReplacementEngine&&) = delete;
    ReplacementEngine& operator=(ReplacementEngine&&) = delete;
    
private:
    ReplacementEngine();
    ~ReplacementEngine();
    
    bool FindAndReplaceWord(const std::wstring& originalWord,
                            const std::wstring& newWord,
                            ReplacementResult& result);
    
    void SendKey(int vkCode, int scanCode, bool keyUp = false);
    
    mutable std::mutex m_mutex;           ///< Thread synchronization
    std::atomic<size_t> m_replacementCount; ///< Total replacements
    std::atomic<size_t> m_successCount;     ///< Successful replacements
    std::atomic<size_t> m_failureCount;     ///< Failed replacements
    
    bool m_clipboardSaved;                ///< Clipboard save state
    std::wstring m_savedClipboardText;    ///< Saved clipboard content
    void* m_hklEnglish;                   ///< English layout handle
    void* m_hklArabic;                    ///< Arabic layout handle
};

} // namespace mubaddil

#endif // REPLACEMENT_H
