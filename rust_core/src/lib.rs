//! Mubaddil Core - Intelligent Keyboard Layout Switcher
//! 
//! This module provides the core engine for detecting and correcting
//! keyboard layout mistakes in real-time.
//! 
//! Architecture:
//! - Rust: Core intelligence (detection, mapping, matching)
//! - C++: Windows integration (hooks, UI, clipboard)
//! - FFI: Clean C-compatible boundary between them

#![allow(dead_code)]
#![allow(unused_variables)]

use std::collections::{HashMap, HashSet};
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_int, c_void};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};

use fuzzy_matcher::skim::SkimMatcherV2;
use once_cell::sync::Lazy;
use serde::{Deserialize, Serialize};
use strsim::levenshtein;

// ============================================================================
// Constants
// ============================================================================

const MAX_WORD_LENGTH: usize = 50;
const MIN_WORD_LENGTH: usize = 3;

// ============================================================================
// Arabic-English Keyboard Mapping (Saudi Arabic Layout)
// ============================================================================

/// Complete mapping from Arabic characters to English (when user typed with Arabic layout but meant English)
static ARABIC_TO_ENGLISH: Lazy<HashMap<char, char>> = Lazy::new(|| {
    [
        // Top row (numbers and symbols)
        ('٠', '0'), ('١', '1'), ('٢', '2'), ('٣', '3'), ('٤', '4'),
        ('٥', '5'), ('٦', '6'), ('٧', '7'), ('٨', '8'), ('٩', '9'),
        // QWERTY row
        ('ض', 'q'), ('َ', 'Q'),
        ('ص', 'w'), ('ً', 'W'),
        ('ث', 'e'), ('ُ', 'E'),
        ('ق', 'r'), ('ٌ', 'R'),
        ('ف', 't'), ('ل', 'T'),
        ('غ', 'y'), ('إ', 'Y'),
        ('ع', 'u'), ('\'', 'U'),
        ('ه', 'i'), ('÷', 'I'),
        ('خ', 'o'), ('×', 'O'),
        ('ح', 'p'), ('؛', 'P'),
        ('ج', '['), ('<', '{'),
        ('د', ']'), ('>', '}'),
        // ASDF row
        ('ش', 'a'), ('ِ', 'A'),
        ('س', 's'), ('ٍ', 'S'),
        ('ي', 'd'), (']', 'D'),
        ('ب', 'f'), ('[', 'F'),
        ('ل', 'g'), ('ل', 'G'),  // lam-alif special case
        ('ا', 'h'), ('أ', 'H'),
        ('ت', 'j'), ('ـ', 'J'),
        ('ن', 'k'), ('،', 'K'),
        ('م', 'l'), ('/', 'L'),
        ('ك', ';'), (':', ':'),
        ('ط', '\''), ('"', '"'),
        // ZXCV row
        ('ئ', 'z'), ('~', 'Z'),
        ('ء', 'x'), ('ْ', 'X'),
        ('ؤ', 'c'), ('}', 'C'),
        ('ر', 'v'), ('{', 'V'),
        ('لا', 'b'), ('آ', 'B'),  // lam-alif
        ('ى', 'n'), ('\'', 'N'),
        ('ة', 'm'), ('\'', 'M'),
        ('و', ','), (',', ','),
        ('ز', '.'), ('.', '.'),
        ('ظ', '/'), ('?', '?'),
        // Other keys
        (' ', ' '), (' ', ' '),
        ('-', '-'), ('_', '_'),
        ('=', '='), ('+', '+'),
        ('\\', '\\'), ('|', '|'),
        ('ذ', '`'), ('ّ', '~'),
    ].iter().cloned().collect()
});

