# Mubaddil Core - Rust Engine

This directory contains the Rust core engine for Mubaddil, an intelligent Windows keyboard layout switcher.

## Architecture

The Rust core provides:
- **Low-level keyboard hook** (`WH_KEYBOARD_LL`) for intercepting keystrokes system-wide
- **Word buffering** with configurable thresholds
- **Layout detection** using character mapping between Arabic and English keyboard layouts
- **Text correction** via Win32 `SendInput` API for sub-3ms latency
- **PyO3 bindings** for seamless Python integration

## Project Structure

```
rust_core/
├── Cargo.toml          # Rust package manifest with dependencies
└── src/
    └── lib.rs          # Main library source code
```

## Dependencies

- **pyo3** (v0.20): Python-Rust interop with `extension-module` feature
- **windows-sys** (v0.52): Win32 API bindings for hooks and input simulation
- **strsim**: String similarity metrics (Levenshtein distance)
- **fuzzy-matcher**: Fuzzy string matching for word validation
- **once_cell**: Lazy initialization utilities

## Building

### Prerequisites

1. **Rust Toolchain** (1.70+ recommended):
   ```bash
   curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
   ```

2. **Maturin** (Python packaging for Rust):
   ```bash
   pip install maturin
   ```

3. **Windows SDK** (for Win32 APIs, Windows only):
   - Install Visual Studio Build Tools or full Visual Studio
   - Ensure "Windows 10/11 SDK" is selected

### Build Commands

#### Development Build (with debugging symbols)
```bash
cd rust_core
maturin develop
```

#### Release Build (optimized for production)
```bash
cd rust_core
maturin develop --release
```

#### Build Wheel for Distribution
```bash
cd rust_core
maturin build --release
# Output: target/wheels/mubaddil_core-*.whl
```

## Usage

### Python Integration

```python
import mubaddil_core

# Create engine instance
core = mubaddil_core.MubaddilCore()

# Start the keyboard hook engine
core.start()

# Check if running
if core.is_running():
    print("Engine is active")

# Manually correct text (without hook)
corrected = core.correct_text("اثممخ")  # Returns "hello"
print(f"Corrected: {corrected}")

# Stop the engine
core.stop()
```

### With PySide6 Event Loop

See `test_engine.py` in the parent directory for a complete example integrating with PySide6.

## API Reference

### `MubaddilCore` Class

#### `__init__() -> MubaddilCore`
Create a new engine instance. The engine starts in a stopped state.

#### `start() -> None`
Start the background thread and install the low-level keyboard hook.
- Raises `RuntimeError` if already running
- Requires administrator privileges on Windows

#### `stop() -> None`
Stop the engine and uninstall the keyboard hook.
- Waits for the background thread to complete
- Safe to call multiple times

#### `is_running() -> bool`
Check if the engine is currently active.
- Returns `True` if the hook is installed and processing

#### `get_buffer() -> str`
Get the current word buffer content (for debugging).
- Returns the characters typed since the last word boundary

#### `correct_text(text: str) -> Optional[str]`
Manually trigger correction on a given text string.
- Returns the corrected text if a layout mismatch is detected
- Returns `None` if the text is already valid or no correction found

## Keyboard Layout Mapping

The engine supports bidirectional correction between:

### Arabic → English
When typing English words with Arabic keyboard layout active:
- `اثممخ` → `hello`
- `مثمر` → `world`

### English → Arabic
When typing Arabic words with English keyboard layout active:
- Detected via fuzzy matching against Arabic dictionary

## Detection Algorithm

1. **Buffer keystrokes** until word boundary (space, enter, tab) or timeout
2. **Check validity** against built-in dictionaries (English + Arabic)
3. **Try conversion** using keyboard layout mappings
4. **Validate converted text** against dictionaries
5. **Inject correction** if a valid alternative is found

### Word Validation
- Minimum length: 3 characters
- Dictionary lookup (exact match)
- Fuzzy matching (Skim algorithm)
- Levenshtein distance ≤ 1 for near-matches

## Thread Safety

The engine uses:
- `Arc<Mutex<EngineState>>` for shared state protection
- `AtomicBool` for running flag (lock-free)
- Separate OS thread for hook message pump
- No blocking operations in hook callback

## Unicode Handling

- Rust strings are UTF-8 native
- Windows `SendInput` uses UTF-16 (WCHAR) via `KEYEVENTF_UNICODE`
- Character mapping preserves Unicode scalar values
- No panics on invalid sequences (graceful fallback)

## Performance

- **Hook callback latency**: < 1ms (typical)
- **Text injection latency**: < 3ms total
- **Memory footprint**: < 1MB
- **CPU usage**: < 0.1% when idle

## Limitations

1. **Windows Only**: Uses Win32 APIs (`SetWindowsHookExW`, `SendInput`)
2. **Administrator Required**: Low-level hooks need elevated privileges
3. **Single Instance**: Multiple instances may conflict
4. **No UI Logic**: Pure backend; UI must be implemented in Python

## Troubleshooting

### Hook Installation Fails
- Ensure running as Administrator
- Check for conflicting keyboard software
- Verify Windows SDK is properly installed

### Correction Not Triggering
- Check minimum word length (default: 3 chars)
- Verify target word exists in dictionary
- Enable debug logging to inspect buffer

### Build Errors
```bash
# Clean and rebuild
cargo clean
maturin develop --release

# Update dependencies
cargo update
```

## Testing

Run unit tests (requires Rust toolchain):
```bash
cd rust_core
cargo test
```

Test cases cover:
- Arabic→English mapping
- English→Arabic mapping
- Word validation
- Correction detection

## License

Part of the Mubaddil project. See main repository for license terms.
