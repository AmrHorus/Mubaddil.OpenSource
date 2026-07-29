#pragma once

/**
 * @file clipboard.h
 * @brief Clipboard management for text operations
 * 
 * Provides safe clipboard access with backup/restore functionality
 * to preserve user's clipboard content during replacements.
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include <windows.h>
#include <string>
#include <mutex>
#include <optional>

namespace mubaddil {

/**
 * @class ClipboardManager
 * @brief Thread-safe clipboard manager with backup support
 * 
 * Safely manages clipboard operations, preserving existing content
 * and handling clipboard ownership correctly.
 */
class ClipboardManager {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static ClipboardManager& Instance();
    
    /**
     * @brief Get current text from clipboard
     * @return Optional containing clipboard text if available
     */
    std::optional<std::wstring> GetText();
    
    /**
     * @brief Set text to clipboard
     * @param text Text to set
     * @return true if successful
     */
    bool SetText(const std::wstring& text);
    
    /**
     * @brief Backup current clipboard content
     * @return true if backup was successful
     */
    bool Backup();
    
    /**
     * @brief Restore previously backed up clipboard content
     * @return true if restore was successful
     */
    bool Restore();
    
    /**
     * @brief Clear clipboard
     * @return true if successful
     */
    bool Clear();
    
    /**
     * @brief Check if clipboard has text
     * @return true if clipboard contains text
     */
    bool HasText() const;
    
    /**
     * @brief Get the backed up clipboard text
     * @return Backed up text or empty string if no backup
     */
    std::wstring GetBackupText() const;
    
    /**
     * @brief Set the backup text directly
     * @param text Text to set as backup
     * @return true if successful
     */
    bool SetBackupText(const std::wstring& text);
    
    // Delete copy/move operations
    ClipboardManager(const ClipboardManager&) = delete;
    ClipboardManager& operator=(const ClipboardManager&) = delete;
    ClipboardManager(ClipboardManager&&) = delete;
    ClipboardManager& operator=(ClipboardManager&&) = delete;
    
private:
    ClipboardManager() = default;
    ~ClipboardManager() = default;
    
    mutable std::mutex m_mutex;              ///< Thread synchronization
    std::optional<std::wstring> m_backup;    ///< Backed up clipboard content
    bool m_hasBackup{false};                 ///< Whether backup exists
};

} // namespace mubaddil

#endif // CLIPBOARD_H
