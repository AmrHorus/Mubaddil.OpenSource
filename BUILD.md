# Mubaddil - Build Instructions

## Architecture Overview

Mubaddil is now a **fully native Windows application** built with:

- **C++20**: Windows integration, keyboard hooks, UI, clipboard, text replacement
- **Rust**: Core intelligence (detection, mapping, fuzzy matching, confidence scoring)
- **Qt6**: Modern cross-platform UI framework
- **CMake + Cargo**: Unified build system

```
┌─────────────────────────────────────────────────────────────┐
│                      Mubaddil.exe                           │
├─────────────────────────────────────────────────────────────┤
│  C++ Layer (Windows Integration)                            │
│  ├── Keyboard Hook (Win32 Low-Level)                       │
│  ├── Clipboard Management                                   │
│  ├── Text Replacement                                       │
│  ├── Qt6 UI (MainWindow, Dialogs, SystemTray)              │
│  └── FFI Bridge to Rust Core                                │
├─────────────────────────────────────────────────────────────┤
│  Rust Core (Safe Intelligence)                              │
│  ├── Language Detection                                     │
│  ├── Keyboard Layout Mapping                                │
│  ├── Fuzzy Matching                                         │
│  ├── Confidence Scoring                                     │
│  └── Dictionary Lookup                                      │
└─────────────────────────────────────────────────────────────┘
```

## Prerequisites

### Required Software

1. **Visual Studio 2022** (Community or higher)
   - Desktop development with C++
   - Windows 10/11 SDK
   - MSVC v143 or later

2. **CMake 3.20+**
   - Download from: https://cmake.org/download/
   - Or install via winget: `winget install Kitware.CMake`

3. **Qt 6.5+**
   - Download Qt Online Installer: https://www.qt.io/download
   - Install Qt 6.5 LTS with components:
     - Qt Widgets
     - Qt Base
   - Note the installation path (e.g., `C:\Qt\6.5.3\msvc2022_64`)

4. **Rust Toolchain**
   - Download rustup: https://rustup.rs/
   - Run: `rustup default stable`
   - Verify: `rustc --version` and `cargo --version`

### Optional Tools

- **Git**: For version control
- **vcpkg**: For package management (if needed)

## Build Steps

### Step 1: Clone Repository

```bash
git clone https://github.com/AmrHorus/Mubaddil.OpenSource.git
cd Mubaddil.OpenSource
```

### Step 2: Create Build Directory

```bash
mkdir build
cd build
```

### Step 3: Configure with CMake

#### Option A: Visual Studio 2022 Generator (Recommended)

```bash
cmake .. ^
    -G "Visual Studio 17 2022" ^
    -A x64 ^
    -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64"
```

Replace `C:\Qt\6.5.3\msvc2022_64` with your actual Qt installation path.

#### Option B: Ninja Generator (Faster Builds)

```bash
cmake .. ^
    -G Ninja ^
    -A x64 ^
    -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64" ^
    -DCMAKE_BUILD_TYPE=Release
```

#### Option C: Command Line (Quick Test)

```bash
cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64"
```

### Step 4: Build

#### Build Release Version

```bash
cmake --build . --config Release
```

#### Build Debug Version

```bash
cmake --build . --config Debug
```

#### Build with Verbose Output

```bash
cmake --build . --config Release --verbose
```

### Step 5: Run

The executable will be in:

```
build/bin/Release/Mubaddil.exe    (Release build)
build/bin/Debug/Mubaddil.exe      (Debug build)
```

Run as Administrator (required for keyboard hook):

```bash
# Right-click Mubaddil.exe → Run as Administrator
```

Or from command line (elevated):

```bash
cd bin\Release
.\Mubaddil.exe
```

## Build Configuration Options

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug/Release/RelWithDebInfo) |
| `CMAKE_PREFIX_PATH` | - | Qt installation path |
| `USE_STATIC_RUNTIME` | ON | Link static C++ runtime |
| `CMAKE_EXPORT_COMPILE_COMMANDS` | ON | Generate compile_commands.json |

### Rust Build Types

The Rust core is automatically built based on CMake configuration:

| CMake Build Type | Rust Build Mode |
|------------------|-----------------|
| Debug | `cargo build --debug` |
| Release | `cargo build --release` |
| RelWithDebInfo | `cargo build --release` |

## Project Structure

