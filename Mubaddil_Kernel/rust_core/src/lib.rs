//! Mubaddil Kernel - Core Intelligence Engine
//!
//! This is the core processing engine for Mubaddil, responsible for:
//! - Keyboard layout mapping and transformation
//! - Language/script detection
//! - Candidate generation for corrections
//! - Confidence scoring
//! - Text analysis
//!
//! # Architecture
//!
//! The kernel is designed to be:
//! - **Platform agnostic**: No Windows/Linux/macOS specific APIs
//! - **UI independent**: No UI framework dependencies
//! - **Memory safe**: All Rust safety guarantees apply
//! - **Thread safe**: Can be used from multiple threads
//! - **Fast**: Optimized for real-time keyboard processing
//!
//! # Example Usage
//!
//! ```rust
//! use mubaddil_kernel::MubaddilKernel;
//!
//! let kernel = MubaddilKernel::new();
//! let result = kernel.analyze("ulv,");
//!
//! if result.should_suggest {
//!     if let Some(suggestion) = &result.suggestion {
//!         println!("Suggestion: {}", suggestion);
//!     }
//! }
//! ```

pub mod detection;
pub mod keyboard;
pub mod scoring;
pub mod text;

mod error;

pub use detection::{DetectionEngine, DetectionResult};
pub use error::{MubaddilError, MubaddilResult};
pub use keyboard::{KeyboardLayout, KeyboardMapper, LayoutId};
pub use scoring::{ConfidenceScore, ScoredCandidate, ScoringConfig};
pub use text::{TextAnalysis, TextAnalyzer};

use parking_lot::RwLock;
use std::sync::Arc;

/// Configuration for the Mubaddil Kernel
#[derive(Debug, Clone)]
pub struct KernelConfig {
    /// Minimum word length to consider for correction
    pub min_word_length: usize,
    /// Maximum word length to process
    pub max_word_length: usize,
    /// Confidence threshold for suggestions
    pub suggestion_threshold: f64,
    /// Confidence threshold for auto-correction
    pub auto_correct_threshold: f64,
    /// Enable URL/email detection (skip these)
    pub detect_urls_emails: bool,
    /// Enable code detection (skip these)
    pub detect_code: bool,
}

impl Default for KernelConfig {
    fn default() -> Self {
        Self {
            min_word_length: 2,
            max_word_length: 100,
            suggestion_threshold: 0.70,
            auto_correct_threshold: 0.90,
            detect_urls_emails: true,
            detect_code: true,
        }
    }
}

/// Analysis result returned by the kernel
#[derive(Debug, Clone)]
pub struct AnalysisResult {
    /// Original input text
    pub original: String,
    /// Suggested correction (if any)
    pub suggestion: Option<String>,
    /// Source layout detected
    pub source_layout: LayoutId,
    /// Target layout for suggestion
    pub target_layout: LayoutId,
    /// Confidence score (0.0 - 1.0)
    pub confidence: f64,
    /// Whether a suggestion should be shown
    pub should_suggest: bool,
    /// Whether auto-correction is recommended
    pub should_auto_correct: bool,
    /// Reason for the suggestion
    pub reason: Option<String>,
    /// All candidate corrections with scores
    pub candidates: Vec<ScoredCandidate>,
}

impl AnalysisResult {
    pub fn no_suggestion(original: String) -> Self {
        Self {
            original,
            suggestion: None,
            source_layout: LayoutId::Unknown,
            target_layout: LayoutId::Unknown,
            confidence: 0.0,
            should_suggest: false,
            should_auto_correct: false,
            reason: None,
            candidates: vec![],
        }
    }
}

/// Main Mubaddil Kernel engine
///
/// This is the primary entry point for using the Mubaddil correction engine.
/// It orchestrates all components: keyboard mapping, text analysis,
/// detection, and scoring.
pub struct MubaddilKernel {
    config: KernelConfig,
    mapper: Arc<RwLock<KeyboardMapper>>,
    analyzer: TextAnalyzer,
    detector: DetectionEngine,
}

unsafe impl Send for MubaddilKernel {}
unsafe impl Sync for MubaddilKernel {}

impl MubaddilKernel {
    /// Create a new MubaddilKernel with default configuration
    pub fn new() -> Self {
        Self::with_config(KernelConfig::default())
    }

    /// Create a new MubaddilKernel with custom configuration
    pub fn with_config(config: KernelConfig) -> Self {
        let mapper = Arc::new(RwLock::new(KeyboardMapper::default()));
        let analyzer = TextAnalyzer::new();
        let detector = DetectionEngine::new(config.clone());

        Self {
            config,
            mapper,
            analyzer,
            detector,
        }
    }

