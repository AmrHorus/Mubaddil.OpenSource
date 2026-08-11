//! Text processing utilities for Mubaddil Core

use crate::keyboard::{is_arabic_char, is_english_letter};

/// Common English words for validation
pub const COMMON_ENGLISH_WORDS: &[&str] = &[
    "the", "be", "to", "of", "and", "a", "in", "that", "have", "it",
    "for", "not", "on", "with", "he", "as", "you", "do", "at", "this",
    "but", "his", "by", "from", "they", "we", "say", "her", "she", "or",
    "an", "will", "my", "one", "all", "would", "there", "their", "what",
    "so", "up", "out", "if", "about", "who", "get", "which", "go", "me",
    "hello", "world", "test", "example", "keyboard", "layout", "switch",
    "typing", "correct", "error", "fix", "auto", "smart", "intelligent",
    "when", "than", "then", "been", "has", "him", "first", "each", "its",
    "new", "after", "two", "into", "other", "can", "had", "let", "could",
    "come", "over", "just", "take", "make", "like", "know", "time", "very",
    "see", "look", "more", "day", "way", "think", "good", "now", "old",
    "also", "only", "most", "should", "even", "back", "own", "right",
    "use", "any", "well", "still", "try", "left", "turn", "mean", "really",
    "before", "great", "again", "off", "long", "little", "place", "high",
    "show", "house", "point", "group", "another", "begin", "start",
    "through", "question", "number", "part", "child", "eye", "woman",
    "system", "program", "hand", "large", "small", "end", "problem",
    "read", "include", "public", "follow", "stand", "probably", "build",
    "nation", "country", "side", "fact", "hi", "hey", "ok", "okay",
    "yes", "no", "please", "thanks", "thanks", "good", "bad", "big",
];

/// Common Arabic words for validation
pub const COMMON_ARABIC_WORDS: &[&str] = &[
    "في", "من", "على", "إلى", "عن", "أن", "إن", "كان", "قد", "لا",
    "ما", "مع", "هو", "هي", "نحن", "أنا", "أنت", "هم", "كتاب", "بيت",
    "بين", "منذ", "حتى", "ثم", "إذا", "لأن", "هذا", "ذلك", "تلك",
    "الله", "محمد", "علي", "أحمد", "عمر", "خالد", "سعود", "عربي",
    "شكرا", "مرحبا", "سلام", "صباح", "مساء", "ليلة", "يوم", "شهر", "سنة",
    "عمل", "دراسة", "مدرسة", "جامعة", "طالب", "معلم", "طبيب", "مهندس",
    "سيارة", "طائرة", "قطار", "حافلة", "طريق", "مدينة", "قرية", "بلد",
    "ماء", "طعام", "خبز", "لحم", "دجاج", "سمك", "فواكه", "خضروات",
    "كبير", "صغير", "جديد", "قديم", "جميل", "قبيح", "سريع", "بطيء",
    "قوي", "ضعيف", "غني", "فقير", "سعيد", "حزين", "غاضب", "هادئ",
];

/// Analyze text to determine language characteristics
#[derive(Debug, Clone)]
pub struct TextAnalysis {
    pub arabic_count: usize,
    pub english_count: usize,
    pub digit_count: usize,
    pub punct_count: usize,
    pub other_count: usize,
    pub arabic_ratio: f64,
    pub english_ratio: f64,
    pub has_digits: bool,
    pub has_punctuation: bool,
}

impl TextAnalysis {
    pub fn analyze(text: &str) -> Self {
        let mut arabic_count = 0;
        let mut english_count = 0;
        let mut digit_count = 0;
        let mut punct_count = 0;
        let mut other_count = 0;

        for ch in text.chars() {
            if is_arabic_char(ch) {
                arabic_count += 1;
            } else if is_english_letter(ch) {
                english_count += 1;
            } else if ch.is_ascii_digit() || ch.is_numeric() {
                digit_count += 1;
            } else if ch.is_ascii_punctuation() {
                punct_count += 1;
            } else {
                other_count += 1;
            }
        }

        let total = text.chars().count();
        let arabic_ratio = if total > 0 { arabic_count as f64 / total as f64 } else { 0.0 };
        let english_ratio = if total > 0 { english_count as f64 / total as f64 } else { 0.0 };

        TextAnalysis {
            arabic_count,
            english_count,
            digit_count,
            punct_count,
            other_count,
            arabic_ratio,
            english_ratio,
            has_digits: digit_count > 0,
            has_punctuation: punct_count > 0,
        }
    }

