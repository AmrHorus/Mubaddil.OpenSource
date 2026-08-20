//! Keyboard layout mapping module
//!
//! This module handles keyboard layout definitions and transformations
//! between different layouts (e.g., English US ↔ Arabic Saudi).

use crate::error::{MubaddilError, MubaddilResult};
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

/// Identifier for a keyboard layout
#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize, Default)]
pub enum LayoutId {
    EnglishUS,
    ArabicSA,
    #[default]
    Unknown,
    Custom(String),
}

impl std::fmt::Display for LayoutId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            LayoutId::EnglishUS => write!(f, "en_us"),
            LayoutId::ArabicSA => write!(f, "ar_sa"),
            LayoutId::Unknown => write!(f, "unknown"),
            LayoutId::Custom(s) => write!(f, "{}", s),
        }
    }
}

/// A single keyboard layout definition
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct KeyboardLayout {
    pub name: String,
    pub code: String,
    pub normal: HashMap<String, String>,
    #[serde(default)]
    pub shift: HashMap<String, String>,
}

/// Bidirectional keyboard mapper
#[derive(Debug)]
pub struct KeyboardMapper {
    /// Layout definitions
    layouts: HashMap<String, KeyboardLayout>,
    /// Direct character mappings (from -> to)
    mappings: HashMap<(LayoutId, LayoutId), HashMap<char, char>>,
}

impl Default for KeyboardMapper {
    fn default() -> Self {
        let mut mapper = Self {
            layouts: HashMap::new(),
            mappings: HashMap::new(),
        };
        mapper.load_default_mappings();
        mapper
    }
}

impl KeyboardMapper {
    /// Create a new empty KeyboardMapper
    pub fn new() -> Self {
        Self::default()
    }

    /// Load default English-Arabic mappings
    fn load_default_mappings(&mut self) {
        // Arabic to English mapping (Saudi layout 101)
        let ar_to_en: Vec<(char, char)> = vec![
            // Numbers
            ('٠', '0'),
            ('١', '1'),
            ('٢', '2'),
            ('٣', '3'),
            ('٤', '4'),
            ('٥', '5'),
            ('٦', '6'),
            ('٧', '7'),
            ('٨', '8'),
            ('٩', '9'),
            // Row 2 (QWERTY top)
            ('ض', 'q'),
            ('ص', 'w'),
            ('ث', 'e'),
            ('ق', 'r'),
            ('ف', 't'),
            ('غ', 'y'),
            ('ع', 'u'),
            ('ه', 'i'),
            ('خ', 'o'),
            ('ح', 'p'),
            ('ج', '['),
            ('د', ']'),
            // Row 3 (ASDFG middle)
            ('ش', 'a'),
            ('س', 's'),
            ('ي', 'd'),
            ('ب', 'f'),
            ('ل', 'g'),
            ('ا', 'h'),
            ('ت', 'j'),
            ('ن', 'k'),
            ('م', 'l'),
            ('ك', ';'),
            ('ط', '\''),
            // Row 4 (ZXCVB bottom)
            ('ئ', 'z'),
            ('ء', 'x'),
            ('ؤ', 'c'),
            ('ر', 'v'),
            ('ى', 'n'),
            ('ة', 'm'),
            ('و', ','),
            ('ز', '.'),
            ('ظ', '/'),
            // Additional
            ('ذ', '`'),
            ('ّ', '~'),
        ];

        // English to Arabic mapping (reverse)
        let en_to_ar: Vec<(char, char)> = vec![
            // Numbers
            ('0', '٠'),
            ('1', '١'),
            ('2', '٢'),
            ('3', '٣'),
            ('4', '٤'),
            ('5', '٥'),
            ('6', '٦'),
            ('7', '٧'),
            ('8', '٨'),
            ('9', '٩'),
            // Row 2
            ('q', 'ض'),
            ('w', 'ص'),
            ('e', 'ث'),
            ('r', 'ق'),
            ('t', 'ف'),
            ('y', 'غ'),
            ('u', 'ع'),
            ('i', 'ه'),
            ('o', 'خ'),
            ('p', 'ح'),
            ('[', 'ج'),
            (']', 'د'),
            // Row 3
            ('a', 'ش'),
            ('s', 'س'),
            ('d', 'ي'),
            ('f', 'ب'),
            ('g', 'ل'),
            ('h', 'ا'),
            ('j', 'ت'),
            ('k', 'ن'),
            ('l', 'م'),
            (';', 'ك'),
            ('\'', 'ط'),
            // Row 4
            ('z', 'ئ'),
            ('x', 'ء'),
            ('c', 'ؤ'),
            ('v', 'ر'),
            ('n', 'ى'),
            ('m', 'ة'),
            (',', 'و'),
            ('.', 'ز'),
            ('/', 'ظ'),
            // Additional
            ('`', 'ذ'),
            ('~', 'ّ'),
        ];

        self.mappings.insert(
            (LayoutId::ArabicSA, LayoutId::EnglishUS),
            ar_to_en.into_iter().collect(),
        );
        self.mappings.insert(
            (LayoutId::EnglishUS, LayoutId::ArabicSA),
            en_to_ar.into_iter().collect(),
        );
    }

