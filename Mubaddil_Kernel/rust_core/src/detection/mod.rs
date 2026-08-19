//! Detection engine for keyboard layout mismatch detection
//! 
//! This module orchestrates the detection pipeline:
//! 1. Analyze input text
//! 2. Generate candidate corrections
//! 3. Score candidates
//! 4. Return best suggestion

use std::collections::HashSet;
use serde::{Deserialize, Serialize};
use crate::keyboard::{KeyboardMapper, LayoutId};
use crate::text::TextAnalysis;
use crate::scoring::{ScoringEngine, ScoredCandidate};
use crate::error::{MubaddilError, MubaddilResult};

/// Detection result
#[derive(Debug, Clone)]
pub struct DetectionResult {
    pub has_suggestion: bool,
    pub original: String,
    pub suggestion: Option<String>,
    pub confidence: f64,
    pub source_layout: LayoutId,
    pub target_layout: LayoutId,
}

/// Dictionary data structure
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DictionaryData {
    pub en: LanguageDictionary,
    pub ar: LanguageDictionary,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LanguageDictionary {
    #[serde(default)]
    pub common_words: Vec<String>,
    #[serde(default)]
    pub names: Vec<String>,
}

/// Detection engine
pub struct DetectionEngine {
    config: crate::KernelConfig,
    mapper: KeyboardMapper,
    scorer: ScoringEngine,
    english_dict: HashSet<String>,
    arabic_dict: HashSet<String>,
}

impl DetectionEngine {
    pub fn new(config: crate::KernelConfig) -> Self {
        let mut engine = Self {
            config,
            mapper: KeyboardMapper::default(),
            scorer: ScoringEngine::default(),
            english_dict: HashSet::new(),
            arabic_dict: HashSet::new(),
        };
        engine.load_default_dictionaries();
        engine
    }

    fn load_default_dictionaries(&mut self) {
        // Common English words
        let en_words = [
            "the", "be", "to", "of", "and", "a", "in", "that", "have", "it",
            "for", "not", "on", "with", "he", "as", "you", "do", "at", "this",
            "but", "his", "by", "from", "they", "we", "say", "her", "she", "or",
            "hello", "world", "test", "example", "keyboard", "layout", "switch",
            "google", "github", "microsoft", "openai",
        ];
        for word in en_words {
            self.english_dict.insert(word.to_lowercase());
        }

        // Common Arabic words
        let ar_words = [
            "في", "من", "على", "إلى", "عن", "أن", "إن", "كان", "قد", "لا",
            "ما", "مع", "هو", "هي", "نحن", "أنا", "أنت", "هم",
            "مرحبا", "شكرا", "سلام", "صباح", "مساء", "كتاب", "بيت",
        ];
        for word in ar_words {
            self.arabic_dict.insert(word.to_string());
        }
    }

    /// Load dictionary from JSON data
    pub fn load_dictionary(&mut self, json_data: &str) -> MubaddilResult<()> {
        let dict: DictionaryData = serde_json::from_str(json_data)
            .map_err(|e| MubaddilError::InvalidInput(format!("Failed to parse dictionary JSON: {}", e)))?;

        for word in dict.en.common_words {
            self.english_dict.insert(word.to_lowercase());
        }
        for word in dict.en.names {
            self.english_dict.insert(word.to_lowercase());
        }
        for word in dict.ar.common_words {
            self.arabic_dict.insert(word);
        }
        for word in dict.ar.names {
            self.arabic_dict.insert(word);
        }

        Ok(())
    }

    /// Generate correction candidates for text
    pub fn generate_candidates(&self, text: &str, analysis: &TextAnalysis) -> Vec<ScoredCandidate> {
        let mut candidates = Vec::new();

        // Skip if text is too short or has digits/punctuation
        if text.len() < self.config.min_word_length || analysis.has_digits || analysis.has_punctuation {
            return candidates;
        }

        // Check if already valid (in dictionary)
        if self.is_valid_word(text) {
            return candidates;
        }

        // Try Arabic -> English conversion
        if analysis.arabic_count > 0 {
            let converted = self.mapper.arabic_to_english(text);
            if converted != text && self.is_valid_word(&converted) {
                let score = self.scorer.calculate(
                    text,
                    &converted,
                    LayoutId::ArabicSA,
                    LayoutId::EnglishUS,
                );
                
                if score.value > 0.3 {
                    candidates.push(ScoredCandidate::new(
                        text.to_string(),
                        converted,
                        LayoutId::ArabicSA,
                        LayoutId::EnglishUS,
                        score.value,
                        "keyboard_layout_mismatch".to_string(),
                    ));
                }
            }
        }

        // Try English -> Arabic conversion
        if analysis.english_count > 0 {
            let converted = self.mapper.english_to_arabic(text);
            if converted != text && self.is_valid_word(&converted) {
                let score = self.scorer.calculate(
                    text,
                    &converted,
                    LayoutId::EnglishUS,
                    LayoutId::ArabicSA,
                );
                
                if score.value > 0.3 {
                    candidates.push(ScoredCandidate::new(
                        text.to_string(),
                        converted,
                        LayoutId::EnglishUS,
                        LayoutId::ArabicSA,
                        score.value,
                        "keyboard_layout_mismatch".to_string(),
                    ));
                }
            }
        }

        // Sort by confidence descending
        candidates.sort_by(|a, b| b.confidence.partial_cmp(&a.confidence).unwrap_or(std::cmp::Ordering::Equal));
        
        candidates
    }

    /// Score a candidate correction
    pub fn score_candidate(
        &self,
        original: &str,
        corrected: &str,
        source: LayoutId,
        target: LayoutId,
    ) -> crate::scoring::ConfidenceScore {
        self.scorer.calculate(original, corrected, source, target)
    }

    /// Check if a word is valid (exists in dictionary or passes heuristics)
    fn is_valid_word(&self, word: &str) -> bool {
        if word.len() < 2 {
            return false;
        }

        let word_lower = word.to_lowercase();

        // Check dictionaries
        if self.english_dict.contains(&word_lower) {
            return true;
        }
        if self.arabic_dict.contains(word) {
            return true;
        }

        // Heuristic: reasonable character distribution
        let arabic_count = word.chars()
            .filter(|c| KeyboardMapper::is_arabic_char(*c))
            .count();
        let english_count = word.chars()
            .filter(|c| KeyboardMapper::is_english_letter(*c))
            .count();
        let total = word.chars().count();

        if total > 0 {
            let ratio = (arabic_count.max(english_count) as f64) / (total as f64);
            if ratio > 0.6 {
                return true;
            }
        }

        false
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::KernelConfig;

    #[test]
    fn test_detection_engine_creation() {
        let config = KernelConfig::default();
        let engine = DetectionEngine::new(config);
        assert!(!engine.english_dict.is_empty());
        assert!(!engine.arabic_dict.is_empty());
    }

    #[test]
    fn test_generate_candidates_arabic_to_english() {
        let config = KernelConfig::default();
        let engine = DetectionEngine::new(config);
        let analysis = TextAnalysis::analyze("اثممخ");
        let candidates = engine.generate_candidates("اثممخ", &analysis);
        
        assert!(!candidates.is_empty());
        assert_eq!(candidates[0].corrected, "hello");
    }

    #[test]
    fn test_generate_candidates_valid_word() {
        let config = KernelConfig::default();
        let engine = DetectionEngine::new(config);
        let analysis = TextAnalysis::analyze("hello");
        let candidates = engine.generate_candidates("hello", &analysis);
        
        assert!(candidates.is_empty()); // Already valid
    }
}
