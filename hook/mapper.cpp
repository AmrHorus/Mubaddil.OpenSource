/**
 * @file mapper.cpp
 * @brief Implementation of keyboard layout mapping
 * 
 * Implements Saudi Arabic keyboard layout mapping based on Windows 101-key layout.
 */

#include "mapper.h"
#include <algorithm>

namespace mubaddil {

KeyboardMapper& KeyboardMapper::Instance() {
    static KeyboardMapper instance;
    return instance;
}

KeyboardMapper::KeyboardMapper() {
    BuildMappings();
}

void KeyboardMapper::BuildMappings() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return;
    }
    
    // Saudi Arabic keyboard layout mapping (Windows 101-key)
    // Format: VK code -> English normal, English shift, Arabic normal, Arabic shift
    
    auto addMapping = [this](int vk, const wchar_t* enNorm, const wchar_t* enShift,
                              const wchar_t* arNorm, const wchar_t* arShift) {
        KeyMapping km;
        km.vkCode = vk;
        km.englishNormal = enNorm;
        km.englishShift = enShift;
        km.arabicNormal = arNorm;
        km.arabicShift = arShift;
        
        m_keyMappings[vk] = km;
        
        // Build character conversion maps
        if (enNorm[0] && arNorm[0]) {
            m_enToArMap[enNorm[0]] = arNorm[0];
            m_arToEnMap[arNorm[0]] = enNorm[0];
            m_arabicChars[arNorm[0]] = true;
            m_englishChars[enNorm[0]] = true;
        }
        if (enShift[0] && arShift[0]) {
            m_enToArMap[enShift[0]] = arShift[0];
            m_arToEnMap[arShift[0]] = enShift[0];
            m_arabicChars[arShift[0]] = true;
            m_englishChars[enShift[0]] = true;
        }
    };
    
    // Numbers row
    addMapping(0x30, L"0", L")", L"٠", L")");
    addMapping(0x31, L"1", L"!", L"١", L"!");
    addMapping(0x32, L"2", L"@", L"٢", L"@");
    addMapping(0x33, L"3", L"#", L"٣", L"#");
    addMapping(0x34, L"4", L"$", L"٤", L"$");
    addMapping(0x35, L"5", L"%", L"٥", L"%");
    addMapping(0x36, L"6", L"^", L"٦", L"^");
    addMapping(0x37, L"7", L"&", L"٧", L"&");
    addMapping(0x38, L"8", L"*", L"٨", L"*");
    addMapping(0x39, L"9", L"(", L"٩", L"(");
    
    // QWERTY row
    addMapping(0x51, L"q", L"Q", L"ض", L"َ");   // Q
    addMapping(0x57, L"w", L"W", L"ص", L"ً");   // W
    addMapping(0x45, L"e", L"E", L"ث", L"ُ");   // E
    addMapping(0x52, L"r", L"R", L"ق", L"ٌ");   // R
    addMapping(0x54, L"t", L"T", L"ف", L"ل");   // T
    addMapping(0x59, L"y", L"Y", L"غ", L"إ");   // Y
    addMapping(0x55, L"u", L"U", L"ع", L"'");   // U
    addMapping(0x49, L"i", L"I", L"ه", L"÷");   // I
    addMapping(0x4F, L"o", L"O", L"خ", L"×");   // O
    addMapping(0x50, L"p", L"P", L"ح", L"؛");   // P
    addMapping(0xDB, L"[", L"{", L"ج", L"<");   // [
    addMapping(0xDD, L"]", L"}", L"د", L">");   // ]
    
    // ASDF row
    addMapping(0x41, L"a", L"A", L"ش", L"ِ");   // A
    addMapping(0x53, L"s", L"S", L"س", L"ٍ");   // S
    addMapping(0x44, L"d", L"D", L"ي", L"]");   // D
    addMapping(0x46, L"f", L"F", L"ب", L"[");   // F
    addMapping(0x47, L"g", L"G", L"ل", L"ل");   // G
    addMapping(0x48, L"h", L"H", L"ا", L"أ");   // H
    addMapping(0x4A, L"j", L"J", L"ت", L"-");   // J
    addMapping(0x4B, L"k", L"K", L"ن", L"،");   // K
    addMapping(0x4C, L"l", L"L", L"م", L"/");   // L
    addMapping(0xBA, L";", L":", L"ك", L":");   // ;
    addMapping(0xDE, L"'", L"\"", L"ط", L"\""); // '
    
    // ZXCV row
    addMapping(0x5A, L"z", L"Z", L"ئ", L"~");   // Z
    addMapping(0x58, L"x", L"X", L"ء", L"ْ");   // X
    addMapping(0x43, L"c", L"C", L"ؤ", L"}");   // C
    addMapping(0x56, L"v", L"V", L"ر", L"{");   // V
    addMapping(0x42, L"b", L"B", L"لا", L"آ");  // B
    addMapping(0x4E, L"n", L"N", L"ى", L"'");   // N
    addMapping(0x4D, L"m", L"M", L"ة", L"'");   // M
    addMapping(0xBC, L",", L"<", L"و", L",");   // ,
    addMapping(0xBE, L".", L">", L"ز", L".");   // .
    addMapping(0xBF, L"/", L"?", L"ظ", L"?");   // /
    
    // Other keys
    addMapping(0x20, L" ", L" ", L" ", L" ");           // Space
    addMapping(0xBD, L"-", L"_", L"-", L"_");           // -
    addMapping(0xBB, L"=", L"+", L"=", L"+");           // =
    addMapping(0xDC, L"\\", L"|", L"\\", L"|");         // \
    addMapping(0xC0, L"`", L"~", L"ذ", L"ّ");           // `
    
    m_initialized = true;
}

std::wstring KeyboardMapper::ConvertEnToAr(const std::wstring& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::wstring result;
    result.reserve(text.length());
    
    for (wchar_t ch : text) {
        auto it = m_enToArMap.find(ch);
        if (it != m_enToArMap.end()) {
            result += it->second;
        } else {
            result += ch;
        }
    }
    
    return result;
}

std::wstring KeyboardMapper::ConvertArToEn(const std::wstring& text) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::wstring result;
    result.reserve(text.length());
    
    for (wchar_t ch : text) {
        auto it = m_arToEnMap.find(ch);
        if (it != m_arToEnMap.end()) {
            result += it->second;
        } else {
            result += ch;
        }
    }
    
    return result;
}

bool KeyboardMapper::IsArabicChar(wchar_t ch) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_arabicChars.find(ch);
    return it != m_arabicChars.end() && it->second;
}

bool KeyboardMapper::IsEnglishChar(wchar_t ch) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_englishChars.find(ch);
    return it != m_englishChars.end() && it->second;
}

std::optional<KeyMapping> KeyboardMapper::GetKeyMapping(int vkCode) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = m_keyMappings.find(vkCode);
    if (it != m_keyMappings.end()) {
        return it->second;
    }
    return std::nullopt;
}

void KeyboardMapper::ClearCache() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_initialized = false;
    m_keyMappings.clear();
    m_enToArMap.clear();
    m_arToEnMap.clear();
    m_arabicChars.clear();
    m_englishChars.clear();
}

} // namespace mubaddil