/// Complete mapping from English characters to Arabic (when user typed with English layout but meant Arabic)
static ENGLISH_TO_ARABIC: Lazy<HashMap<char, char>> = Lazy::new(|| {
    [
        // Top row (numbers)
        ('0', '٠'), ('1', '١'), ('2', '٢'), ('3', '٣'), ('4', '٤'),
        ('5', '٥'), ('6', '٦'), ('7', '٧'), ('8', '٨'), ('9', '٩'),
        // QWERTY row
        ('q', 'ض'), ('Q', 'َ'),
        ('w', 'ص'), ('W', 'ً'),
        ('e', 'ث'), ('E', 'ُ'),
        ('r', 'ق'), ('R', 'ٌ'),
        ('t', 'ف'), ('T', 'ل'),
        ('y', 'غ'), ('Y', 'إ'),
        ('u', 'ع'), ('U', '\''),
        ('i', 'ه'), ('I', '÷'),
        ('o', 'خ'), ('O', '×'),
        ('p', 'ح'), ('P', '؛'),
        ('[', 'ج'), ('{', '<'),
        (']', 'د'), ('}', '>'),
        // ASDF row
        ('a', 'ش'), ('A', 'ِ'),
        ('s', 'س'), ('S', 'ٍ'),
        ('d', 'ي'), ('D', ']'),
        ('f', 'ب'), ('F', '['),
        ('g', 'ل'), ('G', 'ل'),
        ('h', 'ا'), ('H', 'أ'),
        ('j', 'ت'), ('J', 'ـ'),
        ('k', 'ن'), ('K', ','),
        ('l', 'م'), ('L', '/'),
        (';', 'ك'), (':', ':'),
        ('\'', 'ط'), ('"', '"'),
        // ZXCV row
        ('z', 'ئ'), ('Z', '~'),
        ('x', 'ء'), ('X', '_'),
        ('c', 'ؤ'), ('C', '}'),
        ('v', 'ر'), ('V', '{'),
        ('b', 'لا'), ('B', 'آ'),
        ('n', 'ى'), ('N', '\''),
        ('m', 'ة'), ('M', '\''),
        (',', 'و'), ('<', ','),
        ('.', 'ز'), ('>', '.'),
        ('/', 'ظ'), ('?', '؟'),
        // Other keys
        (' ', ' '), (' ', ' '),
        ('-', '-'), ('_', '_'),
        ('=', '='), ('+', '+'),
        ('\\', '\\'), ('|', '|'),
        ('`', 'ذ'), ('~', 'ّ'),
    ].iter().cloned().collect()
});

/// Set of Arabic characters for quick lookup
static ARABIC_CHARS: Lazy<HashSet<char>> = Lazy::new(|| {
    ARABIC_TO_ENGLISH.keys().cloned().collect()
});

/// Set of English characters for quick lookup
static ENGLISH_CHARS: Lazy<HashSet<char>> = Lazy::new(|| {
    ENGLISH_TO_ARABIC.keys().cloned().collect()
});

// ============================================================================
// Common Words Dictionaries
// ============================================================================

static COMMON_ENGLISH_WORDS: Lazy<HashSet<String>> = Lazy::new(|| {
    [
        "the", "be", "to", "of", "and", "a", "in", "that", "have", "it",
        "for", "not", "on", "with", "he", "as", "you", "do", "at", "this",
        "but", "his", "by", "from", "they", "we", "say", "her", "she", "or",
        "an", "will", "my", "one", "all", "would", "there", "their", "what",
        "so", "up", "out", "if", "about", "who", "get", "which", "go", "me",
        "hello", "world", "test", "example", "keyboard", "layout", "switch",
        "typing", "correct", "error", "fix", "auto", "smart", "intelligent",
        "yes", "no", "ok", "okay", "thanks", "please", "good", "bad",
        "new", "old", "big", "small", "long", "short", "time", "work",
        "just", "also", "make", "like", "know", "take", "come", "see",
        "use", "find", "give", "tell", "name", "first", "people", "over",
    ].iter().map(|s| s.to_string()).collect()
});

static COMMON_ARABIC_WORDS: Lazy<HashSet<String>> = Lazy::new(|| {
    [
        "مرحبا", "العالم", "اختبار", "مثال", "لوحة", "مفتاح", "تبديل",
        "كتابة", "تصحيح", "خطأ", "إصلاح", "تلقائي", "ذكي", "عربي",
        "انجليزي", "كلمة", "جملة", "نص", "رسالة", "بريد", "هاتف",
        "في", "من", "على", "إلى", "عن", "أن", "إن", "كان", "قد", "لا",
        "ما", "مع", "هو", "هي", "نحن", "أنا", "أنت", "هم", "كتاب", "بيت",
        "بين", "منذ", "حتى", "ثم", "إذا", "لأن", "هذا", "ذلك", "تلك",
        "الله", "محمد", "علي", "أحمد", "عمر", "خالد", "سعود", "شكرا",
        "سلام", "صباح", "مساء", "ليلة", "يوم", "شهر", "سنة", "نعم", "لا",
    ].iter().map(|s| s.to_string()).collect()
});

// ============================================================================
// Data Structures
// ============================================================================

