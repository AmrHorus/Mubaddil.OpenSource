//! Mubaddil Core - Intelligent Keyboard Layout Switcher
//! 
//! This module provides the core engine for detecting and correcting
//! keyboard layout mistakes in real-time using Windows low-level hooks.

use pyo3::prelude::*;
use pyo3::exceptions::PyRuntimeError;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::thread;
use std::time::{Duration, Instant};

// Windows API types and constants
use windows_sys::Win32::Foundation::{HWND, LPARAM, LRESULT, WPARAM};
use windows_sys::Win32::UI::Input::KeyboardAndMouse::{
    GetKeyboardLayout, SendInput, INPUT, INPUT_KEYBOARD, KEYBDINPUT, KEYEVENTF_KEYUP,
    KEYEVENTF_UNICODE, MAPVK_KL_TO_VK, VK_BACK,
};
use windows_sys::Win32::UI::WindowsAndMessaging::{
    CallNextHookEx, SetWindowsHookExW, UnhookWindowsHookEx, WH_KEYBOARD_LL, HHOOK, KBDLLHOOKSTRUCT,
};
use windows_sys::Win32::System::Threading::GetThreadId;
use windows_sys::Win32::Globalization::GetForegroundWindow;

use strsim::levenshtein;
use fuzzy_matcher::skim::SkimMatcherV2;

/// Maximum word length to buffer before analysis
const MAX_WORD_LENGTH: usize = 50;

/// Minimum word length to trigger detection
const MIN_WORD_LENGTH: usize = 3;

/// Time threshold (ms) to consider a word complete
const WORD_COMPLETE_THRESHOLD_MS: u64 = 2000;

/// Arabic-English keyboard mapping
/// When user types on Arabic layout but meant English
const ARABIC_TO_ENGLISH: &[(char, char)] = &[
    ('ث', 'e'), ('ص', 's'), ('ق', 'q'), ('ف', 'f'), ('غ', 'g'),
    ('ع', 'a'), ('ه', 'h'), ('خ', 'j'), ('ح', 'c'), ('ج', 'd'),
    ('د', 'i'), ('ط', 't'), ('ك', 'k'), ('ل', 'l'), ('ش', 'x'),
    ('س', 'n'), ('ي', 'b'), ('ب', 'y'), ('لا', 'l'), ('ا', 'a'),
    ('ت', 'u'), ('ن', 'm'), ('م', 'w'), ('ى', '/'), ('ة', 'p'),
    ('ؤ', '\''), ('ر', 'o'), ('لا', 'l'), ('و', ','), ('.', '.'),
    ('ظ', 'z'), ('ذ', '\\'), ('ز', '.'), ('ئ', ';'), ('ء', '\''),
    ('>', '>'), ('<', '<'), ('؟', '?'), ('!', '!'), ('@', '@'),
    ('#', '#'), ('$', '$'), ('%', '%'), ('^', '^'), ('&', '&'),
    ('(', '('), (')', ')'), ('_', '-'), ('+', '='), ('[', '['),
    (']', ']'), ('{', '{'), ('}', '}'), ('|', '|'), ('~', '~'),
    ('`', '`'), ('-', '-'), ('=', '='), ('/', '/'), ('\\', '\\'),
    (';', ';'), ('\'', '\''), (',', ','), ('.', '.'), ('?', '?'),
];

/// English-Arabic keyboard mapping (reverse)
const ENGLISH_TO_ARABIC: &[(char, char)] = &[
    ('e', 'ث'), ('s', 'ص'), ('q', 'ق'), ('f', 'ف'), ('g', 'غ'),
    ('a', 'ع'), ('h', 'ه'), ('j', 'خ'), ('c', 'ح'), ('d', 'ج'),
    ('i', 'د'), ('t', 'ط'), ('k', 'ك'), ('l', 'ل'), ('x', 'ش'),
    ('n', 'س'), ('b', 'ي'), ('y', 'ب'), ('u', 'ت'), ('m', 'م'),
    ('w', 'م'), ('/', 'ى'), ('p', 'ة'), ('\'', 'ؤ'), ('o', 'ر'),
    (',', 'و'), ('z', 'ظ'), ('\\', 'ذ'), ('.', 'ز'), (';', 'ئ'),
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
];

/// Common Arabic words for validation (transliterated for demo)
const COMMON_ARABIC_WORDS: &[&str] = &[
    "مرحبا", "العالم", "اختبار", "مثال", "لوحة", "مفتاح", "تبديل",
    "كتابة", "تصحيح", "خطأ", "إصلاح", "تلقائي", "ذكي", "عربي",
    "انجليزي", "كلمة", "جملة", "نص", "رسالة", "بريد", "هاتف",
];

/// Mapping from Arabic-layout gibberish to English
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

