//! Text analysis module
//! 
//! This module provides text analysis capabilities including:
//! - Language/script detection
//! - Character classification
//! - URL/email/code detection

use crate::keyboard::KeyboardMapper;

/// Analysis result for a piece of text
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
    /// Analyze text and return characteristics
    pub fn analyze(text: &str) -> Self {
        let mut arabic_count = 0;
        let mut english_count = 0;
        let mut digit_count = 0;
        let mut punct_count = 0;
        let mut other_count = 0;

        for ch in text.chars() {
            if KeyboardMapper::is_arabic_char(ch) {
                arabic_count += 1;
            } else if KeyboardMapper::is_english_letter(ch) {
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

    /// Check if text is likely a URL
    pub fn is_likely_url(&self, text: &str) -> bool {
        text.starts_with("http://") 
            || text.starts_with("https://") 
            || text.starts_with("www.")
            || (text.contains('.') && text.contains('/'))
    }

    /// Check if text is likely an email address
    pub fn is_likely_email(&self, text: &str) -> bool {
        text.contains('@') && text.contains('.')
    }

    /// Check if text is likely code/syntax
    pub fn is_likely_code(&self, text: &str) -> bool {
        text.contains("{}[]();=")
    }

    /// Get the dominant script
    pub fn dominant_script(&self) -> &'static str {
        if self.arabic_count > self.english_count {
            "arabic"
        } else if self.english_count > self.arabic_count {
            "english"
        } else {
            "unknown"
        }
    }
}

/// Text analyzer - stateless wrapper around TextAnalysis
#[derive(Debug, Default, Clone)]
pub struct TextAnalyzer;

impl TextAnalyzer {
    pub fn new() -> Self {
        Self
    }

    pub fn analyze(&self, text: &str) -> TextAnalysis {
        TextAnalysis::analyze(text)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_analyze_arabic() {
        let analysis = TextAnalysis::analyze("مرحبا");
        assert!(analysis.arabic_ratio > 0.9);
        assert_eq!(analysis.arabic_count, 5);
    }

    #[test]
    fn test_analyze_english() {
        let analysis = TextAnalysis::analyze("hello");
        assert!(analysis.english_ratio > 0.9);
        assert_eq!(analysis.english_count, 5);
    }

    #[test]
    fn test_analyze_mixed() {
        let analysis = TextAnalysis::analyze("hello مرحبا");
        assert!(analysis.arabic_count > 0);
        assert!(analysis.english_count > 0);
    }

    #[test]
    fn test_is_likely_url() {
        let analysis = TextAnalysis::analyze("");
        assert!(analysis.is_likely_url("https://example.com"));
        assert!(analysis.is_likely_url("www.example.com"));
    }

    #[test]
    fn test_is_likely_email() {
        let analysis = TextAnalysis::analyze("");
        assert!(analysis.is_likely_email("test@example.com"));
    }

    #[test]
    fn test_is_likely_code() {
        let analysis = TextAnalysis::analyze("");
        assert!(analysis.is_likely_code("function() {}"));
    }
}