/// Language detection result
#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum LanguageType {
    Arabic,
    English,
    Mixed,
    Unknown,
}

/// Direction of conversion
#[derive(Debug, Clone, Copy, Serialize, Deserialize)]
pub enum ConversionDirection {
    EnToAr,  // User typed English but meant Arabic
    ArToEn,  // User typed Arabic but meant English
}

/// Analysis result for a word
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AnalysisResult {
    pub language: LanguageType,
    pub arabic_count: usize,
    pub english_count: usize,
    pub digit_count: usize,
    pub punctuation_count: usize,
    pub other_count: usize,
    pub arabic_ratio: f64,
    pub english_ratio: f64,
    pub is_likely_url: bool,
    pub is_likely_email: bool,
    pub is_likely_code: bool,
    pub should_analyze: bool,
}

impl Default for AnalysisResult {
    fn default() -> Self {
        Self {
            language: LanguageType::Unknown,
            arabic_count: 0,
            english_count: 0,
            digit_count: 0,
            punctuation_count: 0,
            other_count: 0,
            arabic_ratio: 0.0,
            english_ratio: 0.0,
            is_likely_url: false,
            is_likely_email: false,
            is_likely_code: false,
            should_analyze: true,
        }
    }
}

/// Correction suggestion result
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CorrectionResult {
    pub original: String,
    pub suggested: String,
    pub direction: ConversionDirection,
    pub confidence: f64,
    pub should_suggest: bool,
}

impl Default for CorrectionResult {
    fn default() -> Self {
        Self {
            original: String::new(),
            suggested: String::new(),
            direction: ConversionDirection::ArToEn,
            confidence: 0.0,
            should_suggest: false,
        }
    }
}

// ============================================================================
// Core Functions
// ============================================================================

/// Convert text typed with Arabic layout to English
fn arabic_layout_to_english(text: &str) -> String {
    text.chars()
        .map(|c| *ARABIC_TO_ENGLISH.get(&c).unwrap_or(&c))
        .collect()
}

/// Convert text typed with English layout to Arabic
fn english_layout_to_arabic(text: &str) -> String {
    text.chars()
        .map(|c| *ENGLISH_TO_ARABIC.get(&c).unwrap_or(&c))
        .collect()
}

/// Check if character is Arabic Unicode
fn is_arabic_char(c: char) -> bool {
    ARABIC_CHARS.contains(&c) || 
    matches!(c as u32, 
        0x0600..=0x06FF |      // Arabic
        0xFB50..=0xFDFF |      // Arabic Presentation Forms-A
        0xFE70..=0xFEFF        // Arabic Presentation Forms-B
    )
}

/// Check if character is English letter
fn is_english_char(c: char) -> bool {
    c.is_ascii_alphabetic()
}

/// Check if character is digit
fn is_digit_char(c: char) -> bool {
    c.is_ascii_digit() || matches!(c as u32, 0x0660..=0x0669)  // Arabic-Indic digits
}

/// Check if character is punctuation
fn is_punctuation_char(c: char) -> bool {
    c.is_ascii_punctuation() || matches!(c as u32, 
        0x060C | 0x061B | 0x061F |  // Arabic punctuation
        0x0640 | 0x064B..=0x065F    // Arabic diacritics
    )
}

/// Analyze a word to determine its language characteristics
pub fn analyze_word(word: &str) -> AnalysisResult {
    if word.is_empty() {
        return AnalysisResult::default();
    }

    let mut result = AnalysisResult::default();
    
    for ch in word.chars() {
        if is_arabic_char(ch) {
            result.arabic_count += 1;
        } else if is_english_char(ch) {
            result.english_count += 1;
        } else if is_digit_char(ch) {
            result.digit_count += 1;
        } else if is_punctuation_char(ch) {
            result.punctuation_count += 1;
        } else {
            result.other_count += 1;
        }
    }

    let total = word.len() as f64;
    result.arabic_ratio = result.arabic_count as f64 / total;
    result.english_ratio = result.english_count as f64 / total;

    // Determine dominant language
    if result.arabic_count > result.english_count && result.arabic_count > 0 {
        result.language = LanguageType::Arabic;
    } else if result.english_count > result.arabic_count && result.english_count > 0 {
        result.language = LanguageType::English;
    } else if result.arabic_count > 0 && result.english_count > 0 {
        result.language = LanguageType::Mixed;
    } else {
        result.language = LanguageType::Unknown;
    }

    // Check for special cases
    result.is_likely_url = word.starts_with("http://") || 
                           word.starts_with("https://") || 
                           word.starts_with("www.") ||
                           (word.contains('.') && word.contains('/'));
    
    result.is_likely_email = word.contains('@') && word.contains('.');
    result.is_likely_code = word.contains(|c| "{}[]();=".contains(c));
    
    // Don't analyze words with digits, URLs, emails, or code
    result.should_analyze = !(result.digit_count > 0) &&
                            !result.is_likely_url &&
                            !result.is_likely_email &&
                            !result.is_likely_code;

    result
}