    /// Map text from one layout to another
    pub fn map(&self, text: &str, from: LayoutId, to: LayoutId) -> String {
        if let Some(mapping) = self.mappings.get(&(from.clone(), to.clone())) {
            text.chars()
                .map(|c| *mapping.get(&c).unwrap_or(&c))
                .collect()
        } else {
            text.to_string()
        }
    }

    /// Convert Arabic layout text to English
    pub fn arabic_to_english(&self, text: &str) -> String {
        self.map(text, LayoutId::ArabicSA, LayoutId::EnglishUS)
    }

    /// Convert English layout text to Arabic
    pub fn english_to_arabic(&self, text: &str) -> String {
        self.map(text, LayoutId::EnglishUS, LayoutId::ArabicSA)
    }

    /// Check if a character is Arabic
    pub fn is_arabic_char(c: char) -> bool {
        matches!(c as u32,
            0x0600..=0x06FF |     // Arabic
            0xFB50..=0xFDFF |     // Arabic Presentation Forms-A
            0xFE70..=0xFEFF       // Arabic Presentation Forms-B
        )
    }

    /// Check if a character is an English letter
    pub fn is_english_letter(c: char) -> bool {
        c.is_ascii_alphabetic()
    }

    /// Load layouts from JSON data
    pub fn load_layouts_from_json(&mut self, json_data: &str) -> MubaddilResult<()> {
        #[derive(Deserialize)]
        struct LayoutData {
            layouts: Option<HashMap<String, KeyboardLayout>>,
            #[allow(dead_code)]
            mappings: Option<HashMap<String, HashMap<String, String>>>,
        }

        let data: LayoutData = serde_json::from_str(json_data).map_err(|e| {
            MubaddilError::InvalidInput(format!("Failed to parse layout JSON: {}", e))
        })?;

        if let Some(layouts) = data.layouts {
            for (key, layout) in layouts {
                self.layouts.insert(key, layout);
            }
        }

        Ok(())
    }

    /// Get available layout IDs
    pub fn available_layouts(&self) -> Vec<LayoutId> {
        let mut layouts = vec![LayoutId::EnglishUS, LayoutId::ArabicSA];
        for key in self.layouts.keys() {
            layouts.push(LayoutId::Custom(key.clone()));
        }
        layouts
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_arabic_to_english_hello() {
        let mapper = KeyboardMapper::default();
        // "اثممخ" should map to "hello"
        assert_eq!(mapper.arabic_to_english("اثممخ"), "hello");
    }

    #[test]
    fn test_english_to_arabic() {
        let mapper = KeyboardMapper::default();
        let result = mapper.english_to_arabic("hello");
        assert!(!result.is_empty());
        assert!(result.chars().all(KeyboardMapper::is_arabic_char));
    }

    #[test]
    fn test_is_arabic_char() {
        assert!(KeyboardMapper::is_arabic_char('ع'));
        assert!(KeyboardMapper::is_arabic_char('ر'));
        assert!(!KeyboardMapper::is_arabic_char('a'));
    }

    #[test]
    fn test_is_english_letter() {
        assert!(KeyboardMapper::is_english_letter('a'));
        assert!(KeyboardMapper::is_english_letter('Z'));
        assert!(!KeyboardMapper::is_english_letter('ع'));
    }

    #[test]
    fn test_mapping_preserves_unknown_chars() {
        let mapper = KeyboardMapper::default();
        assert_eq!(mapper.arabic_to_english("hello123"), "hello123");
    }
}