```
Mubaddil/
├── CMakeLists.txt          # Main CMake configuration
├── BUILD.md                # This file
├── README.md               # Project overview
├── rust_core/              # Rust core engine
│   ├── Cargo.toml          # Rust dependencies
│   └── src/
│       └── lib.rs          # Core detection/mapping logic
├── hook/                   # C++ keyboard hook engine
│   ├── CMakeLists.txt
│   ├── Keyboard_hook.cpp/h  # Low-level hook
│   ├── mapper.cpp/h         # Keyboard mapping
│   ├── detector.cpp/h       # Language detection
│   ├── buffer.cpp/h         # Word buffer
│   ├── clipboard.cpp/h      # Clipboard ops
│   ├── replacement.cpp/h    # Text replacement
│   ├── logger.cpp/h         # Logging
│   ├── bridge.cpp/h         # Python bridge (legacy)
│   └── exports.cpp          # C exports
├── src/                    # C++ UI layer
│   ├── main.cpp            # Application entry
│   ├── mainwindow.cpp/h    # Main window
│   ├── dialogs.cpp/h       # Dialogs
│   ├── systemtray.cpp/h    # System tray
│   ├── theme.cpp/h         # Dark/light theme
│   ├── config.cpp/h        # Settings
│   ├── core_engine.cpp/h   # Engine wrapper
│   └── ui_helpers.cpp/h    # UI utilities
├── assets/                 # Icons and resources
│   ├── mubaddil.ico
│   └── mubaddil.png
└── tests/                  # Test files
    └── ...
```

## Testing

### Rust Unit Tests

```bash
cd rust_core
cargo test
```

### C++ Tests

(To be added - currently manual testing)

### Manual Testing Checklist

1. ✅ Keyboard hook installs correctly
2. ✅ Typing "اثممخ" suggests "hello"
3. ✅ Typing "مرحبا" suggests "hello" (if typed wrong)
4. ✅ Suggestion popup appears
5. ✅ Alt+Enter accepts correction
6. ✅ Escape rejects correction
7. ✅ Clipboard preserved after correction
8. ✅ Cursor position maintained
9. ✅ System tray icon works
10. ✅ Settings persist across restarts

## Troubleshooting

### Rust Not Found

**Error**: `Rust toolchain not found`

**Solution**:
```bash
rustup install stable
rustup default stable
```

Verify:
```bash
rustc --version
cargo --version
```

### Qt Not Found

**Error**: `Could not find Qt6`

**Solution**: Specify Qt path explicitly:
```bash
cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64"
```

Find your Qt path:
```bash
dir C:\Qt\*\*\msvc*
```

### Keyboard Hook Fails

**Error**: `Failed to install keyboard hook`

**Solution**: Run as Administrator. The low-level keyboard hook requires elevated privileges.

### Build Fails with LNK1181

**Error**: `cannot open input file '...\mubaddil_core.lib'`

**Solution**: 
1. Ensure Rust built successfully
2. Check `build/rust_core/` for the library
3. Rebuild: `cmake --build . --target clean` then rebuild

### Unicode/Arabic Text Issues

**Problem**: Arabic text displays incorrectly

**Solution**: 
1. Ensure `/utf-8` flag is set (automatic in CMakeLists.txt)
2. Use Windows Terminal or modern console
3. Font must support Arabic (Segoe UI recommended)

## Performance Targets

| Metric | Target | Measurement |
|--------|--------|-------------|
| Startup Time | < 100 ms | From launch to tray icon |
| Detection Latency | < 3 ms | Per word analysis |
| Replacement Time | < 10 ms | Backspace + paste |
| CPU Usage (Idle) | < 1% | Task Manager |
| Memory Usage | < 30 MB | Working set |

## Deployment

### Creating a Distributable

1. Build Release version
2. Copy required DLLs:
   - Qt6Core.dll
   - Qt6Gui.dll
   - Qt6Widgets.dll
   - Platform plugins (qwindows.dll)
   - Style plugins

3. Use `windeployqt`:
```bash
cd bin\Release
windeployqt Mubaddil.exe --release
```

4. Test on clean Windows VM

### Future: Installer

Plans for WiX/InnoSetup installer:
- Single EXE installer
- Auto-start registration
- Admin privilege handling
- Uninstaller

## Development Workflow

### Daily Development

```bash
# Configure once
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64"

# Build and run
cmake --build . --config Debug
.\bin\Debug\Mubaddil.exe
```

### Making Changes

1. **Rust changes**: Edit `rust_core/src/lib.rs`, rebuild
2. **C++ changes**: Edit source files, rebuild
3. **UI changes**: Edit `src/*.cpp`, rebuild

### Code Style

**C++**:
- C++20 standard
- RAII and smart pointers
- const-correctness
- Namespaces for organization

**Rust**:
- Safe code by default
- Minimal unsafe blocks (FFI only)
- Comprehensive tests
- Clear error handling

## Migration Notes

This project was migrated from Python/PySide6 to native C++/Rust. Key changes:

| Component | Before | After |
|-----------|--------|-------|
| UI | PySide6 | Qt6 C++ |
| Core Logic | Python | Rust |
| Keyboard Hook | Python ctypes | Native C++ |
| Fuzzy Matching | RapidFuzz | Rust fuzzy-matcher |
| Build | PyInstaller | CMake + Cargo |
| Runtime | Python 3.11+ | Native EXE |

## License

MIT License - See LICENSE file for details.

## Support

- GitHub Issues: https://github.com/AmrHorus/Mubaddil.OpenSource/issues
- Documentation: README.md, MUBADDIL.md

---

**Last Updated**: 2024
**Version**: 1.0.0