/// Check if a word is valid (exists in dictionary or passes fuzzy matching)
fn is_valid_word(word: &str) -> bool {
    if word.len() < MIN_WORD_LENGTH {
        return false;
    }

    let word_lower = word.to_lowercase();
    
    // Direct dictionary match
    if COMMON_ENGLISH_WORDS.contains(&word_lower) {
        return true;
    }
    
    if COMMON_ARABIC_WORDS.contains(word) {
        return true;
    }

    // Fuzzy matching for English
    let matcher = SkimMatcherV2::default();
    for dict_word in COMMON_ENGLISH_WORDS.iter() {
        if matcher.fuzzy_match(dict_word, &word_lower).is_some() {
            return true;
        }
    }

    // Levenshtein distance for near-matches
    for dict_word in COMMON_ENGLISH_WORDS.iter() {
        if levenshtein(dict_word, &word_lower) <= 1 && word_lower.len() >= 4 {
            return true;
        }
    }

    false
}

/// Calculate confidence score for a correction
fn calculate_confidence(original: &str, corrected: &str, direction: ConversionDirection) -> f64 {
    if corrected.is_empty() {
        return 0.0;
    }

    let mut score = 0.0;

    // Length match bonus
    let len_diff = (original.len() as i32 - corrected.len() as i32).abs() as f64;
    score += (1.0 - (len_diff / original.len().max(1) as f64)).max(0.0) * 0.3;

    // Valid word bonus
    if is_valid_word(corrected) {
        score += 0.5;
    }

    // Common word bonus
    let corrected_lower = corrected.to_lowercase();
    if COMMON_ENGLISH_WORDS.contains(&corrected_lower) || COMMON_ARABIC_WORDS.contains(corrected) {
        score += 0.2;
    }

    score.min(1.0)
}

/// Detect and correct a word typed with wrong keyboard layout
pub fn detect_and_correct(word: &str) -> CorrectionResult {
    if word.is_empty() || word.len() < MIN_WORD_LENGTH {
        return CorrectionResult::default();
    }

    let analysis = analyze_word(word);
    
    // Skip analysis for certain cases
    if !analysis.should_analyze {
        return CorrectionResult::default();
    }

    // If already valid, no correction needed
    if is_valid_word(word) {
        return CorrectionResult::default();
    }

    // Try Arabic -> English conversion
    if analysis.language == LanguageType::Arabic || analysis.arabic_ratio > 0.5 {
        let converted_en = arabic_layout_to_english(word);
        if is_valid_word(&converted_en) {
            let confidence = calculate_confidence(word, &converted_en, ConversionDirection::ArToEn);
            return CorrectionResult {
                original: word.to_string(),
                suggested: converted_en,
                direction: ConversionDirection::ArToEn,
                confidence,
                should_suggest: confidence > 0.5,
            };
        }
    }

    // Try English -> Arabic conversion
    if analysis.language == LanguageType::English || analysis.english_ratio > 0.5 {
        let converted_ar = english_layout_to_arabic(word);
        if is_valid_word(&converted_ar) {
            let confidence = calculate_confidence(word, &converted_ar, ConversionDirection::EnToAr);
            return CorrectionResult {
                original: word.to_string(),
                suggested: converted_ar,
                direction: ConversionDirection::EnToAr,
                confidence,
                should_suggest: confidence > 0.5,
            };
        }
    }

    CorrectionResult::default()
}

// ============================================================================
// C FFI Interface
// ============================================================================

/// Opaque handle to the correction engine
pub struct CorrectionEngine {
    _private: (),
}

/// Result structure for C FFI
#[repr(C)]
pub struct FfiCorrectionResult {
    original: *mut c_char,
    suggested: *mut c_char,
    direction: c_int,  // 0 = ArToEn, 1 = EnToAr
    confidence: f64,
    should_suggest: bool,
}

