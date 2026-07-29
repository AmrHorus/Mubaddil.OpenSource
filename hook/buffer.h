#pragma once

/**
 * @file buffer.h
 * @brief Keyboard input buffer for word accumulation
 * 
 * Manages a thread-safe circular buffer for accumulating keyboard input
 * into words, with automatic word boundary detection.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <functional>

namespace mubaddil {

/**
 * @struct KeyEvent
 * @brief Represents a single keyboard event
 */
struct KeyEvent {
    int vkCode;         ///< Virtual key code
    int scanCode;       ///< Scan code
    DWORD flags;        ///< Key state flags
    DWORD time;         ///< Timestamp
    wchar_t character;  ///< Unicode character if applicable
    
    KeyEvent() : vkCode(0), scanCode(0), flags(0), time(0), character(0) {}
    KeyEvent(int vk, int sc, DWORD f, DWORD t, wchar_t c = 0)
        : vkCode(vk), scanCode(sc), flags(f), time(t), character(c) {}
};

/**
 * @class KeyboardBuffer
 * @brief Thread-safe buffer for accumulating keyboard input into words
 * 
 * Accumulates keystrokes into words, detecting word boundaries
 * based on spaces, punctuation, and modifier keys.
 */
class KeyboardBuffer {
public:
    /**
     * @brief Construct a new Keyboard Buffer
     * @param maxWordLength Maximum word length before auto-flush
     * @param maxBufferSize Maximum number of events to buffer
     */
    explicit KeyboardBuffer(size_t maxWordLength = 50, size_t maxBufferSize = 200);
    
    /**
     * @brief Add a key event to the buffer
     * @param event Key event to add
     * @return true if a word boundary was detected (word ready for processing)
     */
    bool AddKeyEvent(const KeyEvent& event);
    
    /**
     * @brief Get the current accumulated word
     * @return Current word as wide string
     */
    std::wstring GetCurrentWord() const;
    
    /**
     * @brief Clear the current buffer
     */
    void Clear();
    
    /**
     * @brief Check if buffer is empty
     * @return true if no characters buffered
     */
    bool IsEmpty() const noexcept;
    
    /**
     * @brief Get number of characters in current word
     * @return Character count
     */
    size_t GetLength() const noexcept;
    
    /**
     * @brief Set callback for when a complete word is detected
     * @param callback Function to call with completed word
     */
    void SetWordCompleteCallback(std::function<void(const std::wstring&)> callback);
    
    /**
     * @brief Check if a key represents a word boundary
     * @param vkCode Virtual key code
     * @return true if key ends a word
     */
    static bool IsWordBoundary(int vkCode) noexcept;
    
    /**
     * @brief Check if a key is a modifier key
     * @param vkCode Virtual key code
     * @return true if modifier key
     */
    static bool IsModifierKey(int vkCode) noexcept;
    
private:
    mutable std::mutex m_mutex;                    ///< Thread synchronization
    std::vector<KeyEvent> m_events;                ///< Buffered key events
    std::wstring m_currentWord;                    ///< Currently accumulated word
    size_t m_maxWordLength;                        ///< Maximum word length
    size_t m_maxBufferSize;                        ///< Maximum buffer size
    std::function<void(const std::wstring&)> m_wordCompleteCallback;  ///< Word complete callback
    std::atomic<bool> m_hasWord{false};            ///< Flag indicating word ready
};

} // namespace mubaddil

#endif // BUFFER_H
