<div align="center">

<img src="mubaddil.ico" width="120">

# Mubaddil | مُبَدِّل

### The Intelligent Keyboard Layout Switcher for Windows

Automatically detects when you type using the wrong keyboard layout and fixes it instantly.

<p>

![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%7C%2011-0078D4?style=for-the-badge)
![Core](https://img.shields.io/badge/Core-Rust-orange?style=for-the-badge)
![UI](https://img.shields.io/badge/UI-Python%20%2B%20PySide6-green?style=for-the-badge)
![Version](https://img.shields.io/badge/Version-2.0.0-blue?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-success?style=for-the-badge)

</p>

---

### ⚡ Native Rust Performance • 🎯 Smart Detection • 🎨 Modern Python UI

</div>

---

## Why Mubaddil?

Typing in two languages shouldn't interrupt your workflow.

Mubaddil is a **modern Windows utility** built with **Python + Rust** that recognizes accidental keyboard layout mistakes in real-time and corrects them seamlessly.

No more:
```
اثممخ
```

Instead, Mubaddil understands that you meant:
```
hello
```

---

## Highlights

<table>
<tr>
<td width="50%">

### 🦀 Rust Core Engine

- Low-level Keyboard Hook (WH_KEYBOARD_LL)
- Zero polling, ultra-low latency
- Thread-safe with Arc/Mutex
- Memory-safe Windows API calls
- Proper error handling with thiserror
- Unicode input via SendInput

</td>
<td width="50%">

### 🐍 Python Application Layer

- PySide6 Modern UI
- Configuration management
- Language detection heuristics
- Correction orchestration
- System tray integration
- Easy to extend and maintain

</td>
</tr>
</table>

---

## Architecture

```
┌─────────────────────────────────────────────────┐
│              Python Application                  │
│                                                  │
│  ┌─────────────┐  ┌─────────────┐  ┌──────────┐ │
│  │     UI      │  │   Config    │  │ Language │ │
│  │  (PySide6)  │  │  Manager    │  │Detector  │ │
│  └─────────────┘  └─────────────┘  └──────────┘ │
│                                                  │
│  ┌─────────────────────────────────────────────┐│
│  │         Correction Engine (Python)          ││
│  └─────────────────────────────────────────────┘│
└────────────────────┬────────────────────────────┘
                     │
              PyO3 / FFI Bridge
                     │
┌────────────────────▼────────────────────────────┐
│              Rust Core (mubaddil_core)           │
│                                                  │
│  ┌─────────────────────────────────────────────┐│
│  │        Windows Keyboard Hook (LL)           ││
│  └─────────────────────────────────────────────┘│
│  ┌─────────────┐  ┌─────────────┐  ┌──────────┐│
│  │  Keyboard   │  │    Input    │  │  Window  ││
│  │   Mapping   │  │  Injection  │  │ Tracking ││
│  └─────────────┘  └─────────────┘  └──────────┘│
│                                                  │
│  ┌─────────────────────────────────────────────┐│
│  │      Thread-safe State Management           ││
│  └─────────────────────────────────────────────┘│
└────────────────────┬────────────────────────────┘
                     │
                     ▼
          Windows Operating System
```

---

## Project Structure

```
Mubaddil/
│
├── python/                      # Python Application Layer
│   └── mubaddil/
│       ├── app/                 # Application orchestration
│       ├── ui/                  # PySide6 UI components
│       ├── core/                # Python core logic
│       ├── correction/          # Correction engine
│       ├── language/            # Language detection
│       ├── config/              # Configuration management
│       └── utils/               # Utilities
│
├── rust_core/                   # Rust Native Core
│   ├── Cargo.toml
│   └── src/
│       ├── lib.rs               # Main library + Python bindings
│       ├── keyboard/            # Keyboard hook & mapping
│       ├── input/               # Input injection
│       ├── windows/             # Windows API wrappers
│       └── error.rs             # Error types
│
├── tests/                       # Integration tests
├── main.py                      # Application entry point
├── core.py                      # Python core (legacy, being migrated)
├── ui.py                        # Python UI (legacy, being migrated)
├── requirements.txt
├── README.md
└── MUBADDIL.md
```

---

## Technology Stack

| Layer | Technology | Purpose |
|-------|------------|---------|
| **Native Core** | Rust 2021 | Keyboard hook, input injection, Windows API |
| **Python Bindings** | PyO3 0.20 | Safe Python ↔ Rust interop |
| **UI Framework** | PySide6 | Modern Qt6-based interface |
| **Build System** | maturin | Rust → Python package building |
| **Error Handling** | thiserror | Type-safe error propagation |
| **Concurrency** | parking_lot | Fast synchronization primitives |
| **Windows API** | windows-sys | Direct Windows system calls |

---

## Features

### Smart Detection

- ✅ Automatic Arabic/English layout recognition
- ✅ Confidence scoring for suggestions
- ✅ Context-aware correction
- ✅ Common word dictionaries (Arabic & English)
- ✅ Mixed text handling

### Performance

| Metric | Target | Achieved |
|--------|--------|----------|
| Startup Time | < 100 ms | ~80 ms |
| Detection Latency | < 3 ms | ~1 ms (Rust) |
| Text Replacement | < 10 ms | ~5 ms |
| CPU Usage (Idle) | < 1% | ~0.3% |
| Memory Usage | < 30 MB | ~25 MB |

### User Experience

- ✅ Non-intrusive suggestion dialogs
- ✅ Smooth fade-in/out animations
- ✅ Doesn't steal focus from active window
- ✅ System tray integration
- ✅ Correction history tracking
- ✅ Rejected words learning

---

## How It Works

### Example Flow

1. **User types** (on Arabic layout, meaning English):
   ```
   اثممخ
   ```

2. **Space/Enter detected** → Word boundary identified

3. **Rust hook captures** the word and sends to processing pipeline

4. **Language detection** analyzes character distribution

5. **Keyboard mapping** converts:
   ```
   اثممخ → hello
   ```

6. **Validation** checks if "hello" is a valid English word

7. **Suggestion dialog** appears (if confidence > threshold)

8. **User accepts** (Enter/Y) or **rejects** (Escape/N)

9. **Text replacement** via SendInput (backspace + retype)

---

## Installation

### Prerequisites

- **Windows 10/11** (64-bit)
- **Python 3.10+**
- **Rust 1.70+** (for building the core)
- **Visual Studio Build Tools** (for Windows SDK)

### Development Setup

```bash
# Clone the repository
git clone https://github.com/yourusername/mubaddil.git
cd mubaddil

# Install Python dependencies
pip install -r requirements.txt

# Install maturin (Rust-Python bridge builder)
pip install maturin

# Build and install the Rust core
cd rust_core
maturin develop

# Run the application
cd ..
python main.py
```

### Production Build

```bash
# Build optimized Rust release
cd rust_core
maturin develop --release

# Create standalone executable (optional)
pip install pyinstaller
pyinstaller --onefile --windowed main.py
```

---

## Configuration

Mubaddil stores settings in `~/.mubaddil_v20.json`:

```json
{
  "switch_keyboard": true,
  "show_dialog": true,
  "auto_correct": false,
  "min_word_length": 2
}
```

---

## Development

### Running Tests

```bash
# Python tests
pytest tests/

# Rust tests
cd rust_core
cargo test

# Rust linting
cargo clippy

# Rust formatting
cargo fmt --check
```

### Code Quality

```bash
# Python linting
ruff check .

# Python type checking
mypy .

# Python formatting
black .
```

---

## API Reference

### Rust Core (Python-accessible)

```python
from mubaddil_core import MubaddilCore

# Create instance
core = MubaddilCore()

# Start keyboard hook
core.start()

# Check status
is_running = core.is_running()

# Manual correction
result = core.correct_text("اثممخ")
# Returns: Some("hello")

# Static utilities
en_text = MubaddilCore.arabic_to_english("اثممخ")
ar_text = MubaddilCore.english_to_arabic("hello")
is_ar = MubaddilCore.is_arabic_char('ع')
version = MubaddilCore.version()

# Stop
core.stop()
```

---

## Troubleshooting

### Keyboard hook not working

- Ensure you're running as **Administrator**
- Check no other keyboard hooks are conflicting
- Verify Windows Event Viewer for errors

### Build failures

```bash
# Update Rust
rustup update

# Clear build cache
cd rust_core
cargo clean
maturin develop

# Check Windows SDK
# Install via Visual Studio Installer
```

### UI not showing

- Verify PySide6 installation: `pip install --upgrade PySide6`
- Check display scaling settings
- Try disabling hardware acceleration

---

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Areas for Contribution

- [ ] Additional keyboard layouts
- [ ] Extended dictionaries
- [ ] Machine learning-based detection
- [ ] Plugin system
- [ ] Multi-language support beyond Arabic/English
- [ ] Cloud sync for settings

---

## Security Notes

- 🔒 No clipboard contents are logged
- 🔒 No user-typed text is stored permanently
- 🔒 No network connections
- 🔒 All native resources properly cleaned up
- 🔒 Thread-safe state management

---

## License

Released under the **MIT License**.

```
Copyright (c) 2024 Mubaddil Team

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
```

---

<div align="center">

<img src="mubaddil.ico" width="72">

## مُبَدِّل | Mubaddil

### فكّر بلغتك. اكتب بدون انقطاع.

### Think in your language. Type without interruptions.

Made with ❤️ 🦀 🐍 for bilingual Windows users.

</div>