/// Free a C string allocated by the library
#[no_mangle]
pub extern "C" fn mubaddil_free_string(ptr: *mut c_char) {
    if !ptr.is_null() {
        unsafe {
            let _ = CString::from_raw(ptr);
        }
    }
}

/// Create a new correction engine instance
#[no_mangle]
pub extern "C" fn mubaddil_engine_create() -> *mut CorrectionEngine {
    Box::into_raw(Box::new(CorrectionEngine { _private: () }))
}

/// Destroy a correction engine instance
#[no_mangle]
pub extern "C" fn mubaddil_engine_destroy(engine: *mut CorrectionEngine) {
    if !engine.is_null() {
        unsafe {
            let _ = Box::from_raw(engine);
        }
    }
}

/// Analyze a word and return correction result
/// Returns null if no correction needed
#[no_mangle]
pub extern "C" fn mubaddil_analyze_word(
    engine: *mut CorrectionEngine,
    word: *const c_char,
) -> *mut FfiCorrectionResult {
    if word.is_null() {
        return std::ptr::null_mut();
    }

    let word_str = unsafe {
        match CStr::from_ptr(word).to_str() {
            Ok(s) => s,
            Err(_) => return std::ptr::null_mut(),
        }
    };

    let result = detect_and_correct(word_str);
    
    if !result.should_suggest {
        return std::ptr::null_mut();
    }

    let ffi_result = Box::new(FfiCorrectionResult {
        original: CString::new(result.original).unwrap_or_default().into_raw(),
        suggested: CString::new(result.suggested).unwrap_or_default().into_raw(),
        direction: match result.direction {
            ConversionDirection::ArToEn => 0,
            ConversionDirection::EnToAr => 1,
        },
        confidence: result.confidence,
        should_suggest: result.should_suggest,
    });

    Box::into_raw(ffi_result)
}

/// Free a correction result
#[no_mangle]
pub extern "C" fn mubaddil_free_result(result: *mut FfiCorrectionResult) {
    if !result.is_null() {
        unsafe {
            let ffi_result = Box::from_raw(result);
            mubaddil_free_string(ffi_result.original);
            mubaddil_free_string(ffi_result.suggested);
        }
    }
}

/// Get version string
#[no_mangle]
pub extern "C" fn mubaddil_version() -> *const c_char {
    c"0.1.0\0".as_ptr()
}

// ============================================================================
// Tests
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_arabic_to_english_hello() {
        // "اثممخ" should map to "hello"
        let result = arabic_layout_to_english("اثممخ");
        assert_eq!(result, "hello");
    }

    #[test]
    fn test_english_to_arabic() {
        // Test basic English to Arabic conversion
        let result = english_layout_to_arabic("hello");
        assert!(!result.is_empty());
    }

    #[test]
    fn test_detect_and_correct_hello() {
        let result = detect_and_correct("اثممخ");
        assert!(result.should_suggest);
        assert_eq!(result.suggested, "hello");
        assert_eq!(result.direction, ConversionDirection::ArToEn);
    }

    #[test]
    fn test_is_valid_word() {
        assert!(is_valid_word("hello"));
        assert!(is_valid_word("the"));
        assert!(!is_valid_word("xyzabc"));
        assert!(!is_valid_word("ab"));  // Too short
    }

    #[test]
    fn test_analyze_word_arabic() {
        let result = analyze_word("مرحبا");
        assert_eq!(result.language, LanguageType::Arabic);
        assert!(result.arabic_count > 0);
    }

    #[test]
    fn test_analyze_word_english() {
        let result = analyze_word("hello");
        assert_eq!(result.language, LanguageType::English);
        assert!(result.english_count > 0);
    }

    #[test]
    fn test_analyze_word_mixed() {
        let result = analyze_word("helloمرحبا");
        assert_eq!(result.language, LanguageType::Mixed);
    }

    #[test]
    fn test_analyze_word_skip_url() {
        let result = analyze_word("https://example.com");
        assert!(result.is_likely_url);
        assert!(!result.should_analyze);
    }

    #[test]
    fn test_analyze_word_skip_email() {
        let result = analyze_word("test@example.com");
        assert!(result.is_likely_email);
        assert!(!result.should_analyze);
    }

    #[test]
    fn test_confidence_calculation() {
        let result = detect_and_correct("اثممخ");
        assert!(result.confidence > 0.5);
    }
}
