#pragma once

/**
 * @file mapper.h
 * @brief Keyboard layout mapping between English and Arabic
 * 
 * Provides bidirectional character mapping based on keyboard layouts.
 * Supports Saudi Arabic keyboard layout mapping.
 */

#ifndef MAPPER_H
#define MAPPER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>

namespace mubaddil {

/**
 * @struct KeyMapping
 * @brief Represents a single key's mapping in different layouts
 */
struct KeyMapping {
    int vkCode;                 ///< Virtual key code
    std::wstring englishNormal; ///< Character when pressed normally (English)
    std::wstring englishShift;  ///< Character with Shift (English)
    std::wstring arabicNormal;  ///< Character when pressed normally (Arabic)
    std::wstring arabicShift;   ///< Character with Shift (Arabic)
};

/**
 * @class KeyboardMapper
 * @brief Bidirectional keyboard layout mapper for English/Arabic
 * 
 * Provides character-level mapping between English QWERTY and Arabic layouts.
 * Thread-safe singleton implementation with lazy initialization.
 */
class KeyboardMapper {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static KeyboardMapper& Instance();
    
    /**
     * @brief Convert English text to Arabic layout equivalent
     * @param text Input text in English layout
     * @return Converted text as if typed in Arabic layout
     */
    std::wstring ConvertEnToAr(const std::wstring& text);
    
    /**
     * @brief Convert Arabic text to English layout equivalent
     * @param text Input text in Arabic layout
     * @return Converted text as if typed in English layout
     */
    std::wstring ConvertArToEn(const std::wstring& text);
    
    /**
     * @brief Check if a character is an Arabic character
     * @param ch Character to check
     * @return true if Arabic character
     */
    bool IsArabicChar(wchar_t ch) const;
    
    /**
     * @brief Check if a character is an English letter
     * @param ch Character to check
     * @return true if English letter
     */
    bool IsEnglishChar(wchar_t ch) const;
    
    /**
     * @brief Get the mapping for a specific virtual key code
     * @param vkCode Virtual key code
     * @return Optional containing the mapping if found
     */
    std::optional<KeyMapping> GetKeyMapping(int vkCode) const;
    
    /**
     * @brief Clear cached mappings (for reinitialization)
     */
    void ClearCache();
    
    // Delete copy/move operations
    KeyboardMapper(const KeyboardMapper&) = delete;
    KeyboardMapper& operator=(const KeyboardMapper&) = delete;
    KeyboardMapper(KeyboardMapper&&) = delete;
    KeyboardMapper& operator=(KeyboardMapper&&) = delete;
    
private:
    KeyboardMapper();
    ~KeyboardMapper() = default;
    
    /**
     * @brief Build the character mapping tables
     */
    void BuildMappings();
    
    mutable std::mutex m_mutex;                              ///< Thread synchronization
    std::unordered_map<int, KeyMapping> m_keyMappings;       ///< VK code to mapping
    std::unordered_map<wchar_t, wchar_t> m_enToArMap;        ///< English to Arabic char map
    std::unordered_map<wchar_t, wchar_t> m_arToEnMap;        ///< Arabic to English char map
    std::unordered_map<wchar_t, bool> m_arabicChars;         ///< Arabic character set
    std::unordered_map<wchar_t, bool> m_englishChars;        ///< English character set
    bool m_initialized{false};                               ///< Initialization flag
};

} // namespace mubaddil

#endif // MAPPER_H
