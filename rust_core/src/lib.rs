//! Mubaddil Core - Intelligent Keyboard Layout Switcher
//! 
//! This module provides the core engine for detecting and correcting
//! keyboard layout mistakes in real-time using Windows low-level hooks.
//!
//! # Architecture
//! 
//! The Rust core handles:
//! - Windows low-level keyboard hook (WH_KEYBOARD_LL)
//! - Keyboard input processing
//! - Arabic/English keyboard mapping
//! - Text injection via SendInput
//! - Thread-safe state management
//!
//! Python handles:
//! - UI (PySide6)
//! - Configuration management
//! - High-level correction logic
//! - Language detection heuristics
//! - Application lifecycle

use pyo3::prelude::*;
use pyo3::exceptions::{PyRuntimeError, PyValueError};
use std::sync::atomic::{AtomicBool, AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, Instant};
use thiserror::Error;

// Windows API types and constants
use windows_sys::Win32::Foundation::{HWND, LPARAM, LRESULT, WPARAM, BOOL};
use windows_sys::Win32::UI::Input::KeyboardAndMouse::{
    GetKeyboardLayout, SendInput, INPUT, INPUT_KEYBOARD, KEYBDINPUT,
    KEYEVENTF_KEYUP, KEYEVENTF_UNICODE, VK_BACK, VK_SPACE,
};
use windows_sys::Win32::UI::WindowsAndMessaging::{
    CallNextHookEx, SetWindowsHookExW, UnhookWindowsHookEx, WH_KEYBOARD_LL,
    HHOOK, KBDLLHOOKSTRUCT, WM_KEYDOWN, WM_KEYUP, WM_SYSKEYDOWN, WM_SYSKEYUP,
};
use windows_sys::Win32::Globalization::GetForegroundWindow;

/// Error types for the Mubaddil Core
#[derive(Error, Debug)]
pub enum MubaddilError {
    #[error("Failed to install keyboard hook: {0}")]
    HookInstallationFailed(String),
    #[error("Failed to remove keyboard hook: {0}")]
    HookRemovalFailed(String),
    #[error("Engine is already running")]
    EngineAlreadyRunning,
    #[error("Engine is not running")]
    EngineNotRunning,
    #[error("Thread synchronization error: {0}")]
    SyncError(String),
    #[error("Invalid input: {0}")]
    InvalidInput(String),
    #[error("Windows API error: {0}")]
    WindowsApiError(String),
}

impl From<MubaddilError> for PyErr {
    fn from(err: MubaddilError) -> PyErr {
        match err {
            MubaddilError::EngineAlreadyRunning => PyRuntimeError::new_err(err.to_string()),
            MubaddilError::EngineNotRunning => PyRuntimeError::new_err(err.to_string()),
            MubaddilError::InvalidInput(_) => PyValueError::new_err(err.to_string()),
            _ => PyRuntimeError::new_err(err.to_string()),
        }
    }
}

/// Result type alias for Mubaddil operations
type MubaddilResult<T> = Result<T, MubaddilError>;

// ============================================================================
// Keyboard Mapping Tables
// ============================================================================

