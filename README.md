<div align="center">

# Mubaddil | مبدل

### Intelligent Keyboard Layout Switcher for Windows

Automatically detects when you type using the wrong keyboard layout and fixes it instantly.

![Platform](https://img.shields.io/badge/Platform-Windows-blue)
![Language](https://img.shields.io/badge/Python-3.11+-yellow)
![Core](https://img.shields.io/badge/Core-C++20-green)
![Qt](https://img.shields.io/badge/UI-PySide6-success)
![License](https://img.shields.io/badge/License-MIT-orange)

</div>

---

# Overview

**Mubaddil (مبدل)** is a high-performance Windows utility that automatically detects when text has been typed using the wrong keyboard layout and intelligently converts it to the intended language.

Instead of deleting and retyping entire words, Mubaddil analyzes your input, predicts the intended language, and offers an instant correction with minimal interruption.

Designed for developers, students, professionals, writers, and anyone who frequently switches between Arabic and English keyboards.

---

# Why Mubaddil?

Typing with the wrong keyboard layout is one of the most common frustrations for bilingual users.

Examples:

```
ghg
```

↓

```
لالا
```

---

```
اثممخ
```

↓

```
hello
```

Instead of:

❌ Deleting everything

❌ Switching keyboard

❌ Retyping

Mubaddil fixes it automatically.

---

# Features

## Intelligent Detection

- Automatic keyboard layout detection
- Arabic ↔ English conversion
- Smart confidence scoring
- Dictionary validation
- Language probability analysis
- Word frequency analysis
- RapidFuzz similarity scoring

---

## High Performance Core

Built using modern C++20.

Features:

- Low-level Windows Keyboard Hook
- Unicode processing
- Word reconstruction
- Keyboard mapping
- Fast text conversion
- Memory optimized
- Multi-threaded processing

---

## Modern Windows UI

Built with PySide6.

Includes:

- Windows 11 Fluent Design
- Glassmorphism
- Acrylic Blur
- Mica-inspired interface
- Dark Mode
- Light Mode
- Smooth animations
- High-DPI support
- SVG icons
- Native feel

---

## Smart Popup

The popup:

- Appears only when needed
- Never steals focus
- Never interrupts typing
- Shows the corrected text
- Supports one-click replacement
- Automatically positions near the typing area

---

## Safe Replacement Engine

Mubaddil safely replaces the incorrect text by:

- Preserving cursor position
- Backing up clipboard
- Restoring clipboard
- Avoiding duplicated characters
- Preventing extra spaces

---

## Performance

Designed for daily usage.

Target performance:

| Metric | Target |
|----------|---------|
| Startup | <100 ms |
| Detection | <3 ms |
| Replacement | <10 ms |
| CPU Idle | <1% |
| RAM Usage | <30 MB |

---

# Project Structure

```
Mubaddil/

│

├── hook/
│   ├── Keyboard_hook.cpp
│   ├── Keyboard_hook.h
│   └── CMakeLists.txt
│
├── main.py
├── core.py
├── ui.py
├── requirements.txt
└── README.md
```

---

# Architecture

```
                 +--------------------+
                 |      User          |
                 +---------+----------+
                           |
                           v
              Windows Keyboard Events
                           |
                           v
          +------------------------------+
          |     C++ Keyboard Hook         |
          +------------------------------+
                           |
                           v
             Word Reconstruction Engine
                           |
                           v
             Language Detection Engine
                           |
                           v
             Keyboard Mapping Engine
                           |
                           v
            Confidence Calculation Engine
                           |
                           v
            Replacement Decision Engine
                           |
                           v
               Python UI (PySide6)
                           |
                           v
                  Smart Popup Window
                           |
                           v
                Instant Text Replacement
```

---

# Technologies

## UI

- PySide6
- Qt Material
- QtAwesome
- SuperQt

---

## Core

- Modern C++20
- Windows API
- Win32 Hooks
- STL
- RAII
- Smart Pointers

---

## Python Libraries

- keyboard
- pynput
- pyautogui
- pyperclip
- rapidfuzz
- regex
- loguru
- watchdog
- langdetect
- cachetools
- qasync
- psutil

---

# How It Works

1. Install a global keyboard hook.

2. Listen to keyboard events.

3. Build complete words.

4. Detect language.

5. Calculate confidence.

6. Convert keyboard layout.

7. Validate conversion.

8. Display popup if necessary.

9. Replace text safely.

10. Continue typing.

---

# Design Goals

- Extremely lightweight
- Native Windows experience
- High accuracy
- Fast response
- Minimal CPU usage
- Minimal RAM usage
- Modern interface
- Stable architecture
- Production-ready

---

# Future Features

- AI-assisted language prediction
- Custom dictionaries
- User-trained vocabulary
- Cloud synchronization
- Plugin system
- OCR text correction
- Clipboard history
- Multiple keyboard layouts
- Auto-learning engine
- Linux support
- macOS support

---

# Development

## Requirements

- Windows 10 / 11
- Python 3.11+
- CMake
- MSVC (Visual Studio 2022)
- C++20

---

## Install

```bash
pip install -r requirements.txt
```

---

## Build C++ Core

```bash
cd hook

mkdir build

cd build

cmake ..

cmake --build . --config Release
```

---

## Run

```bash
python main.py
```

---

# Contributing

Contributions are welcome.

Please:

- Open an issue.
- Fork the repository.
- Create a feature branch.
- Submit a pull request.

---

# License

MIT License

---

# Author

**Amr Shehata**

Founder of **HORUS Startup**

---

<div align="center">

### Mubaddil

**Type Naturally.  
We'll Handle the Keyboard.**

</div>