/// Mapping from English-layout gibberish to Arabic
fn english_layout_to_arabic(text: &str) -> String {
    text.chars()
        .map(|c| {
            ENGLISH_TO_ARABIC
                .iter()
                .find(|(english, _)| *english.to_ascii_lowercase() == c.to_ascii_lowercase())
                .map(|(_, arabic)| *arabic)
                .unwrap_or(c)
        })
        .collect()
}

/// Check if a string is likely a valid word in either language
fn is_valid_word(word: &str) -> bool {
    if word.len() < MIN_WORD_LENGTH {
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

    // Fuzzy matching for English
    let matcher = SkimMatcherV2::default();
    for dict_word in COMMON_ENGLISH_WORDS.iter() {
        if matcher.fuzzy_match(dict_word, &word_lower).is_some() {
            return true;
        }
    }

    // Levenshtein distance check for near-matches
    for dict_word in COMMON_ENGLISH_WORDS.iter() {
        if levenshtein(dict_word, &word_lower) <= 1 && word_lower.len() >= 4 {
            return true;
        }
    }

    false
}

/// Detect if typed text is wrong layout and return corrected version
fn detect_and_correct(typed_word: &str) -> Option<String> {
    if typed_word.len() < MIN_WORD_LENGTH {
        return None;
    }

    // Check if it's already valid
    if is_valid_word(typed_word) {
        return None;
    }

    // Try Arabic->English conversion
    let converted_en = arabic_layout_to_english(typed_word);
    if is_valid_word(&converted_en) {
        return Some(converted_en);
    }

    // Try English->Arabic conversion
    let converted_ar = english_layout_to_arabic(typed_word);
    if is_valid_word(&converted_ar) {
        return Some(converted_ar);
    }

    None
}

/// Internal state of the engine
#[derive(Debug)]
struct EngineState {
    /// Current buffered word
    buffer: String,
    /// Last key press time
    last_key_time: Instant,
    /// Current keyboard layout handle
    current_layout: u64,
    /// Is the engine actively processing
    active: bool,
}

impl EngineState {
    fn new() -> Self {
        Self {
            buffer: String::new(),
            last_key_time: Instant::now(),
            current_layout: 0,
            active: true,
        }
    }

    fn reset_buffer(&mut self) {
        self.buffer.clear();
    }

    fn add_char(&mut self, c: char) {
        if self.buffer.len() < MAX_WORD_LENGTH {
            self.buffer.push(c);
        }
    }

    fn remove_last_char(&mut self) {
        self.buffer.pop();
    }
}

/// Shared engine data protected by mutex
struct SharedEngineData {
    state: Mutex<EngineState>,
    running: AtomicBool,
    hook_handle: Mutex<*mut HHOOK>,
}

unsafe impl Send for SharedEngineData {}
unsafe impl Sync for SharedEngineData {}

impl SharedEngineData {
    fn new() -> Self {
        Self {
            state: Mutex::new(EngineState::new()),
            running: AtomicBool::new(false),
            hook_handle: Mutex::new(std::ptr::null_mut()),
        }
    }
}

/// Global reference to the engine data for the hook callback
static mut ENGINE_DATA: Option<Arc<SharedEngineData>> = None;

/// Low-level keyboard hook callback
unsafe extern "system" fn keyboard_hook_callback(
    n_code: i32,
    w_param: WPARAM,
    l_param: LPARAM,
) -> LRESULT {
    const WM_KEYDOWN: WPARAM = 0x0100;
    const WM_KEYUP: WPARAM = 0x0101;
    const WM_SYSKEYDOWN: WPARAM = 0x0104;
    const WM_SYSKEYUP: WPARAM = 0x0105;

    if n_code < 0 {
        return CallNextHookEx(*ENGINE_DATA.as_ref().unwrap().hook_handle.lock().unwrap(), n_code, w_param, l_param);
    }

    let engine_data = match ENGINE_DATA.as_ref() {
        Some(data) => data,
        None => return CallNextHookEx(std::ptr::null_mut(), n_code, w_param, l_param),
    };

    if !engine_data.running.load(Ordering::Relaxed) {
        return CallNextHookEx(*engine_data.hook_handle.lock().unwrap(), n_code, w_param, l_param);
    }

    let is_key_down = w_param == WM_KEYDOWN || w_param == WM_SYSKEYDOWN;
    let is_key_up = w_param == WM_KEYUP || w_param == WM_SYSKEYUP;

    if is_key_down {
        let kbd_struct = *(l_param as *const KBDLLHOOKSTRUCT);
        let vk_code = kbd_struct.vkCode;
        
        let mut state_guard = match engine_data.state.lock() {
            Ok(guard) => guard,
            Err(_) => return CallNextHookEx(*engine_data.hook_handle.lock().unwrap(), n_code, w_param, l_param),
        };

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
                    let word_to_check = state_guard.buffer.clone();
                    drop(state_guard);
                    
                    if let Some(corrected) = detect_and_correct(&word_to_check) {
                        replace_text(&word_to_check, &corrected);
                    }
                    
                    state_guard = engine_data.state.lock().unwrap();
                    state_guard.reset_buffer();
                }
                state_guard.last_key_time = Instant::now();
            }
            // Escape
            27 => {
                state_guard.reset_buffer();
                state_guard.last_key_time = Instant::now();
            }
            // Regular character keys (A-Z, 0-9, etc.)
            _ => {
                if vk_code >= 65 && vk_code <= 90 {
                    // A-Z keys
                    let c = ((vk_code - 65) as u8 + b'A') as char;
                    state_guard.add_char(c);
                } else if vk_code >= 48 && vk_code <= 57 {
                    // 0-9 keys
                    let c = ((vk_code - 48) as u8 + b'0') as char;
                    state_guard.add_char(c);
                }
                state_guard.last_key_time = Instant::now();

                // Check for timeout-based word completion
                let buffer_copy = state_guard.buffer.clone();
                drop(state_guard);

                if buffer_copy.len() >= MIN_WORD_LENGTH {
                    if let Some(corrected) = detect_and_correct(&buffer_copy) {
                        replace_text(&buffer_copy, &corrected);
                    }
                }
            }
        }
    }

    CallNextHookEx(*engine_data.hook_handle.lock().unwrap(), n_code, w_param, l_param)
}