/// Arabic to English keyboard mapping (Saudi Arabic layout 101)
/// When user types on Arabic layout but meant English
const ARABIC_TO_ENGLISH: &[(char, char)] = &[
    // Row 1 (numbers)
    ('٠', '0'), ('١', '1'), ('٢', '2'), ('٣', '3'), ('٤', '4'),
    ('٥', '5'), ('٦', '6'), ('٧', '7'), ('٨', '8'), ('٩', '9'),
    // Row 2 (QWERTY top)
    ('ض', 'q'), ('ص', 'w'), ('ث', 'e'), ('ق', 'r'), ('ف', 't'),
    ('غ', 'y'), ('ع', 'u'), ('ه', 'i'), ('خ', 'o'), ('ح', 'p'),
    ('ج', '['), ('د', ']'),
    // Row 3 (ASDFG middle)
    ('ش', 'a'), ('س', 's'), ('ي', 'd'), ('ب', 'f'), ('ل', 'g'),
    ('ا', 'h'), ('ت', 'j'), ('ن', 'k'), ('م', 'l'), ('ك', ';'),
    ('ط', '\''),
    // Row 4 (ZXCVB bottom)
    ('ئ', 'z'), ('ء', 'x'), ('ؤ', 'c'), ('ر', 'v'), ('لا', 'b'),
    ('ى', 'n'), ('ة', 'm'), ('و', ','), ('ز', '.'), ('ظ', '/'),
    // Additional characters
    ('ذ', '`'), ('ّ', '~'), ('َ', 'q'), ('ً', 'w'), ('ُ', 'e'),
    ('ٌ', 'r'), ('ل', 't'), ('إ', 'y'), ('\'', 'u'), ('÷', 'i'),
    ('×', 'o'), ('؛', 'p'), ('<', '['), ('>', ']'), ('ِ', 'a'),
    ('ٍ', 's'), (']', 'd'), ('[', 'f'), ('ل', 'g'), ('أ', 'h'),
    ('ـ', 'j'), ('،', 'k'), ('/', 'l'), (':', ';'), ('"', '\''),
    ('~', 'z'), ('ْ', 'x'), ('}', 'c'), ('{', 'v'), ('آ', 'b'),
    ('\'', 'n'), ('?', '/'),
];

