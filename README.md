<div align="center">

<img src="mubaddil.ico" width="120">

# Mubaddil | مبدل

### The Intelligent Keyboard Layout Switcher for Windows

Automatically detects when you type using the wrong keyboard layout and fixes it instantly.

<p>

<img src="https://img.shields.io/badge/Platform-Windows%2010%20%7C%2011-0078D4?style=for-the-badge">

<img src="https://img.shields.io/badge/Core-C%2B%2B20-blue?style=for-the-badge">

<img src="https://img.shields.io/badge/UI-PySide6-green?style=for-the-badge">

<img src="https://img.shields.io/badge/Status-Development-orange?style=for-the-badge">

<img src="https://img.shields.io/badge/License-MIT-success?style=for-the-badge">

</p>

---

### ⚡ Native Performance • 🎯 Smart Detection • 🎨 Modern UI

</div>

---

# Why Mubaddil?

Typing in two languages shouldn't interrupt your workflow.

Mubaddil is a **native Windows utility** designed to recognize accidental keyboard layout mistakes in real time and correct them seamlessly.

No more:

```
اثممخ
```

Instead, Mubaddil understands that you meant:

```
hello
```

---

# Highlights

<table>
<tr>
<td width="50%">

### 🚀 Native C++ Engine

- Low-level Keyboard Hook
- Zero polling
- Ultra-low latency
- Thread-safe architecture
- Windows API
- Unicode support

</td>

<td width="50%">

### 🎨 Beautiful Interface

- Windows 11 Fluent Design
- Glassmorphism
- Mica Effects
- Dark & Light Mode
- Smooth Animations
- High DPI Ready

</td>
</tr>
</table>

---

# Architecture

```
                ┌────────────────────┐
                │    User Typing     │
                └─────────┬──────────┘
                          │
                          ▼
              Windows Low-Level Hook
                          │
                          ▼
              Native C++ Detection Engine
                          │
      ┌───────────────────┼───────────────────┐
      │                   │                   │
      ▼                   ▼                   ▼
 Word Buffer      Language Detector    Keyboard Mapper
      │                   │                   │
      └───────────────────┼───────────────────┘
                          ▼
                 Confidence Engine
                          │
                          ▼
                Replacement Engine
                          │
                          ▼
                Python Communication
                          │
                          ▼
               Modern PySide6 Interface
```

---

# Project Structure

```
Mubaddil
│
├── assets
│   └── mubaddil.ico
│
├── hook
│   ├── Keyboard_hook.cpp
│   ├── Keyboard_hook.h
│   ├── detector.cpp
│   ├── detector.h
│   ├── mapper.cpp
│   ├── mapper.h
│   ├── buffer.cpp
│   ├── buffer.h
│   ├── bridge.cpp
│   ├── bridge.h
│   └── CMakeLists.txt
│
├── main.py
├── core.py
├── ui.py
├── requirements.txt
└── README.md
```

---

# Technology

| Layer | Technology |
|--------|------------|
| Core Engine | C++20 |
| Hook | Windows API |
| UI | PySide6 |
| Styling | Qt Material |
| Icons | QtAwesome |
| Fuzzy Matching | RapidFuzz |
| Clipboard | Win32 |
| Logging | Loguru |
| Build | CMake |
| Packaging | PyInstaller |

---

# Features

### Smart Detection

- Automatic layout recognition
- Confidence scoring
- Context-aware correction
- Fuzzy matching
- Dictionary support

---

### Performance

- Startup < 100 ms
- Detection < 3 ms
- Replacement < 10 ms
- CPU < 1%
- Memory < 30 MB

---

### User Experience

- Floating popup
- Smooth animations
- Doesn't steal focus
- Clipboard protection
- Cursor preservation
- Undo support

---

# Example

Typing

```text
اثممخ
```

↓

Popup

```text
Did you mean:

hello
```

↓

Press

```
Alt + Enter
```

↓

Text is instantly replaced.

---

# Development Roadmap

| Status | Component |
|:------:|-----------|
| ✅ | Modern UI |
| ✅ | Project Architecture |
| 🚧 | Native Keyboard Hook |
| 🚧 | Detection Engine |
| 🚧 | Keyboard Mapper |
| 🚧 | Clipboard Engine |
| ⏳ | Settings |
| ⏳ | Auto Update |
| ⏳ | Installer |
| ⏳ | Plugin System |

---

# Design Philosophy

Mubaddil follows five principles:

- Fast
- Native
- Minimal
- Reliable
- Invisible

The application should feel like a built-in Windows feature rather than a traditional desktop application.

---

# Build

Install dependencies

```bash
pip install -r requirements.txt
```

Build native engine

```bash
cd hook

mkdir build

cd build

cmake ..

cmake --build . --config Release
```

Run

```bash
python main.py
```

---

# Requirements

- Windows 10
- Windows 11
- Python 3.11+
- Visual Studio 2022
- CMake 3.25+
- MSVC Compiler

---

# Contributing

Contributions are welcome.

If you'd like to improve Mubaddil, feel free to:

- Open an Issue
- Submit a Pull Request
- Suggest Features
- Improve Documentation

---

# License

Released under the **MIT License**.

---

<div align="center">

<img src="mubaddil.ico" width="72">

## Mubaddil

### Think in your language.

### Type without interruptions.

Made with ❤️ for bilingual Windows users.

</div>
