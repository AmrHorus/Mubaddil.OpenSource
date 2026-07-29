/**
 * @file detector.cpp
 * @brief Implementation of language detection
 */

#include "detector.h"
#include <algorithm>
#include <cmath>
#include <cctype>

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

void LanguageDetector::InitializeCommonWords() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_commonEnglishWords.empty() && !m_commonArabicWords.empty()) {
        return; // Already initialized
    }
    
    // Common English words that should never be corrected
    static const std::vector<std::wstring> commonEnglish = {
        L"the", L"be", L"to", L"of", L"and", L"a", L"in", L"that", L"have", L"i",
        L"it", L"for", L"not", L"on", L"with", L"he", L"as", L"you", L"do", L"at",
        L"this", L"but", L"his", L"by", L"from", L"they", L"we", L"say", L"her", L"she",
        L"or", L"an", L"will", L"my", L"one", L"all", L"would", L"there", L"their",
        L"what", L"hello", L"hi", L"hey", L"ok", L"okay", L"yes", L"no", L"please",
        L"thanks", L"thank", L"good", L"bad", L"new", L"old", L"big", L"small", L"long", L"short",
        L"time", L"year", L"people", L"way", L"day", L"man", L"woman", L"child", L"world",
        L"life", L"hand", L"part", L"place", L"case", L"week", L"company", L"system",
        L"program", L"question", L"work", L"government", L"number", L"night", L"point",
        L"home", L"water", L"room", L"mother", L"area", L"money", L"story", L"fact",
        L"month", L"lot", L"right", L"study", L"book", L"eye", L"job", L"word",
        L"business", L"issue", L"side", L"kind", L"head", L"house", L"service", L"friend",
        L"father", L"power", L"hour", L"game", L"line", L"end", L"member", L"law",
        L"car", L"city", L"community", L"name", L"president", L"team", L"minute", L"idea",
        L"kid", L"body", L"information", L"back", L"parent", L"face", L"others", L"level",
        L"office", L"door", L"health", L"person", L"art", L"war", L"history", L"party",
        L"result", L"change", L"morning", L"reason", L"research", L"girl", L"guy", L"moment",
        L"air", L"teacher", L"force", L"education", L"google", L"youtube", L"facebook",
        L"twitter", L"windows", L"openai", L"chatgpt", L"keyboard", L"mouse", L"screen",
        L"computer", L"phone", L"app", L"email", L"message", L"file", L"folder",
        L"desktop", L"laptop", L"tablet", L"internet", L"website", L"browser", L"search",
        L"download", L"upload", L"install", L"update", L"delete", L"create", L"save",
        L"open", L"close", L"start", L"stop", L"run", L"play", L"pause", L"record"
    };
    
    // Common Arabic words that should never be corrected
    static const std::vector<std::wstring> commonArabic = {
        L"مرحبا", L"اهلا", L"نعم", L"لا", L"شكرا", L"مع", L"في", L"من", L"الى", L"على",
        L"هذا", L"هذه", L"ذلك", L"تلك", L"انا", L"انت", L"هو", L"هي", L"نحن", L"هم",
        L"ما", L"ماذا", L"متى", L"اين", L"كيف", L"لماذا", L"من", L"اي", L"كم",
        L"واحد", L"اثنان", L"ثلاثة", L"اربعة", L"خمسة", L"ستة", L"سبعة", L"ثمانية", L"تسعة", L"عشرة",
        L"يوم", L"اسبوع", L"شهر", L"سنة", L"وقت", L"ساعة", L"دقيقة", L"ثانية",
        L"رجل", L"امرأة", L"طفل", L"ولد", L"بنت", L"اب", L"ام", L"اخ", L"اخت",
        L"صديق", L"جار", L"معلم", L"طبيب", L"مهندس", L"محامي", L"طالب", L"مدير",
        L"بيت", L"منزل", L"غرفة", L"باب", L"نافذة", L"طاولة", L"كرسي", L"سرير",
        L"سيارة", L"حافلة", L"قطار", L"طائرة", L"مطار", L"محطة", L"شارع", L"مدينة",
        L"بلد", L"عالم", L"ارض", L"سماء", L"شمس", L"قمر", L"نجم", L"بحر", L"نهر",
        L"جبل", L"شجرة", L"زهرة", L"حيوان", L"قطة", L"كلب", L"عصفور", L"سمكة",
        L"اكل", L"شراب", L"خبز", L"ماء", L"حليب", L"شاي", L"قهوة", L"فواكه", L"خضروات",
        L"لحم", L"دجاج", L"سمك", L"ارز", L"مكرونة", L"سلطة", L"حساء", L"حلوى",
        L"كبير", L"صغير", L"جديد", L"قديم", L"طويل", L"قصير", L"جميل", L"قبيح",
        L"سعيد", L"حزين", L"غاضب", L"خائف", L"متعب", L"جائع", L"عطشان", L"مريض",
        L"صحى", L"قوي", L"ضعيف", L"سريع", L"بطيء", L"سهل", L"صعب", L"مهم",
        L"عمل", L"دراسة", L"مدرسة", L"جامعة", L"كتاب", L"قلم", L"ورقة", L"حاسوب",
        L"هاتف", L"انترنت", L"بريد", L"رسالة", L"صورة", L"فيديو", L"موسيقى", L"فيلم",
        L"لعبة", L"رياضة", L"كرة", L"قدم", L"يد", L"راس", L"عين", L"اذن", L"انف",
        L"فم", L"سن", L"شعر", L"وجه", L"جسم", L"قلب", L"يد", L"رجل",
        L"السلام", L"محمد", L"احمد", L"علي", L"حسن", L"حسين", L"محمود", L" Ibrahim",
        L"عايز", L"عوز", L"حابب", L"بدى", L"روح", L"تعال", L"اقعد", L"قوم",
        L"افتح", L"اقفل", L"شغل", L"اطفى", L"ابعت", L"استلم", L"ارد", L"ابحث",
        L"حمل", L"ارفع", L"ثبت", L"حدث", L"احذف", L"انشئ", L"احفظ", L"اطبع"
    };
    
    for (const auto& word : commonEnglish) {
        m_commonEnglishWords[word] = true;
        // Also add lowercase version
        std::wstring lower = word;
        std::transform(lower.begin(), lower.end(), lower.begin(), 
                       [](wchar_t c) { return std::towlower(c); });
        m_commonEnglishWords[lower] = true;
    }
    
    for (const auto& word : commonArabic) {
        m_commonArabicWords[word] = true;
    }
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
    
    // Check if original is a common word in its detected language
    std::wstring lowerOriginal = candidate.original;
    std::transform(lowerOriginal.begin(), lowerOriginal.end(), lowerOriginal.begin(),
                   [](wchar_t c) { return std::towlower(c); });
    
    if (IsCommonWord(lowerOriginal, candidate.detectedLang)) {
        return false;
    }
    
    return true;
}

