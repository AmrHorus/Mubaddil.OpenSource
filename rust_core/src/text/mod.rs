//! Text module for Mubaddil Core

pub mod analysis;

pub use analysis::{
    detect_and_correct,
    is_valid_word,
    TextAnalysis,
    COMMON_ENGLISH_WORDS,
    COMMON_ARABIC_WORDS,
};