/// English to Arabic keyboard mapping (reverse)
const ENGLISH_TO_ARABIC: &[(char, char)] = &[
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

/// Common English words for validation
const COMMON_ENGLISH_WORDS: &[&str] = &[
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
    "before", "great", "again", "off", "long", "great", "little", "own",
    "after", "while", "around", "same", "game", "different", "important",
    "such", "here", "place", "high", "show", "house", "point", "group",
    "another", "begin", "start", "through", "question", "number", "part",
    "child", "eye", "woman", "system", "program", "hand", "large", "small",
    "end", "problem", "read", "include", "public", "follow", "stand",
    "probably", "build", "nation", "country", "company", "side", "fact",
];

/// Common Arabic words for validation
const COMMON_ARABIC_WORDS: &[&str] = &[
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

// ============================================================================
// Utility Functions
// ============================================================================

/// Convert text typed with Arabic layout to English
fn arabic_layout_to_english(text: &str) -> String {
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

/// Convert text typed with English layout to Arabic
fn english_layout_to_arabic(text: &str) -> String {
    text.chars()
        .map(|c| {
            ENGLISH_TO_ARABIC
                .iter()
                .find(|(english, _)| english.to_ascii_lowercase() == c.to_ascii_lowercase())
                .map(|(_, arabic)| *arabic)
                .unwrap_or(c)
        })
        .collect()
}

/// Check if a character is an Arabic letter
fn is_arabic_char(c: char) -> bool {
    matches!(c as u32,
        0x0600..=0x06FF |     // Arabic
        0xFB50..=0xFDFF |     // Arabic Presentation Forms-A
        0xFE70..=0xFEFF       // Arabic Presentation Forms-B
    )
}

/// Check if a character is an English letter
fn is_english_letter(c: char) -> bool {
    c.is_ascii_alphabetic()
}

/// Check if a string is likely a valid word
fn is_valid_word(word: &str) -> bool {
    if word.len() < 2 {
        return false;
    }

    let word_lower = word.to_lowercase();
    
    // Check English dictionary
    if COMMON_ENGLISH_WORDS.contains(&word_lower.as_str()) {
        return true;
    }

    // Check Arabic dictionary
    if COMMON_ARABIC_WORDS.contains(&word.as_ref()) {
        return true;
    }

    // Simple heuristic: check if word has reasonable character distribution
    if word.chars().all(|c| is_arabic_char(c) || is_english_letter(c) || c.is_ascii_digit()) {
        return word.len() >= 2;
    }

    false
}

/// Detect if typed text is wrong layout and return corrected version
fn detect_and_correct(typed_word: &str) -> Option<(String, String)> {
    if typed_word.len() < 2 {
        return None;
    }

    // Check if it's already valid
    if is_valid_word(typed_word) {
        return None;
    }

    // Analyze the word
    let arabic_count = typed_word.chars().filter(|c| is_arabic_char(*c)).count();
    let english_count = typed_word.chars().filter(|c| is_english_letter(*c)).count();
    let total = typed_word.chars().count();

    if total == 0 {
        return None;
    }

    // Try Arabic->English conversion (user was on Arabic layout, meant English)
    if arabic_count > 0 {
        let converted_en = arabic_layout_to_english(typed_word);
        if is_valid_word(&converted_en) {
            return Some((typed_word.to_string(), converted_en));
        }
    }

    // Try English->Arabic conversion (user was on English layout, meant Arabic)
    if english_count > 0 {
        let converted_ar = english_layout_to_arabic(typed_word);
        if is_valid_word(&converted_ar) {
            return Some((typed_word.to_string(), converted_ar));
        }
    }

    None
}

// ============================================================================
// Input Injection
// ============================================================================

/// Replace text by simulating backspaces and re-typing
fn replace_text(original: &str, corrected: &str) -> MubaddilResult<()> {
    unsafe {
        let original_len = original.chars().count();
        
        // Send backspaces
        for _ in 0..original_len {
            let mut input = INPUT {
                r#type: INPUT_KEYBOARD,
                Anonymous: std::mem::zeroed(),
            };
            input.Anonymous.ki.wVk = VK_BACK as u16;
            input.Anonymous.ki.dwExtraInfo = 0;
            
            let sent = SendInput(1, &mut input, std::mem::size_of::<INPUT>() as u32);
            if sent == 0 {
                return Err(MubaddilError::WindowsApiError("Failed to send backspace".to_string()));
            }
        }

        // Small delay between backspaces and typing
        thread::sleep(Duration::from_millis(10));

        // Send corrected characters using Unicode input
        for c in corrected.chars() {
            // Key down
            let mut input_down = INPUT {
                r#type: INPUT_KEYBOARD,
                Anonymous: std::mem::zeroed(),
            };
            input_down.Anonymous.ki.wScan = c as u16;
            input_down.Anonymous.ki.dwFlags = KEYEVENTF_UNICODE;
            input_down.Anonymous.ki.dwExtraInfo = 0;
            
            let sent = SendInput(1, &mut input_down, std::mem::size_of::<INPUT>() as u32);
            if sent == 0 {
                return Err(MubaddilError::WindowsApiError("Failed to send key down".to_string()));
            }

            // Key up
            let mut input_up = INPUT {
                r#type: INPUT_KEYBOARD,
                Anonymous: std::mem::zeroed(),
            };
            input_up.Anonymous.ki.wScan = c as u16;
            input_up.Anonymous.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
            input_up.Anonymous.ki.dwExtraInfo = 0;
            
            let sent = SendInput(1, &mut input_up, std::mem::size_of::<INPUT>() as u32);
            if sent == 0 {
                return Err(MubaddilError::WindowsApiError("Failed to send key up".to_string()));
            }

            // Tiny delay between characters for reliability
            thread::sleep(Duration::from_millis(3));
        }
    }
    
    Ok(())
}

// ============================================================================
// Engine State Management
// ============================================================================

/// Internal state of the engine
#[derive(Debug)]
struct EngineState {
    /// Current buffered word
    buffer: String,
    /// Last key press time
    last_key_time: Instant,
    /// Is the engine actively processing
    active: bool,
    /// Target window handle for corrections
    target_hwnd: u64,
}

impl EngineState {
    fn new() -> Self {
        Self {
            buffer: String::with_capacity(64),
            last_key_time: Instant::now(),
            active: true,
            target_hwnd: 0,
        }
    }

    fn reset_buffer(&mut self) {
        self.buffer.clear();
    }

    fn add_char(&mut self, c: char) {
        if self.buffer.len() < 100 {
            self.buffer.push(c);
        }
    }

    fn remove_last_char(&mut self) {
        self.buffer.pop();
    }

    fn get_buffer(&self) -> String {
        self.buffer.clone()
    }
}

/// Shared engine data protected by mutex
struct SharedEngineData {
    state: Mutex<EngineState>,
    running: AtomicBool,
    hook_handle: Mutex<*mut HHOOK>,
    event_counter: AtomicU64,
}

unsafe impl Send for SharedEngineData {}
unsafe impl Sync for SharedEngineData {}

impl SharedEngineData {
    fn new() -> Self {
        Self {
            state: Mutex::new(EngineState::new()),
            running: AtomicBool::new(false),
            hook_handle: Mutex::new(std::ptr::null_mut()),
            event_counter: AtomicU64::new(0),
        }
    }
}

/// Global reference to the engine data for the hook callback
static mut ENGINE_DATA: Option<Arc<SharedEngineData>> = None;

// ============================================================================
// Keyboard Hook Implementation
// ============================================================================

/// Low-level keyboard hook callback
/// 
/// # Safety
/// This function is called by Windows and must follow the hook callback convention.
/// It accesses global static data which is safe because:
/// 1. The data is Arc'd and properly synchronized
/// 2. We only access it when n_code >= 0
/// 3. We use proper locking for mutable access
unsafe extern "system" fn keyboard_hook_callback(
    n_code: i32,
    w_param: WPARAM,
    l_param: LPARAM,
) -> LRESULT {
    const HC_ACTION: i32 = 0;

    if n_code != HC_ACTION {
        return match ENGINE_DATA.as_ref() {
            Some(data) => {
                let hook = data.hook_handle.lock().unwrap();
                CallNextHookEx(*hook, n_code, w_param, l_param)
            }
            None => 0,
        };
    }

    let engine_data = match ENGINE_DATA.as_ref() {
        Some(data) => data,
        None => return 0,
    };

    if !engine_data.running.load(Ordering::Relaxed) {
        let hook = engine_data.hook_handle.lock().unwrap();
        return CallNextHookEx(*hook, n_code, w_param, l_param);
    }

    let is_key_down = w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN;
    let is_key_up = w_param == WM_KEYUP || w_param == WM_SYSKEYUP;

    if is_key_down {
        let kbd_struct = *(l_param as *const KBDLLHOOKSTRUCT);
        let vk_code = kbd_struct.vkCode;
        let scan_code = kbd_struct.scanCode;
        
        // Capture foreground window for later correction
        let hwnd = GetForegroundWindow();
        
        let mut state_guard = match engine_data.state.lock() {
            Ok(guard) => guard,
            Err(_) => {
                let hook = engine_data.hook_handle.lock().unwrap();
                return CallNextHookEx(*hook, n_code, w_param, l_param);
            }
        };

        state_guard.target_hwnd = hwnd as u64;

        // Handle special keys
        match vk_code {
            // Backspace
            8 => {
                state_guard.remove_last_char();
                state_guard.last_key_time = Instant::now();
            }
            // Space, Enter, Tab - word separators
            32 | 13 | 9 => {
                // Process the buffered word
                if !state_guard.buffer.is_empty() {
                    let word_to_check = state_guard.get_buffer();
                    let target = state_guard.target_hwnd;
                    drop(state_guard);
                    
                    // Attempt correction
                    if let Some((original, corrected)) = detect_and_correct(&word_to_check) {
                        // Store correction request
                        let counter = engine_data.event_counter.fetch_add(1, Ordering::Relaxed);
                        
                        // For now, we just process directly
                        // In a full implementation, this would queue for Python processing
                        let _ = replace_text(&original, &corrected);
                    }
                    
                    state_guard = engine_data.state.lock().unwrap();
                    state_guard.reset_buffer();
                    state_guard.target_hwnd = target;
                }
                state_guard.last_key_time = Instant::now();
            }
            // Escape
            27 => {
                state_guard.reset_buffer();
                state_guard.last_key_time = Instant::now();
            }
            // Regular character keys
            _ => {
                // Try to get the character from the virtual key code
                // This is a simplified approach - a full implementation would use ToUnicodeEx
                if vk_code >= 65 && vk_code <= 90 {
                    // A-Z keys
                    let c = ((vk_code - 65) as u8 + b'A') as char;
                    state_guard.add_char(c);
                } else if vk_code >= 48 && vk_code <= 57 {
                    // 0-9 keys
                    let c = ((vk_code - 48) as u8 + b'0') as char;
                    state_guard.add_char(c);
                }
                // Add more key ranges as needed
                
                state_guard.last_key_time = Instant::now();
            }
        }
    }

    let hook = engine_data.hook_handle.lock().unwrap();
    CallNextHookEx(*hook, n_code, w_param, l_param)
}

/// Install the low-level keyboard hook
fn install_hook(engine_data: Arc<SharedEngineData>) -> MubaddilResult<()> {
    unsafe {
        let hook_proc = Some(keyboard_hook_callback as unsafe extern "system" fn(i32, WPARAM, LPARAM) -> LRESULT);
        
        let hook = SetWindowsHookExW(WH_KEYBOARD_LL, hook_proc, std::ptr::null_mut(), 0);
        
        if hook.is_null() {
            return Err(MubaddilError::HookInstallationFailed(
                "SetWindowsHookExW returned null".to_string()
            ));
        }

        *engine_data.hook_handle.lock().unwrap() = hook;
        
        // Store global reference for the callback
        ENGINE_DATA = Some(Arc::clone(&engine_data));
    }
    
    Ok(())
}

/// Uninstall the keyboard hook
fn uninstall_hook(engine_data: &SharedEngineData) {
    unsafe {
        let hook_handle = engine_data.hook_handle.lock().unwrap();
        if !hook_handle.is_null() {
            UnhookWindowsHookEx(*hook_handle);
        }
        ENGINE_DATA = None;
    }
}

/// Background thread function that runs the message pump
fn hook_thread_main(engine_data: Arc<SharedEngineData>) {
    // Install the hook
    if let Err(e) = install_hook(Arc::clone(&engine_data)) {
        eprintln!("Failed to install hook: {}", e);
        return;
    }

    engine_data.running.store(true, Ordering::Relaxed);

    // Message pump - required for hooks to work on Windows
    // Using a simple loop with sleep for cross-platform compatibility
    while engine_data.running.load(Ordering::Relaxed) {
        thread::sleep(Duration::from_millis(10));
        
        // In a pure Windows implementation, you would use:
        // let mut msg = std::mem::zeroed();
        // if PeekMessageW(&mut msg, std::ptr::null_mut(), 0, 0, PM_REMOVE) != 0 {
        //     TranslateMessage(&msg);
        //     DispatchMessageW(&msg);
        // }
    }

    // Uninstall hook when stopping
    uninstall_hook(&engine_data);
}

// ============================================================================
// Python Bindings
// ============================================================================

/// Python-exposed MubaddilCore class
#[pyclass]
pub struct MubaddilCore {
    engine_data: Arc<SharedEngineData>,
    thread_handle: Mutex<Option<thread::JoinHandle<()>>>,
}

#[pymethods]
impl MubaddilCore {
    /// Create a new MubaddilCore instance
    #[new]
    fn new() -> Self {
        Self {
            engine_data: Arc::new(SharedEngineData::new()),
            thread_handle: Mutex::new(None),
        }
    }

    /// Start the keyboard hook engine
    fn start(&self) -> PyResult<()> {
        let mut thread_guard = self.thread_handle.lock().map_err(|e| {
            PyRuntimeError::new_err(format!("Failed to lock thread handle: {}", e))
        })?;

        if thread_guard.is_some() {
            return Err(PyRuntimeError::new_err("Engine is already running"));
        }

        let engine_data_clone = Arc::clone(&self.engine_data);
        let handle = thread::spawn(move || {
            hook_thread_main(engine_data_clone);
        });

        *thread_guard = Some(handle);
        
        Ok(())
    }

    /// Stop the keyboard hook engine
    fn stop(&self) -> PyResult<()> {
        // Signal the thread to stop
        self.engine_data.running.store(false, Ordering::Relaxed);

        // Wait for the thread to finish
        let mut thread_guard = self.thread_handle.lock().map_err(|e| {
            PyRuntimeError::new_err(format!("Failed to lock thread handle: {}", e))
        })?;

        if let Some(handle) = thread_guard.take() {
            let _ = handle.join();
        }

        Ok(())
    }

    /// Check if the engine is currently running
    fn is_running(&self) -> bool {
        self.engine_data.running.load(Ordering::Relaxed)
    }

    /// Get the current buffer content (for debugging)
    fn get_buffer(&self) -> PyResult<String> {
        let state = self.engine_data.state.lock().map_err(|e| {
            PyRuntimeError::new_err(format!("Failed to lock state: {}", e))
        })?;
        Ok(state.get_buffer())
    }

    /// Manually trigger correction on a given text
    fn correct_text(&self, text: &str) -> PyResult<Option<String>> {
        match detect_and_correct(text) {
            Some((_, corrected)) => Ok(Some(corrected)),
            None => Ok(None),
        }
    }

    /// Convert Arabic layout text to English
    #[staticmethod]
    fn arabic_to_english(text: &str) -> String {
        arabic_layout_to_english(text)
    }

    /// Convert English layout text to Arabic
    #[staticmethod]
    fn english_to_arabic(text: &str) -> String {
        english_layout_to_arabic(text)
    }

    /// Check if a character is Arabic
    #[staticmethod]
    fn is_arabic_char(c: char) -> bool {
        is_arabic_char(c)
    }

    /// Check if a character is English
    #[staticmethod]
    fn is_english_letter(c: char) -> bool {
        is_english_letter(c)
    }

    /// Get the version of the core library
    #[staticmethod]
    fn version() -> &'static str {
        env!("CARGO_PKG_VERSION")
    }
}

/// Python module definition
#[pymodule]
fn mubaddil_core(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_class::<MubaddilCore>()?;
    Ok(())
}

// ============================================================================
// Tests
// ============================================================================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_arabic_to_english_mapping_hello() {
        // "اثممخ" should map to "hello"
        let result = arabic_layout_to_english("اثممخ");
        assert_eq!(result, "hello");
    }

    #[test]
    fn test_arabic_to_english_mapping_world() {
        // "قورلد" should map to "world"
        let result = arabic_layout_to_english("قورلد");
        assert_eq!(result, "world");
    }

    #[test]
    fn test_english_to_arabic_mapping() {
        // "hello" typed on Arabic layout would produce Arabic chars
        let result = english_layout_to_arabic("hello");
        assert!(!result.is_empty());
    }

    #[test]
    fn test_detect_and_correct_hello() {
        // "اثممخ" is "hello" typed with Arabic layout
        let result = detect_and_correct("اثممخ");
        assert_eq!(result, Some(("اثممخ".to_string(), "hello".to_string())));
    }

    #[test]
    fn test_is_valid_word() {
        assert!(is_valid_word("hello"));
        assert!(is_valid_word("the"));
        assert!(is_valid_word("مرحبا"));
        assert!(is_valid_word("في"));
        assert!(!is_valid_word("xyzabc"));
        assert!(!is_valid_word("ab")); // Too short
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
}