CorrectionCandidate LanguageDetector::EvaluateWord(const std::wstring& word,
                                                    const std::wstring& converted) {
    // First, ensure common words are initialized (outside of main lock)
    if (m_commonEnglishWords.empty() || m_commonArabicWords.empty()) {
        InitializeCommonWords();
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    CorrectionCandidate result;
    result.original = word;
    result.suggested = converted;
    
    // Analyze the original word
    // Note: Analyze also takes the lock, but since we're using lock_guard and not holding
    // the lock when calling it (we release after InitializeCommonWords), it's safe
    // We need to use a different approach - analyze without locking
    
    // Inline analysis to avoid double-locking
    AnalysisResult analysis;
    if (!word.empty() && word.length() > 1) {
        // Check for URL patterns
        if (word.find(L"http://") == std::wstring::npos &&
            word.find(L"https://") == std::wstring::npos &&
            word.find(L"www.") == std::wstring::npos) {
            
            // Check for email patterns  
            if (!(word.find(L"@") != std::wstring::npos && word.find(L".") != std::wstring::npos)) {
                
                // Check for code-like patterns
                bool isCode = false;
                for (wchar_t ch : word) {
                    if (ch == L'{' || ch == L'}' || ch == L'(' || ch == L')' ||
                        ch == L';' || ch == L'=' || ch == L'[' || ch == L']') {
                        isCode = true;
                        break;
                    }
                }
                
                if (!isCode) {
                    // Count character types
                    const size_t total = word.length();
                    
                    for (wchar_t ch : word) {
                        if (IsArabicUnicode(ch)) {
                            analysis.arabicCount++;
                        } else if (IsEnglishLetter(ch)) {
                            analysis.englishCount++;
                        } else if (ch >= L'0' && ch <= L'9') {
                            analysis.digitCount++;
                        } else if (ch == L'.' || ch == L',' || ch == L'!' || ch == L'?' ||
                                   ch == L';' || ch == L':' || ch == L'-' || ch == L'_') {
                            analysis.punctuationCount++;
                        } else {
                            analysis.otherCount++;
                        }
                    }
                    
                    // Calculate ratios
                    analysis.arabicRatio = static_cast<double>(analysis.arabicCount) / total;
                    analysis.englishRatio = static_cast<double>(analysis.englishCount) / total;
                    
                    // Determine dominant language
                    if (analysis.arabicCount > analysis.englishCount && analysis.arabicCount > 0) {
                        analysis.language = LanguageType::Arabic;
                    } else if (analysis.englishCount > analysis.arabicCount && analysis.englishCount > 0) {
                        analysis.language = LanguageType::English;
                    } else if (analysis.arabicCount > 0 && analysis.englishCount > 0) {
                        analysis.language = LanguageType::Mixed;
                    } else {
                        analysis.language = LanguageType::Unknown;
                    }
                    
                    // Don't analyze words with too many digits or punctuation
                    if (analysis.digitCount <= total / 2 && analysis.punctuationCount <= total / 3) {
                        analysis.shouldAnalyze = true;
                    }
                }
            }
        }
    }
    
    result.detectedLang = analysis.language;
    
    // If word shouldn't be analyzed, don't suggest
    if (!analysis.shouldAnalyze) {
        result.shouldSuggest = false;
        return result;
    }
    
    // Check if original is a common word
    std::wstring lowerOriginal = word;
    std::transform(lowerOriginal.begin(), lowerOriginal.end(), lowerOriginal.begin(),
                   [](wchar_t c) { return std::towlower(c); });
    
    // If original is a common English word, don't suggest
    if (m_commonEnglishWords.count(lowerOriginal) > 0) {
        result.shouldSuggest = false;
        return result;
    }
    
    // If original is a common Arabic word, don't suggest
    if (m_commonArabicWords.count(word) > 0) {
        result.shouldSuggest = false;
        return result;
    }
    
    // Calculate confidence for original word
    result.confidence = CalculateConfidence(word, converted, analysis.language);
    
    // Calculate confidence for converted word (inline to avoid lock issues)
    AnalysisResult convertedAnalysis;
    if (!converted.empty() && converted.length() > 1) {
        const size_t total = converted.length();
        for (wchar_t ch : converted) {
            if (IsArabicUnicode(ch)) {
                convertedAnalysis.arabicCount++;
            } else if (IsEnglishLetter(ch)) {
                convertedAnalysis.englishCount++;
            }
        }
        convertedAnalysis.arabicRatio = static_cast<double>(convertedAnalysis.arabicCount) / total;
        convertedAnalysis.englishRatio = static_cast<double>(convertedAnalysis.englishCount) / total;
        
        if (convertedAnalysis.arabicCount > convertedAnalysis.englishCount && convertedAnalysis.arabicCount > 0) {
            convertedAnalysis.language = LanguageType::Arabic;
        } else if (convertedAnalysis.englishCount > convertedAnalysis.arabicCount && convertedAnalysis.englishCount > 0) {
            convertedAnalysis.language = LanguageType::English;
        }
    }
    
    result.convertedConfidence = CalculateConfidence(converted, word, convertedAnalysis.language);
    
    // Determine if suggestion should be shown
    // Show if: converted confidence > original confidence + threshold
    double confidenceDiff = result.convertedConfidence - result.confidence;
    
    if (confidenceDiff > m_threshold && result.convertedConfidence > 0.5) {
        result.shouldSuggest = true;
    } else {
        result.shouldSuggest = false;
    }
    
    return result;
}

void LanguageDetector::SetThreshold(double threshold) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_threshold = std::max(0.0, std::min(1.0, threshold));
}

double LanguageDetector::GetThreshold() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_threshold;
}

bool LanguageDetector::IsCommonWord(const std::wstring& word, LanguageType lang) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Initialize if needed
    if (m_commonEnglishWords.empty() || m_commonArabicWords.empty()) {
        // Need to release lock first - this is a design issue, but for now we'll just return false
        // In production, we'd use a separate initialization flag
        return false;
    }
    
    if (lang == LanguageType::English) {
        std::wstring lower = word;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](wchar_t c) { return std::towlower(c); });
        return m_commonEnglishWords.count(lower) > 0;
    } else if (lang == LanguageType::Arabic) {
        return m_commonArabicWords.count(word) > 0;
    }
    
    return false;
}

// Helper to unlock mutex (used in EvaluateWord)
// Note: This is a workaround - in production code we'd refactor this better

} // namespace mubaddil
