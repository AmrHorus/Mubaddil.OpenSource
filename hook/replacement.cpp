/**
 * @file replacement.cpp
 * @brief Implementation of text replacement engine
 */

#include "replacement.h"
#include "clipboard.h"
#include <thread>
#include <chrono>

namespace mubaddil {

ReplacementEngine& ReplacementEngine::Instance() {
    static ReplacementEngine instance;
    return instance;
}

void ReplacementEngine::SendKey(int vkCode, int scanCode, bool keyUp) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = static_cast<WORD>(vkCode);
    input.ki.wScan = static_cast<WORD>(scanCode);
    input.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : 0;
    
    SendInput(1, &input, sizeof(INPUT));
}

bool ReplacementEngine::SendBackspaces(size_t count) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (size_t i = 0; i < count; ++i) {
        // Key down
        SendKey(VK_BACK, 0x0E, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        
        // Key up
        SendKey(VK_BACK, 0x0E, true);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    
    return true;
}

bool ReplacementEngine::TypeText(const std::wstring& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (text.empty()) {
        return true;
    }
    
    // Use clipboard method for complex Unicode text
    auto& clipboard = ClipboardManager::Instance();
    
    // Backup existing clipboard content
    clipboard.Backup();
    
    // Set new text to clipboard
    if (!clipboard.SetText(text)) {
        clipboard.Restore();
        return false;
    }
    
    // Small delay to ensure clipboard is ready
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Send Ctrl+V to paste
    INPUT inputs[4] = {};
    
    // Ctrl down
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;
    inputs[0].ki.dwFlags = 0;
    
    // V down
    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';
    inputs[1].ki.dwFlags = 0;
    
    // V up
    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    
    // Ctrl up
    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    
    UINT sent = SendInput(4, inputs, sizeof(INPUT));
    
    // Small delay for paste to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Restore original clipboard content
    clipboard.Restore();
    
    return sent == 4;
}

bool ReplacementEngine::ReplaceWord(const std::wstring& originalWord,
                                     const std::wstring& replacementWord,
                                     HWND hwnd) {
    if (originalWord.empty() || replacementWord.empty()) {
        return false;
    }
    
    // Optionally activate target window
    if (hwnd != nullptr) {
        SetForegroundWindow(hwnd);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Delete the original word character by character
    // Note: For multi-byte characters, we need to count properly
    size_t charCount = originalWord.length();
    
    if (!SendBackspaces(charCount)) {
        return false;
    }
    
    // Small delay after backspacing
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    // Type the replacement word
    return TypeText(replacementWord);
}

HWND ReplacementEngine::GetForegroundWindow() const {
    return ::GetForegroundWindow();
}

} // namespace mubaddil
