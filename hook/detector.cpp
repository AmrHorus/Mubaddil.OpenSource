/**
 * @file detector.cpp
 * @brief Implementation of language detection
 */

#include "detector.h"
#include <algorithm>
#include <cmath>

namespace mubaddil {

LanguageDetector& LanguageDetector::Instance() {
    static LanguageDetector instance;
    return instance;
}

bool LanguageDetector::IsArabicUnicode(wchar_t ch) noexcept {
    // Arabic Unicode range: U+0600 to U+06FF
    // Arabic Presentation Forms: U+FB50 to U+FDFF
    unsigned int code = static_cast<unsigned int>(ch);
    return (code >= 0x0600 && code <= 0x06FF) ||
           (code >= 0xFB50 && code <= 0xFDFF) ||
           (code >= 0xFE70 && code <= 0xFEFF);
}

bool LanguageDetector::IsEnglishLetter(wchar_t ch) noexcept {
    return (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');
}

AnalysisResult LanguageDetector::Analyze(const std::wstring& word) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    AnalysisResult result;
    
    if (word.empty()) {
        result.shouldAnalyze = false;
        return result;
    }
    
    // Quick checks for non-analyzable content
    if (word.length() == 1) {
        result.shouldAnalyze = false;
        return result;
    }
    
    // Check for URL patterns
    if (word.find(L"http://") != std::wstring::npos ||
        word.find(L"https://") != std::wstring::npos ||
        word.find(L"www.") != std::wstring::npos) {
        result.isLikelyUrl = true;
        result.shouldAnalyze = false;
        return result;
    }
    
    // Check for email patterns
    if (word.find(L"@") != std::wstring::npos && word.find(L".") != std::wstring::npos) {
        result.isLikelyEmail = true;
        result.shouldAnalyze = false;
        return result;
    }
    
    // Check for code-like patterns
    for (wchar_t ch : word) {
        if (ch == L'{' || ch == L'}' || ch == L'(' || ch == L')' ||
            ch == L';' || ch == L'=' || ch == L'[' || ch == L']') {
            result.isLikelyCode = true;
            result.shouldAnalyze = false;
            return result;
        }
    }
    
    // Count character types
    const size_t total = word.length();
    
    for (wchar_t ch : word) {
        if (IsArabicUnicode(ch)) {
            result.arabicCount++;
        } else if (IsEnglishLetter(ch)) {
            result.englishCount++;
        } else if (ch >= L'0' && ch <= L'9') {
            result.digitCount++;
        } else if (ch == L'.' || ch == L',' || ch == L'!' || ch == L'?' ||
                   ch == L';' || ch == L':' || ch == L'-' || ch == L'_') {
            result.punctuationCount++;
        } else {
            result.otherCount++;
        }
    }
    
    // Calculate ratios
    result.arabicRatio = static_cast<double>(result.arabicCount) / total;
    result.englishRatio = static_cast<double>(result.englishCount) / total;
    
    // Determine dominant language
    if (result.arabicCount > result.englishCount && result.arabicCount > 0) {
        result.language = LanguageType::Arabic;
    } else if (result.englishCount > result.arabicCount && result.englishCount > 0) {
        result.language = LanguageType::English;
    } else if (result.arabicCount > 0 && result.englishCount > 0) {
        result.language = LanguageType::Mixed;
    } else {
        result.language = LanguageType::Unknown;
    }
    
    // Don't analyze words with too many digits or punctuation
    if (result.digitCount > total / 2 || result.punctuationCount > total / 3) {
        result.shouldAnalyze = false;
    }
    
    return result;
}

double LanguageDetector::CalculateConfidence(const std::wstring& original,
                                              const std::wstring& converted,
                                              LanguageType detectedLang) {
    if (original.empty() || converted.empty()) {
        return 0.0;
    }
    
    // Base confidence from length
    double lengthScore = 0.0;
    if (original.length() >= 3) {
        lengthScore = 0.3;
    } else if (original.length() >= 2) {
        lengthScore = 0.15;
    }
    
    // Confidence from character conversion ratio
    size_t convertedChars = 0;
    for (size_t i = 0; i < std::min(original.length(), converted.length()); i++) {
        if (original[i] != converted[i]) {
            convertedChars++;
        }
    }
    
    double conversionScore = static_cast<double>(convertedChars) / original.length();
    
    // Boost confidence if detected language differs from expected keyboard layout
    double layoutMismatchBonus = 0.0;
    if (detectedLang == LanguageType::Arabic || detectedLang == LanguageType::English) {
        layoutMismatchBonus = 0.2;
    }
    
    // Penalty for mixed content
    double mixedPenalty = 0.0;
    auto analysis = Analyze(original);
    if (analysis.language == LanguageType::Mixed) {
        mixedPenalty = -0.2;
    }
    
    // Combine scores
    double confidence = lengthScore + (conversionScore * 0.5) + layoutMismatchBonus + mixedPenalty;
    
    // Clamp to [0.0, 1.0]
    return std::max(0.0, std::min(1.0, confidence));
}

bool LanguageDetector::ShouldShowSuggestion(const CorrectionCandidate& candidate) {
    if (!candidate.shouldSuggest) {
        return false;
    }
    
    if (candidate.confidence < 0.4) {
        return false;
    }
    
    if (candidate.original == candidate.suggested) {
        return false;
    }
    
    // Don't suggest if original is already a common word
    std::wstring lowerOriginal = candidate.original;
    std::transform(lowerOriginal.begin(), lowerOriginal.end(), lowerOriginal.begin(),
                   [](wchar_t c) { return std::towlower(c); });
    
    // Common English words that shouldn't be corrected
    static const std::vector<std::wstring> commonEnglish = {
        L"the", L"be", L"to", L"of", L"and", L"a", L"in", L"that", L"have", L"i",
        L"it", L"for", L"not", L"on", L"with", L"he", L"as", L"you", L"do", L"at",
        L"this", L"but", L"his", L"by", L"from", L"they", L"we", L"say", L"her", L"she",
        L"or", L"an", L"will", L"my", L"one", L"all", L"would", L"there", L"their",
        L"what", L"hello", L"hi", L"hey", L"ok", L"okay", L"yes", L"no", L"please",
        L"thanks", L"good", L"bad", L"new", L"old", L"big", L"small", L"long", L"short"
    };
    
    for (const auto& common : commonEnglish) {
        if (lowerOriginal == common) {
            return false;
        }
    }
    
    return true;
}

} // namespace mubaddil
