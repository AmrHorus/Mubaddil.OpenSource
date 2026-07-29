/**
 * @file replacement.cpp
 * @brief Implementation of text replacement engine
 */

#include "replacement.h"
#include "clipboard.h"
#include <thread>
#include <chrono>

namespace mubaddil {

ReplacementEngine::ReplacementEngine()
    : m_replacementCount(0)
    , m_successCount(0)
    , m_failureCount(0)
    , m_clipboardSaved(false)
    , m_hklEnglish(nullptr)
    , m_hklArabic(nullptr)
{
    // Pre-load keyboard layout handles
    m_hklEnglish = LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);
    m_hklArabic = LoadKeyboardLayoutA("00000401", KLF_ACTIVATE);
}

ReplacementEngine::~ReplacementEngine() {
    if (m_hklEnglish) {
        UnloadKeyboardLayout(static_cast<HKL>(m_hklEnglish));
    }
    if (m_hklArabic) {
        UnloadKeyboardLayout(static_cast<HKL>(m_hklArabic));
    }
}

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

ReplacementResult ReplacementEngine::ReplaceWord(const std::wstring& originalWord,
                                                  const std::wstring& newWord,
                                                  HWND hwnd) {
    ReplacementResult result;
    result.originalWord = originalWord;
    result.newWord = newWord;
    
    if (originalWord.empty() || newWord.empty()) {
        result.success = false;
        result.errorMessage = L"Empty word provided";
        m_failureCount++;
        return result;
    }
    
    // Optionally activate target window
    if (hwnd != nullptr) {
        SetForegroundWindow(hwnd);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } else {
        hwnd = GetForegroundWindow();
    }
    
    if (hwnd == nullptr) {
        result.success = false;
        result.errorMessage = L"No active window found";
        m_failureCount++;
        return result;
    }
    
    // Save clipboard before operation
    SaveClipboard();
    
    // Delete the original word character by character
    size_t charCount = originalWord.length();
    result.charsDeleted = charCount;
    
    if (!SendBackspaces(charCount)) {
        result.success = false;
        result.errorMessage = L"Failed to delete original word";
        m_failureCount++;
        RestoreClipboard();
        return result;
    }
    
    // Small delay after backspacing
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    // Type the replacement word
    result.charsInserted = newWord.length();
    result.caretOffset = static_cast<int>(newWord.length()) - static_cast<int>(originalWord.length());
    
    if (TypeText(newWord)) {
        result.success = true;
        m_successCount++;
    } else {
        result.success = false;
        result.errorMessage = L"Failed to type replacement word";
        m_failureCount++;
    }
    
    // Restore clipboard
    RestoreClipboard();
    
    m_replacementCount++;
    return result;
}

ReplacementResult ReplacementEngine::ReplaceWordAndSwitchLayout(const std::wstring& originalWord,
                                                                 const std::wstring& newWord,
                                                                 unsigned long targetLang,
                                                                 HWND hwnd) {
    // First perform the replacement
    ReplacementResult result = ReplaceWord(originalWord, newWord, hwnd);
    
    if (!result.success) {
        return result;
    }
    
    // Switch keyboard layout based on target language
    bool layoutSwitched = false;
    if (targetLang == 0x0409) { // English
        layoutSwitched = SwitchToEnglish();
    } else if (targetLang == 0x0401) { // Arabic
        layoutSwitched = SwitchToArabic();
    }
    
    // Note: We don't fail the replacement if layout switch fails
    // The word was still replaced successfully
    
    return result;
}

void* ReplacementEngine::GetCurrentKeyboardLayout() {
    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        return GetKeyboardLayout(hwnd);
    }
    return nullptr;
}

bool ReplacementEngine::SetKeyboardLayout(void* hkl) {
    if (!hkl) {
        return false;
    }
    
    HWND hwnd = GetForegroundWindow();
    if (!hwnd) {
        return false;
    }
    
    HKL result = ActivateKeyboardLayout(static_cast<HKL>(hkl), KLF_SETFORPROCESS);
    return result != nullptr;
}

bool ReplacementEngine::SwitchToEnglish() {
    if (!m_hklEnglish) {
        m_hklEnglish = LoadKeyboardLayoutA("00000409", 0);
    }
    return SetKeyboardLayout(m_hklEnglish);
}

bool ReplacementEngine::SwitchToArabic() {
    if (!m_hklArabic) {
        m_hklArabic = LoadKeyboardLayoutA("00000401", 0);
    }
    return SetKeyboardLayout(m_hklArabic);
}

unsigned long ReplacementEngine::GetCurrentLayoutLanguage() {
    void* hkl = GetCurrentKeyboardLayout();
    if (hkl) {
        return LOWORD(reinterpret_cast<uintptr_t>(hkl));
    }
    return 0;
}

bool ReplacementEngine::IsArabicLayoutActive() {
    return GetCurrentLayoutLanguage() == 0x0401;
}

bool ReplacementEngine::IsEnglishLayoutActive() {
    return GetCurrentLayoutLanguage() == 0x0409;
}

bool ReplacementEngine::SaveClipboard() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto& clipboard = ClipboardManager::Instance();
    m_savedClipboardText = clipboard.GetBackupText();
    m_clipboardSaved = true;
    
    return true;
}

bool ReplacementEngine::RestoreClipboard() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_clipboardSaved) {
        return true; // Nothing to restore
    }
    
    auto& clipboard = ClipboardManager::Instance();
    bool result = clipboard.SetBackupText(m_savedClipboardText);
    m_clipboardSaved = false;
    
    return result;
}

size_t ReplacementEngine::GetReplacementCount() const {
    return m_replacementCount.load();
}

size_t ReplacementEngine::GetSuccessCount() const {
    return m_successCount.load();
}

size_t ReplacementEngine::GetFailureCount() const {
    return m_failureCount.load();
}

void ReplacementEngine::ResetStatistics() {
    m_replacementCount = 0;
    m_successCount = 0;
    m_failureCount = 0;
}

HWND ReplacementEngine::GetForegroundWindow() const {
    return ::GetForegroundWindow();
}

} // namespace mubaddil
