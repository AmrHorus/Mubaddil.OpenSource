//! Keyboard module for Mubaddil Core

pub mod mapping;

pub use mapping::{
    arabic_layout_to_english,
    english_layout_to_arabic,
    is_arabic_char,
    is_english_letter,
    ARABIC_TO_ENGLISH,
    ENGLISH_TO_ARABIC,
};
