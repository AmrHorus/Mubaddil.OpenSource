//! Keyboard mapping for Arabic/English layouts
//! 
//! This module provides bidirectional keyboard mapping between
//! Saudi Arabic (101) and US English keyboard layouts.

/// Arabic to English keyboard mapping (Saudi Arabic layout 101)
/// When user types on Arabic layout but meant English
pub const ARABIC_TO_ENGLISH: &[(char, char)] = &[
    // Row 1 (numbers with Eastern Arabic numerals)
    ('٠', '0'), ('١', '1'), ('٢', '2'), ('٣', '3'), ('٤', '4'),
    ('٥', '5'), ('٦', '6'), ('٧', '7'), ('٨', '8'), ('٩', '9'),
    // Row 2 (QWERTY top - Arabic letters)
    ('ض', 'q'), ('ص', 'w'), ('ث', 'e'), ('ق', 'r'), ('ف', 't'),
    ('غ', 'y'), ('ع', 'u'), ('ه', 'i'), ('خ', 'o'), ('ح', 'p'),
    ('ج', '['), ('د', ']'),
    // Row 3 (ASDFG middle - Arabic letters)
    ('ش', 'a'), ('س', 's'), ('ي', 'd'), ('ب', 'f'), ('ل', 'g'),
    ('ا', 'h'), ('ت', 'j'), ('ن', 'k'), ('م', 'l'), ('ك', ';'),
    ('ط', '\''),
    // Row 4 (ZXCVB bottom - Arabic letters)
    ('ئ', 'z'), ('ء', 'x'), ('ؤ', 'c'), ('ر', 'v'), ('لا', 'b'),
    ('ى', 'n'), ('ة', 'm'), ('و', ','), ('ز', '.'), ('ظ', '/'),
    // Additional characters (shift states and diacritics)
    ('ذ', '`'), ('ّ', '~'),
    // Farsi/Arabic extended digits
];

/// English to Arabic keyboard mapping (reverse)
pub const ENGLISH_TO_ARABIC: &[(char, char)] = &[
    // Row 1 (numbers)
    ('0', '٠'), ('1', '١'), ('2', '٢'), ('3', '٣'), ('4', '٤'),
    ('5', '٥'), ('6', '٦'), ('7', '٧'), ('8', '٨'), ('9', '٩'),
    // Row 2 (QWERTY top)
    ('q', 'ض'), ('w', 'ص'), ('e', 'ث'), ('r', 'ق'), ('t', 'ف'),
    ('y', 'غ'), ('u', 'ع'), ('i', 'ه'), ('o', 'خ'), ('p', 'ح'),
    ('[', 'ج'), (']', 'د'),
    // Row 3 (ASDFG middle)
    ('a', 'ش'), ('s', 'س'), ('d', 'ي'), ('f', 'ب'), ('g', 'ل'),
    ('h', 'ا'), ('j', 'ت'), ('k', 'ن'), ('l', 'م'), (';', 'ك'),
    ('\'', 'ط'),
    // Row 4 (ZXCVB bottom)
    ('z', 'ئ'), ('x', 'ء'), ('c', 'ؤ'), ('v', 'ر'), ('b', 'لا'),
    ('n', 'ى'), ('m', 'ة'), (',', 'و'), ('.', 'ز'), ('/', 'ظ'),
    // Additional
    ('`', 'ذ'), ('~', 'ّ'),
];

/// Shift state mappings for Arabic keyboard
pub const ARABIC_SHIFT_TO_ENGLISH: &[(char, char)] = &[
    ('َ', 'q'), ('ً', 'w'), ('ُ', 'e'), ('ٌ', 'r'),
    ('ل', 't'), ('إ', 'y'), ('\'', 'u'), ('÷', 'i'),
    ('×', 'o'), ('؛', 'p'), ('<', '['), ('>', ']'),
    ('ِ', 'a'), ('ٍ', 's'), (']', 'd'), ('[', 'f'),
    ('أ', 'h'), ('ـ', 'j'), ('،', 'k'), ('/', 'l'),
    (':', ';'), ('"', '\''), ('~', 'z'), ('ْ', 'x'),
    ('}', 'c'), ('{', 'v'), ('آ', 'b'), ('?', '/'),
];

/// Convert text typed with Arabic layout to English equivalent
pub fn arabic_layout_to_english(text: &str) -> String {
    text.chars()
        .map(|c| {
            ARABIC_TO_ENGLISH
                .iter()
                .find(|(arabic, _)| *arabic == c)
                .map(|(_, english)| *english)
                .unwrap_or(c)
        })
        .collect()
}

/// Convert text typed with English layout to Arabic equivalent
pub fn english_layout_to_arabic(text: &str) -> String {
    text.chars()
        .map(|c| {
            // Case-insensitive matching for English letters
            let lower_c = c.to_ascii_lowercase();
            ENGLISH_TO_ARABIC
                .iter()
                .find(|(english, _)| english.to_ascii_lowercase() == lower_c)
                .map(|(_, arabic)| *arabic)
                .unwrap_or(c)
        })
        .collect()
}

/// Check if a character is an Arabic letter
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_arabic_to_english_hello() {
        // "اثممخ" should map to "hello"
        assert_eq!(arabic_layout_to_english("اثممخ"), "hello");
    }

    #[test]
    fn test_arabic_to_english_world() {
        // "قورلد" should map to "world"
        assert_eq!(arabic_layout_to_english("قورلد"), "world");
    }

    #[test]
    fn test_english_to_arabic() {
        let result = english_layout_to_arabic("hello");
        assert!(!result.is_empty());
        assert!(result.chars().all(is_arabic_char));
    }

    #[test]
    fn test_is_arabic_char() {
        assert!(is_arabic_char('ع'));
        assert!(is_arabic_char('ر'));
        assert!(is_arabic_char('ب'));
        assert!(!is_arabic_char('a'));
        assert!(!is_arabic_char('1'));
    }

    #[test]
    fn test_is_english_letter() {
        assert!(is_english_letter('a'));
        assert!(is_english_letter('Z'));
        assert!(!is_english_letter('ع'));
        assert!(!is_english_letter('1'));
    }

    #[test]
    fn test_mapping_preserves_unknown_chars() {
        assert_eq!(arabic_layout_to_english("hello123"), "hello123");
        assert_eq!(english_layout_to_arabic("مرحبا١٢٣"), "مرحبا١٢٣");
    }
}