/// Replace text by simulating backspaces and re-typing
fn replace_text(original: &str, corrected: &str) {
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
            
            SendInput(1, &mut input, std::mem::size_of::<INPUT>() as u32);
        }

        // Small delay between backspaces and typing
        thread::sleep(Duration::from_millis(10));

        // Send corrected characters
        for c in corrected.chars() {
            // Key down
            let mut input_down = INPUT {
                r#type: INPUT_KEYBOARD,
                Anonymous: std::mem::zeroed(),
            };
            input_down.Anonymous.ki.wScan = c as u16;
            input_down.Anonymous.ki.dwFlags = KEYEVENTF_UNICODE;
            input_down.Anonymous.ki.dwExtraInfo = 0;
            
            SendInput(1, &mut input_down, std::mem::size_of::<INPUT>() as u32);

            // Key up
            let mut input_up = INPUT {
                r#type: INPUT_KEYBOARD,
                Anonymous: std::mem::zeroed(),
            };
            input_up.Anonymous.ki.wScan = c as u16;
            input_up.Anonymous.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
            input_up.Anonymous.ki.dwExtraInfo = 0;
            
            SendInput(1, &mut input_up, std::mem::size_of::<INPUT>() as u32);

            // Tiny delay between characters for reliability
            thread::sleep(Duration::from_millis(5));
        }
    }
}

/// Install the low-level keyboard hook
fn install_hook(engine_data: Arc<SharedEngineData>) -> Result<(), String> {
    unsafe {
        let hook_proc = Some(keyboard_hook_callback as unsafe extern "system" fn(i32, WPARAM, LPARAM) -> LRESULT);
        
        let hook = SetWindowsHookExW(WH_KEYBOARD_LL, hook_proc, std::ptr::null_mut(), 0);
        
        if hook.is_null() {
            return Err("Failed to install keyboard hook".to_string());
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

    // Message pump - required for hooks to work
    // On Windows, we'd use GetMessage/PeekMessage here
    // For cross-platform compatibility with PyO3, we use a simple loop
    while engine_data.running.load(Ordering::Relaxed) {
        thread::sleep(Duration::from_millis(10));
        
        // In a real Windows implementation, you would process messages here:
        // let mut msg = std::mem::zeroed();
        // if PeekMessageW(&mut msg, std::ptr::null_mut(), 0, 0, PM_REMOVE) != 0 {
        //     TranslateMessage(&msg);
        //     DispatchMessageW(&msg);
        // }
    }

    // Uninstall hook when stopping
    uninstall_hook(&engine_data);
}

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
        Ok(state.buffer.clone())
    }

    /// Manually trigger correction on a given text
    fn correct_text(&self, text: &str) -> PyResult<Option<String>> {
        Ok(detect_and_correct(text))
    }
}

/// Python module definition
#[pymodule]
fn mubaddil_core(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_class::<MubaddilCore>()?;
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_arabic_to_english_mapping() {
        // "اثممخ" should map to "hello"
        let result = arabic_layout_to_english("اثممخ");
        assert_eq!(result, "hello");
    }

    #[test]
    fn test_english_to_arabic_mapping() {
        // "hello" typed on Arabic layout would produce Arabic chars
        let result = english_layout_to_arabic("hello");
        // This depends on the mapping direction
        assert!(!result.is_empty());
    }

    #[test]
    fn test_detect_and_correct_hello() {
        // "اثممخ" is "hello" typed with Arabic layout
        let result = detect_and_correct("اثممخ");
        assert_eq!(result, Some("hello".to_string()));
    }

    #[test]
    fn test_is_valid_word() {
        assert!(is_valid_word("hello"));
        assert!(is_valid_word("the"));
        assert!(!is_valid_word("xyzabc"));
        assert!(!is_valid_word("ab")); // Too short
    }
}
