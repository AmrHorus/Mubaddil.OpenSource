/**
 * @file buffer.cpp
 * @brief Implementation of keyboard input buffer
 */

#include "buffer.h"
#include <algorithm>

namespace mubaddil {

// Word boundary keys: space, enter, tab, punctuation that ends words
constexpr int WORD_BOUNDARY_KEYS[] = {
    0x20,  // VK_SPACE
    0x0D,  // VK_RETURN
    0x09,  // VK_TAB
    0xBC,  // VK_OEM_COMMA
    0xBE,  // VK_OEM_PERIOD
    0xBF,  // VK_OEM_2
    0xBA,  // VK_OEM_1 (semicolon)
    0xDE,  // VK_OEM_7 (quote)
    0xDB,  // VK_OEM_4 ([)
    0xDD,  // VK_OEM_6 (])
    0xDC,  // VK_OEM_5 (\)
    0xC0,  // VK_OEM_3 (`)
    0xBD,  // VK_OEM_MINUS
    0xBB,  // VK_OEM_PLUS
};

// Modifier keys
constexpr int MODIFIER_KEYS[] = {
    0x10,  // VK_SHIFT
    0x11,  // VK_CONTROL
    0x12,  // VK_MENU (Alt)
    0x14,  // VK_CAPITAL
    0x5B,  // VK_LWIN
    0x5C,  // VK_RWIN
    0xA0,  // VK_LSHIFT
    0xA1,  // VK_RSHIFT
    0xA2,  // VK_LCONTROL
    0xA3,  // VK_RCONTROL
    0xA4,  // VK_LMENU
    0xA5,  // VK_RMENU
};

KeyboardBuffer::KeyboardBuffer(size_t maxWordLength, size_t maxBufferSize)
    : m_maxWordLength(maxWordLength)
    , m_maxBufferSize(maxBufferSize) {
}

bool KeyboardBuffer::AddKeyEvent(const KeyEvent& event) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Ignore key-up events (flags & 0x80 indicates key-up for LL hooks)
    if (event.flags & 0x80) {
        return false;
    }
    
    // Check for word boundary
    if (IsWordBoundary(event.vkCode)) {
        bool hasWord = !m_currentWord.empty();
        if (hasWord && m_wordCompleteCallback) {
            // Invoke callback outside lock to prevent deadlock
            std::wstring wordToProcess = m_currentWord;
            m_currentWord.clear();
            m_events.clear();
            m_hasWord = false;
            
            // Release lock before calling callback
            lock.~lock_guard();
            m_wordCompleteCallback(wordToProcess);
            std::construct_at(&lock, m_mutex);
        } else {
            m_currentWord.clear();
            m_events.clear();
            m_hasWord = false;
        }
        return hasWord;
    }
    
    // Ignore modifier keys
    if (IsModifierKey(event.vkCode)) {
        return false;
    }
    
    // Handle backspace
    if (event.vkCode == VK_BACK) {
        if (!m_currentWord.empty()) {
            m_currentWord.pop_back();
            if (!m_events.empty()) {
                m_events.pop_back();
            }
        }
        return false;
    }
    
    // Handle escape - clear buffer
    if (event.vkCode == VK_ESCAPE) {
        m_currentWord.clear();
        m_events.clear();
        m_hasWord = false;
        return false;
    }
    
    // Convert virtual key to character (simplified - full implementation would use ToUnicodeEx)
    wchar_t ch = 0;
    
    // Basic ASCII conversion for common keys
    if (event.vkCode >= 0x41 && event.vkCode <= 0x5A) {
        // A-Z keys
        ch = static_cast<wchar_t>(event.vkCode);
        // Check shift state for case (simplified - would need GetAsyncKeyState in real impl)
        if (!(event.flags & 0x10)) {  // Not shifted
            ch += 32;  // Convert to lowercase
        }
    } else if (event.vkCode >= 0x30 && event.vkCode <= 0x39) {
        // 0-9 keys
        ch = static_cast<wchar_t>(event.vkCode);
    }
    
    if (ch != 0) {
        // Check buffer limits
        if (m_events.size() >= m_maxBufferSize || m_currentWord.length() >= m_maxWordLength) {
            // Auto-flush on overflow
            if (!m_currentWord.empty() && m_wordCompleteCallback) {
                std::wstring wordToProcess = m_currentWord;
                m_currentWord.clear();
                m_events.clear();
                m_hasWord = false;
                
                lock.~lock_guard();
                m_wordCompleteCallback(wordToProcess);
                std::construct_at(&lock, m_mutex);
            }
        }
        
        m_events.push_back(event);
        m_currentWord += ch;
        m_hasWord = true;
    }
    
    return false;
}

std::wstring KeyboardBuffer::GetCurrentWord() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentWord;
}

void KeyboardBuffer::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentWord.clear();
    m_events.clear();
    m_hasWord = false;
}

bool KeyboardBuffer::IsEmpty() const noexcept {
    return m_currentWord.empty();
}

size_t KeyboardBuffer::GetLength() const noexcept {
    return m_currentWord.length();
}

void KeyboardBuffer::SetWordCompleteCallback(std::function<void(const std::wstring&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_wordCompleteCallback = std::move(callback);
}

bool KeyboardBuffer::IsWordBoundary(int vkCode) noexcept {
    for (int key : WORD_BOUNDARY_KEYS) {
        if (vkCode == key) {
            return true;
        }
    }
    return false;
}

bool KeyboardBuffer::IsModifierKey(int vkCode) noexcept {
    for (int key : MODIFIER_KEYS) {
        if (vkCode == key) {
            return true;
        }
    }
    return false;
}

} // namespace mubaddil
