<div align="center">

<img src="mubaddil.ico" width="140" alt="Mubaddil Logo"/>

# مبدل | Mubaddil

### Smart Arabic ↔ English Keyboard Layout Switcher

A modern, lightweight, AI-assisted Windows utility that automatically detects words typed using the wrong keyboard layout and instantly suggests or replaces them.

![Platform](https://img.shields.io/badge/Platform-Windows%2010%20%26%2011-0078D6?style=for-the-badge)
![Language](https://img.shields.io/badge/Python-3.11+-3776AB?style=for-the-badge&logo=python&logoColor=white)
![Core](https://img.shields.io/badge/Core-C%2B%2B20-00599C?style=for-the-badge&logo=cplusplus)
![Qt](https://img.shields.io/badge/UI-PySide6-41CD52?style=for-the-badge&logo=qt)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

---

**Fast • Lightweight • Native • Modern • Intelligent**

</div>

---

# 📖 About

Typing with the wrong keyboard layout is one of the most common productivity problems for bilingual users.

Instead of deleting everything and typing again, **Mubaddil** automatically detects the mistake, intelligently reconstructs the intended word, and suggests the correct version instantly.

The application is designed to feel like a native Windows utility with commercial-grade performance, modern UI, and minimal resource usage.

---

# ✨ Features

## 🚀 Intelligent Detection

- Automatic Arabic ↔ English layout detection
- Confidence-based correction engine
- Real-time word analysis
- Smart typo recognition
- Unicode-aware processing
- Context-aware language detection

---

## ⚡ High Performance

- Native C++20 Core
- Windows Low-Level Keyboard Hook
- Event-driven architecture
- Multi-threaded processing
- Low memory footprint
- Minimal CPU usage
- Zero polling

---

## 🎨 Modern User Interface

- Windows 11 inspired design
- Fluent Design
- Glassmorphism
- Acrylic & Mica effects
- Smooth animations
- High DPI support
- Dark & Light themes
- SVG icon support
- Adaptive layouts

---

## 🔒 Safe Replacement

- Clipboard preservation
- Cursor preservation
- Undo support
- Instant replacement
- Non-destructive workflow
- No data loss

---

## ⚙️ Customization

- Auto replace
- Manual confirmation
- Keyboard shortcuts
- Theme selection
- Startup behavior
- Popup duration
- Logging level
- Whitelist / Blacklist

---

# 🏗 Architecture

```
Mubaddil
│
├── hook
│   ├── Keyboard_hook.cpp
│   ├── Keyboard_hook.h
│   └── CMakeLists.txt
│
├── main.py
├── core.py
├── ui.py
├── requirements.txt
│
└── mubaddil.ico
```

---

# 🧠 Architecture Overview

```
                 User Typing
                      │
                      ▼
           Windows Keyboard Hook
                      │
                      ▼
              Native C++ Engine
                      │
      ┌───────────────┴───────────────┐
      ▼                               ▼
 Word Buffer                  Language Detector
      │                               │
      └───────────────┬───────────────┘
                      ▼
            Confidence Engine
                      │
                      ▼
          Keyboard Layout Mapper
                      │
                      ▼
             Replacement Engine
                      │
                      ▼
            Python Communication
                      │
                      ▼
             Modern Qt Interface
```

---

# 🛠 Technology Stack

## Frontend

- PySide6
- Qt Material
- QtAwesome
- SuperQt

## Core Engine

- Modern C++20
- Windows API
- Win32 Hooks
- STL
- Unicode
- Multi-threading

## Python

- RapidFuzz
- Regex
- Loguru
- PyWin32
- PyAutoGUI
- Pyperclip
- Watchdog
- CacheTools

---

# ⚙️ How It Works

Example

User types:

```
اثممخ
```

Mubaddil detects:

```
hello
```

or

User types:

```
ghg
```

Mubaddil detects:

```
لالا
```

The popup appears only when the confidence score indicates the correction is significantly more likely than the original input.

---

# 🎯 Performance Goals

| Item | Target |
|------|--------|
| Startup | <100 ms |
| Detection | <3 ms |
| Replacement | <10 ms |
| Idle CPU | <1% |
| Memory | <30 MB |

---

# 🔒 Safety

Mubaddil never stores your typing.

The application only processes text locally on your computer.

No telemetry.

No cloud processing.

No user tracking.

No data collection.

---

# 📦 Requirements

- Windows 10
- Windows 11
- Python 3.11+
- Visual Studio 2022
- CMake
- MSVC Compiler

---

# 🚀 Installation

Clone the repository

```bash
git clone https://github.com/yourusername/Mubaddil.git
```

Enter the project

```bash
cd Mubaddil
```

Install dependencies

```bash
pip install -r requirements.txt
```

Build the C++ core

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

# 📂 Project Structure

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
├── mubaddil.ico
└── README.md
```

---

# 📋 Roadmap

- [ ] Native C++ Core
- [ ] Low-Level Keyboard Hook
- [ ] Confidence Engine
- [ ] Popup UI
- [ ] Auto Replace
- [ ] Settings Window
- [ ] Custom Dictionaries
- [ ] Multi-language Support
- [ ] Plugin System
- [ ] Installer
- [ ] Auto Update

---

# 🤝 Contributing

Contributions are welcome.

Feel free to submit issues, feature requests, or pull requests to improve Mubaddil.

---

# 📜 License

This project is licensed under the MIT License.

---

<div align="center">

## مبدل | Mubaddil

### Type Naturally. Switch Intelligently.

<img src="mubaddil.ico" width="90"/>

Built with ❤️ for bilingual users.

</div>
