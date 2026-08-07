# Building Mubaddil - Complete C++ Application

This guide explains how to build the Mubaddil application entirely in C++.

## Prerequisites

### Windows
- **Visual Studio 2022** (with C++ desktop development workload)
- **CMake 3.20+** 
- **Qt 6.x** for Windows
- **Windows SDK**

### Installation Steps

1. **Install Visual Studio 2022**
   - Download from: https://visualstudio.microsoft.com/
   - Select "Desktop development with C++" workload

2. **Install Qt 6**
   - Download Qt Online Installer from: https://www.qt.io/download
   - Install Qt 6.5+ with MSVC 2022 components

3. **Install CMake**
   - Download from: https://cmake.org/download/
   - Or install via winget: `winget install Kitware.CMake`

## Build Instructions

### Using Visual Studio Developer Command Prompt

```batch
# Open "Developer Command Prompt for VS 2022"
cd C:\path\to\mubaddil

# Create build directory
mkdir build
cd build

# Configure with CMake (specify Qt path)
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64"

# Build Release version
cmake --build . --config Release

# The executable will be in: build\bin\Release\Mubaddil.exe
```

### Using PowerShell

```powershell
cd C:\path\to\mubaddil

# Create and enter build directory
New-Item -ItemType Directory -Force build | Set-Location

# Configure
cmake .. -G "Visual Studio 17 2022" -A x64 `
    -DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64"

# Build
cmake --build . --config Release
```

### Using Qt Creator

1. Open Qt Creator
2. File → Open File or Project
3. Select `CMakeLists.txt` in the root directory
4. Configure kit to use MSVC 2022 + Qt 6
5. Click Build → Run

## Project Structure

```
mubaddil/
├── CMakeLists.txt          # Main CMake configuration
├── BUILD.md                # This file
├── hook/                   # Core keyboard engine (existing C++)
│   ├── CMakeLists.txt
│   ├── Keyboard_hook.cpp/h
│   ├── mapper.cpp/h
│   ├── detector.cpp/h
│   ├── buffer.cpp/h
│   ├── clipboard.cpp/h
│   ├── replacement.cpp/h
│   ├── logger.cpp/h
│   ├── bridge.cpp/h
│   └── exports.cpp
└── src/                    # New Qt UI (all C++)
    ├── main.cpp            # Application entry point
    ├── mainwindow.cpp/h    # Main window
    ├── dialogs.cpp/h       # Dialogs (suggestion, history, rejected)
    ├── systemtray.cpp/h    # System tray icon
    ├── theme.cpp/h         # Application theme
    ├── config.cpp/h        # Configuration management
    ├── core_engine.cpp/h   # Core engine wrapper
    └── ui_helpers.cpp/h    # UI helper components
```

## Features

The C++ version includes:

✅ **Complete Keyboard Hook Engine** (existing C++ code)
- Low-level keyboard monitoring
- English ↔ Arabic layout mapping
- Language detection
- Text replacement via SendInput

✅ **Native Qt6 User Interface**
- Modern dark theme
- Statistics dashboard
- Correction history
- Rejected words list
- System tray integration
- Settings management

✅ **All Original Features**
- Real-time keyboard monitoring
- Suggestion dialogs
- Bidirectional conversion
- Auto-correction support
- Minimize to tray

## Running the Application

After building:

```batch
cd build\bin\Release
Mubaddil.exe
```

**Note:** Run as Administrator for keyboard hook functionality.

## Troubleshooting

### CMake can't find Qt
```
-DCMAKE_PREFIX_PATH="C:\Qt\6.5.3\msvc2022_64"
```
Make sure this path matches your Qt installation.

### Build errors
- Ensure you're using the **Developer Command Prompt** for VS 2022
- Check that all Qt components are installed (Core, Gui, Widgets)
- Verify CMake version is 3.20 or higher

### Runtime errors
- Run as Administrator (required for keyboard hooks)
- Ensure no other keyboard hooks are conflicting
- Check Windows Event Viewer for detailed error logs

## Migration from Python

The original Python files (`main.py`, `core.py`, `ui.py`) are kept for reference but are no longer needed. The complete application now runs natively in C++.

To remove Python dependencies:
1. Delete `main.py`, `core.py`, `ui.py`
2. Delete `requirements.txt`
3. Keep only the C++ source files and CMake configuration