    pub fn is_likely_url(&self, text: &str) -> bool {
        text.starts_with("http://") || text.starts_with("https://") || 
        text.starts_with("www.") || (text.contains('.') && text.contains('/'))
    }

    pub fn is_likely_email(&self, text: &str) -> bool {
        text.contains('@') && text.contains('.')
    }

    pub fn is_likely_code(&self, text: &str) -> bool {
        text.contains("{}[]();=")
    }
}

/// Check if a word is valid (exists in dictionary or passes heuristics)
pub fn is_valid_word(word: &str) -> bool {
    if word.len() < 2 {
        return false;
    }

    let word_lower = word.to_lowercase();
    
    // Check English dictionary
    if COMMON_ENGLISH_WORDS.contains(&word_lower.as_str()) {
        return true;
    }

    // Check Arabic dictionary
    if COMMON_ARABIC_WORDS.contains(&word) {
        return true;
    }

    // Simple heuristic: check if word has reasonable character distribution
    let analysis = TextAnalysis::analyze(word);
    if analysis.arabic_ratio > 0.6 || analysis.english_ratio > 0.6 {
        return true;
    }

    false
}

/// Detect if typed text is wrong layout and return corrected version
pub fn detect_and_correct(typed_word: &str) -> Option<(String, String)> {
    if typed_word.len() < 2 {
        return None;
    }

    // Check if it's already valid
    if is_valid_word(typed_word) {
        return None;
    }

    // Analyze the word
    let analysis = TextAnalysis::analyze(typed_word);

    // Try Arabic->English conversion (user was on Arabic layout, meant English)
    if analysis.arabic_count > 0 {
        let converted_en = crate::keyboard::arabic_layout_to_english(typed_word);
        if is_valid_word(&converted_en) {
            return Some((typed_word.to_string(), converted_en));
        }
    }

    // Try English->Arabic conversion (user was on English layout, meant Arabic)
    if analysis.english_count > 0 {
        let converted_ar = crate::keyboard::english_layout_to_arabic(typed_word);
        if is_valid_word(&converted_ar) {
            return Some((typed_word.to_string(), converted_ar));
        }
    }

    None
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_text_analysis_arabic() {
        let analysis = TextAnalysis::analyze("مرحبا");
        assert!(analysis.arabic_ratio > 0.9);
        assert_eq!(analysis.arabic_count, 5);
    }

    #[test]
    fn test_text_analysis_english() {
        let analysis = TextAnalysis::analyze("hello");
        assert!(analysis.english_ratio > 0.9);
        assert_eq!(analysis.english_count, 5);
    }

    #[test]
    fn test_text_analysis_mixed() {
        let analysis = TextAnalysis::analyze("hello مرحبا");
        assert!(analysis.arabic_count > 0);
        assert!(analysis.english_count > 0);
    }

    #[test]
    fn test_is_valid_word_common() {
        assert!(is_valid_word("hello"));
        assert!(is_valid_word("the"));
        assert!(is_valid_word("مرحبا"));
        assert!(is_valid_word("في"));
    }

    #[test]
    fn test_detect_and_correct_hello() {
        // "اثممخ" is "hello" typed with Arabic layout
        let result = detect_and_correct("اثممخ");
        assert_eq!(result, Some(("اثممخ".to_string(), "hello".to_string())));
    }

    #[test]
    fn test_detect_and_correct_already_valid() {
        // "hello" is already valid
        let result = detect_and_correct("hello");
        assert_eq!(result, None);
    }
}
