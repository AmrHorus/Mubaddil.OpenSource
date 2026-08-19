//! Scoring module for confidence calculation
//! 
//! This module provides confidence scoring for correction candidates.

use crate::keyboard::LayoutId;

/// Configuration for scoring
#[derive(Debug, Clone)]
pub struct ScoringConfig {
    /// Base weight for dictionary match
    pub dictionary_weight: f64,
    /// Weight for character distribution match
    pub distribution_weight: f64,
    /// Weight for layout mapping quality
    pub mapping_weight: f64,
    /// Minimum characters needed for high confidence
    pub min_chars_high_conf: usize,
}

impl Default for ScoringConfig {
    fn default() -> Self {
        Self {
            dictionary_weight: 0.5,
            distribution_weight: 0.3,
            mapping_weight: 0.2,
            min_chars_high_conf: 4,
        }
    }
}

/// Confidence score (0.0 - 1.0)
#[derive(Debug, Clone)]
pub struct ConfidenceScore {
    pub value: f64,
    pub dictionary_score: f64,
    pub distribution_score: f64,
    pub mapping_score: f64,
}

impl ConfidenceScore {
    pub fn new(value: f64) -> Self {
        Self {
            value: value.clamp(0.0, 1.0),
            dictionary_score: 0.0,
            distribution_score: 0.0,
            mapping_score: 0.0,
        }
    }

    pub fn with_components(value: f64, dict: f64, dist: f64, map: f64) -> Self {
        Self {
            value: value.clamp(0.0, 1.0),
            dictionary_score: dict,
            distribution_score: dist,
            mapping_score: map,
        }
    }

    pub fn is_high(&self) -> bool {
        self.value >= 0.9
    }

    pub fn is_medium(&self) -> bool {
        self.value >= 0.7 && self.value < 0.9
    }

    pub fn is_low(&self) -> bool {
        self.value < 0.7
    }
}

/// A candidate correction with its score
#[derive(Debug, Clone)]
pub struct ScoredCandidate {
    /// Original text
    pub original: String,
    /// Corrected text
    pub corrected: String,
    /// Source layout
    pub source_layout: LayoutId,
    /// Target layout
    pub target_layout: LayoutId,
    /// Confidence score
    pub confidence: f64,
    /// Reason for suggestion
    pub reason: String,
}

impl ScoredCandidate {
    pub fn new(
        original: String,
        corrected: String,
        source: LayoutId,
        target: LayoutId,
        confidence: f64,
        reason: String,
    ) -> Self {
        Self {
            original,
            corrected,
            source_layout: source,
            target_layout: target,
            confidence,
            reason,
        }
    }
}

/// Scoring engine
#[derive(Debug, Default, Clone)]
pub struct ScoringEngine {
    config: ScoringConfig,
}

impl ScoringEngine {
    pub fn new() -> Self {
        Self::with_config(ScoringConfig::default())
    }

    pub fn with_config(config: ScoringConfig) -> Self {
        Self { config }
    }

    /// Calculate confidence score for a candidate
    pub fn calculate(
        &self,
        original: &str,
        corrected: &str,
        _source: LayoutId,
        _target: LayoutId,
    ) -> ConfidenceScore {
        // Dictionary score: check if corrected word is in common words
        let dict_score = self.score_dictionary_match(corrected);

        // Distribution score: check character distribution
        let dist_score = self.score_distribution(original, corrected);

        // Mapping score: check how many characters were actually mapped
        let map_score = self.score_mapping_quality(original, corrected);

        // Weighted average
        let value = dict_score * self.config.dictionary_weight
            + dist_score * self.config.distribution_weight
            + map_score * self.config.mapping_weight;

        ConfidenceScore::with_components(value, dict_score, dist_score, map_score)
    }

    fn score_dictionary_match(&self, word: &str) -> f64 {
        let word_lower = word.to_lowercase();
        
        // Check against common English words
        const COMMON_EN: &[&str] = &[
            "the", "be", "to", "of", "and", "a", "in", "that", "have", "it",
            "for", "not", "on", "with", "he", "as", "you", "do", "at", "this",
            "but", "his", "by", "from", "they", "we", "say", "her", "she", "or",
            "hello", "world", "test", "example", "keyboard", "layout",
        ];

        // Check against common Arabic words
        const COMMON_AR: &[&str] = &[
            "في", "من", "على", "إلى", "عن", "أن", "إن", "كان", "قد", "لا",
            "ما", "مع", "هو", "هي", "نحن", "أنا", "أنت", "هم",
            "مرحبا", "شكرا", "سلام", "صباح", "مساء",
        ];

        if COMMON_EN.contains(&word_lower.as_str()) || COMMON_AR.contains(&word) {
            return 1.0;
        }

        // Partial credit for reasonable-looking words
        if word.len() >= 3 && word.chars().all(|c| c.is_alphabetic()) {
            return 0.6;
        }

        0.3
    }

    fn score_distribution(&self, _original: &str, corrected: &str) -> f64 {
        // Good distribution: consistent script in corrected text
        let arabic_count = corrected.chars()
            .filter(|c| crate::keyboard::KeyboardMapper::is_arabic_char(*c))
            .count();
        let english_count = corrected.chars()
            .filter(|c| crate::keyboard::KeyboardMapper::is_english_letter(*c))
            .count();
        let total = corrected.chars().count();

        if total == 0 {
            return 0.0;
        }

        let ratio = (arabic_count.max(english_count) as f64) / (total as f64);
        
        // Higher score if corrected text has consistent script
        if ratio > 0.8 {
            1.0
        } else if ratio > 0.6 {
            0.7
        } else {
            0.4
        }
    }

    fn score_mapping_quality(&self, original: &str, corrected: &str) -> f64 {
        // Check how many characters actually changed
        let orig_len = original.chars().count();
        let corr_len = corrected.chars().count();

        if orig_len != corr_len {
            return 0.5; // Length change reduces confidence
        }

        let changed = original.chars()
            .zip(corrected.chars())
            .filter(|(a, b)| a != b)
            .count();

        if changed == 0 {
            return 0.3; // No change means no correction happened
        }

        // Good mapping: most characters changed (indicates successful transformation)
        let change_ratio = changed as f64 / orig_len as f64;
        if change_ratio > 0.8 {
            1.0
        } else if change_ratio > 0.5 {
            0.7
        } else {
            0.4
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_confidence_score_clamping() {
        let score = ConfidenceScore::new(1.5);
        assert!(score.value <= 1.0);
        
        let score = ConfidenceScore::new(-0.5);
        assert!(score.value >= 0.0);
    }

    #[test]
    fn test_scoring_engine() {
        let engine = ScoringEngine::new();
        let score = engine.calculate(
            "اثممخ",
            "hello",
            LayoutId::ArabicSA,
            LayoutId::EnglishUS,
        );
        assert!(score.value > 0.0);
    }
}
