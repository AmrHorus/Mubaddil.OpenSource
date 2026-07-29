/**
 * @file clipboard.cpp
 * @brief Implementation of clipboard management
 */

#include "clipboard.h"

namespace mubaddil {

ClipboardManager& ClipboardManager::Instance() {
    static ClipboardManager instance;
    return instance;
}

std::optional<std::wstring> ClipboardManager::GetText() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!OpenClipboard(nullptr)) {
        return std::nullopt;
    }
    
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (!hData) {
        CloseClipboard();
        return std::nullopt;
    }
    
    wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
    if (!pText) {
        CloseClipboard();
        return std::nullopt;
    }
    
    std::wstring text(pText);
    GlobalUnlock(hData);
    CloseClipboard();
    
    return text;
}

bool ClipboardManager::SetText(const std::wstring& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    EmptyClipboard();
    
    // Allocate memory for the text
    size_t size = (text.length() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, size);
    
    if (!hMem) {
        CloseClipboard();
        return false;
    }
    
    wchar_t* pMem = static_cast<wchar_t*>(GlobalLock(hMem));
    if (!pMem) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    
    wcscpy_s(pMem, size / sizeof(wchar_t), text.c_str());
    GlobalUnlock(hMem);
    
    if (!SetClipboardData(CF_UNICODETEXT, hMem)) {
        GlobalFree(hMem);
        CloseClipboard();
        return false;
    }
    
    CloseClipboard();
    return true;
}

bool ClipboardManager::Backup() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto text = GetText();
    if (text.has_value()) {
        m_backup = text.value();
        m_hasBackup = true;
        return true;
    }
    
    m_backup = std::nullopt;
    m_hasBackup = false;
    return false;
}

bool ClipboardManager::Restore() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_hasBackup || !m_backup.has_value()) {
        return false;
    }
    
    bool result = SetText(m_backup.value());
    m_backup = std::nullopt;
    m_hasBackup = false;
    
    return result;
}

bool ClipboardManager::Clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    bool result = EmptyClipboard() != 0;
    CloseClipboard();
    
    return result;
}

bool ClipboardManager::HasText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!OpenClipboard(nullptr)) {
        return false;
    }
    
    bool hasText = IsClipboardFormatAvailable(CF_UNICODETEXT) != 0;
    CloseClipboard();
    
    return hasText;
}

std::wstring ClipboardManager::GetBackupText() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_hasBackup && m_backup.has_value()) {
        return m_backup.value();
    }
    
    return L"";
}

bool ClipboardManager::SetBackupText(const std::wstring& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_backup = text;
    m_hasBackup = !text.empty();
    
    return true;
}

} // namespace mubaddil
