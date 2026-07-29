#pragma once

/**
 * @file detector.h
 * @brief Language detection for typed text
 * 
 * Analyzes text to determine if it's Arabic, English, or mixed.
 * Provides confidence scoring for layout mismatch detection.
 */

#ifndef DETECTOR_H
#define DETECTOR_H

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace mubaddil {

/**
 * @enum LanguageType
 * @brief Detected language type
 */
enum class LanguageType {
    Arabic,
    English,
    Mixed,
    Unknown
};

/**
 * @struct AnalysisResult
 * @brief Result of text analysis
 */
struct AnalysisResult {
    LanguageType language;      ///< Detected primary language
    size_t arabicCount;         ///< Number of Arabic characters
    size_t englishCount;        ///< Number of English letters
    size_t digitCount;          ///< Number of digits
    size_t punctuationCount;    ///< Number of punctuation marks
    size_t otherCount;          ///< Other characters
    double arabicRatio;         ///< Ratio of Arabic chars (0.0-1.0)
    double englishRatio;        ///< Ratio of English chars (0.0-1.0)
    bool isLikelyUrl;           ///< Appears to be a URL
    bool isLikelyEmail;         ///< Appears to be an email
    bool isLikelyCode;          ///< Appears to be code/syntax
    bool shouldAnalyze;         ///< Whether this word should be analyzed
    
    AnalysisResult()
        : language(LanguageType::Unknown)
        , arabicCount(0), englishCount(0), digitCount(0)
        , punctuationCount(0), otherCount(0)
        , arabicRatio(0.0), englishRatio(0.0)
        , isLikelyUrl(false), isLikelyEmail(false)
        , isLikelyCode(false), shouldAnalyze(true) {}
};

/**
 * @struct CorrectionCandidate
 * @brief A potential correction with confidence score
 */
struct CorrectionCandidate {
    std::wstring original;      ///< Original typed word
    std::wstring suggested;     ///< Suggested correction
    LanguageType detectedLang;  ///< Detected language of original
    double confidence;          ///< Confidence score (0.0-1.0)
    bool shouldSuggest;         ///< Whether to show suggestion
    
    CorrectionCandidate()
        : detectedLang(LanguageType::Unknown)
        , confidence(0.0), shouldSuggest(false) {}
};

/**
 * @class LanguageDetector
 * @brief Analyzes text to detect language and suggest corrections
 * 
 * Uses character-level analysis and pattern matching to determine
 * if text was typed with the wrong keyboard layout.
 */
class LanguageDetector {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static LanguageDetector& Instance();
    
    /**
     * @brief Analyze a word to determine its language characteristics
     * @param word Word to analyze
     * @return Analysis result with language detection details
     */
    AnalysisResult Analyze(const std::wstring& word);
    
    /**
     * @brief Check if a character is an Arabic Unicode character
     * @param ch Character to check
     * @return true if Arabic character
     */
    static bool IsArabicUnicode(wchar_t ch) noexcept;
    
    /**
     * @brief Check if a character is an English letter
     * @param ch Character to check
     * @return true if English letter
     */
    static bool IsEnglishLetter(wchar_t ch) noexcept;
    
    /**
     * @brief Calculate confidence score for a potential correction
     * @param original Original typed word
     * @param converted Converted word
     * @param detectedLang Detected language of original
     * @return Confidence score between 0.0 and 1.0
     */
    double CalculateConfidence(const std::wstring& original,
                               const std::wstring& converted,
                               LanguageType detectedLang);
    
    /**
     * @brief Determine if a suggestion should be shown
     * @param candidate Correction candidate
     * @return true if suggestion should be displayed
     */
    bool ShouldShowSuggestion(const CorrectionCandidate& candidate);
    
    // Delete copy/move operations
    LanguageDetector(const LanguageDetector&) = delete;
    LanguageDetector& operator=(const LanguageDetector&) = delete;
    LanguageDetector(LanguageDetector&&) = delete;
    LanguageDetector& operator=(LanguageDetector&&) = delete;
    
private:
    LanguageDetector() = default;
    ~LanguageDetector() = default;
    
    mutable std::mutex m_mutex;                          ///< Thread synchronization
    std::unordered_map<std::wstring, bool> m_commonWords; ///< Common words cache
};

} // namespace mubaddil

#endif // DETECTOR_H