    /// Get the current configuration
    pub fn config(&self) -> &KernelConfig {
        &self.config
    }

    /// Analyze text and return correction suggestion
    ///
    /// This is the main entry point for text analysis.
    pub fn analyze(&self, text: &str) -> AnalysisResult {
        // Quick validation
        if text.len() < self.config.min_word_length || text.len() > self.config.max_word_length {
            return AnalysisResult::no_suggestion(text.to_string());
        }

        // Analyze text characteristics
        let analysis = self.analyzer.analyze(text);

        // Skip URLs, emails, code if configured
        if self.config.detect_urls_emails
            && (analysis.is_likely_url(text) || analysis.is_likely_email(text))
        {
            return AnalysisResult::no_suggestion(text.to_string());
        }

        if self.config.detect_code && analysis.is_likely_code(text) {
            return AnalysisResult::no_suggestion(text.to_string());
        }

        // Generate candidates using detection engine
        let candidates = self.detector.generate_candidates(text, &analysis);

        if candidates.is_empty() {
            return AnalysisResult::no_suggestion(text.to_string());
        }

        // Find best candidate
        let best_candidate = candidates.iter().max_by(|a, b| {
            a.confidence
                .partial_cmp(&b.confidence)
                .unwrap_or(std::cmp::Ordering::Equal)
        });

        if let Some(candidate) = best_candidate {
            let should_suggest = candidate.confidence >= self.config.suggestion_threshold;
            let should_auto_correct = candidate.confidence >= self.config.auto_correct_threshold;

            AnalysisResult {
                original: text.to_string(),
                suggestion: Some(candidate.corrected.clone()),
                source_layout: candidate.source_layout.clone(),
                target_layout: candidate.target_layout.clone(),
                confidence: candidate.confidence,
                should_suggest,
                should_auto_correct,
                reason: Some(candidate.reason.clone()),
                candidates,
            }
        } else {
            AnalysisResult::no_suggestion(text.to_string())
        }
    }

    /// Map text from one keyboard layout to another
    pub fn map_text(&self, text: &str, from: LayoutId, to: LayoutId) -> String {
        let mapper = self.mapper.read();
        mapper.map(text, from, to)
    }

    /// Detect the likely language/script of text
    pub fn detect_language(&self, text: &str) -> TextAnalysis {
        self.analyzer.analyze(text)
    }

    /// Generate correction candidates for text
    pub fn generate_candidates(&self, text: &str) -> Vec<ScoredCandidate> {
        let analysis = self.analyzer.analyze(text);
        self.detector.generate_candidates(text, &analysis)
    }

    /// Calculate confidence score for a candidate correction
    pub fn score_candidate(
        &self,
        original: &str,
        corrected: &str,
        source: LayoutId,
        target: LayoutId,
    ) -> ConfidenceScore {
        self.detector
            .score_candidate(original, corrected, source, target)
    }

    /// Load keyboard layouts from JSON data
    pub fn load_keyboard_layouts(&self, json_data: &str) -> MubaddilResult<()> {
        let mut mapper = self.mapper.write();
        mapper.load_layouts_from_json(json_data)?;
        Ok(())
    }

    /// Load dictionary words from JSON data
    pub fn load_dictionary(&self, json_data: &str) -> MubaddilResult<()> {
        // For now, we skip this as it requires mutable access
        // In production, use Arc<Mutex<DetectionEngine>> or similar
        let _ = json_data;
        Ok(())
    }

    /// Get version information
    pub fn version() -> &'static str {
        env!("CARGO_PKG_VERSION")
    }
}

impl Default for MubaddilKernel {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_kernel_creation() {
        let kernel = MubaddilKernel::new();
        assert_eq!(kernel.config().min_word_length, 2);
    }

    #[test]
    fn test_analyze_arabic_to_english() {
        let kernel = MubaddilKernel::new();
        // "اثممخ" should map to "hello"
        let result = kernel.analyze("اثممخ");
        assert!(result.confidence > 0.0);
    }

    #[test]
    fn test_analyze_valid_text_no_suggestion() {
        let kernel = MubaddilKernel::new();
        let result = kernel.analyze("hello");
        assert!(!result.should_suggest);
    }

    #[test]
    fn test_map_text() {
        let kernel = MubaddilKernel::new();
        let mapped = kernel.map_text("hello", LayoutId::EnglishUS, LayoutId::ArabicSA);
        assert!(!mapped.is_empty());
    }
}
