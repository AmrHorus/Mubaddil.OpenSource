@echo off
REM ============================================================================
REM Mubaddil Build Script for Windows
REM Builds C++ hook, Rust core, and packages everything into Mubaddil.exe
REM ============================================================================

setlocal EnableDelayedExpansion

echo.
echo ============================================================================
echo                    MUBADDIL BUILD SCRIPT
echo ============================================================================
echo.

REM Check if running on Windows
if not defined OS (
    echo ERROR: This script must be run on Windows
    exit /b 1
)

REM Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo WARNING: Not running as Administrator. Low-level keyboard hooks require admin rights.
    echo The built executable MUST be run as Administrator.
    echo.
)

REM Configuration
set PROJECT_ROOT=%~dp0
set BUILD_DIR=%PROJECT_ROOT%build
set RELEASE_DIR=%PROJECT_ROOT%Release
set BIN_DIR=%PROJECT_ROOT%bin
set VENV_DIR=%PROJECT_ROOT%venv
set HOOK_DIR=%PROJECT_ROOT%hook
set RUST_DIR=%PROJECT_ROOT%rust_core

REM Build type (default to Release)
set BUILD_TYPE=Release
if "%1"=="debug" set BUILD_TYPE=Debug

echo Build Type: %BUILD_TYPE%
echo Project Root: %PROJECT_ROOT%
echo.

REM ============================================================================
REM Step 1: Check Prerequisites
REM ============================================================================
echo [1/7] Checking prerequisites...

REM Check for Python
python --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Python is not installed or not in PATH
    echo Please install Python from https://python.org
    exit /b 1
)
echo   ✓ Python found

REM Check for pip
pip --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: pip is not available
    exit /b 1
)
echo   ✓ pip found

REM Check for CMake
cmake --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from https://cmake.org
    exit /b 1
)
echo   ✓ CMake found

REM Check for Rust/Cargo
cargo --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Rust/Cargo is not installed
    echo Please install Rust from https://rustup.rs
    exit /b 1
)
echo   ✓ Rust found

REM Check for Visual Studio Build Tools
where cl.exe >nul 2>&1
if errorlevel 1 (
    echo ERROR: Visual Studio Build Tools not found
    echo Please install Visual Studio Build Tools from:
    echo https://visualstudio.microsoft.com/visual-cpp-build-tools/
    exit /b 1
)
echo   ✓ Visual Studio Build Tools found

REM Check for maturin
pip show maturin >nul 2>&1
if errorlevel 1 (
    echo   Installing maturin...
    pip install maturin --quiet
)
echo   ✓ maturin found

REM Check for PyInstaller
pip show pyinstaller >nul 2>&1
if errorlevel 1 (
    echo   Installing PyInstaller...
    pip install pyinstaller --quiet
)
echo   ✓ PyInstaller found

echo.

REM ============================================================================
REM Step 2: Create Virtual Environment and Install Dependencies
REM ============================================================================
echo [2/7] Setting up Python environment...

if exist "%VENV_DIR%" (
    echo   Removing old virtual environment...
    rmdir /s /q "%VENV_DIR%"
)

python -m venv "%VENV_DIR%"
if errorlevel 1 (
    echo ERROR: Failed to create virtual environment
    exit /b 1
)

call "%VENV_DIR%\Scripts\activate.bat"
if errorlevel 1 (
    echo ERROR: Failed to activate virtual environment
    exit /b 1
)

echo   Installing requirements...
pip install --upgrade pip --quiet
pip install -r "%PROJECT_ROOT%requirements.txt" --quiet
if errorlevel 1 (
    echo WARNING: Some requirements may have failed to install
)

echo   ✓ Python environment ready

echo.

REM ============================================================================
REM Step 3: Build C++ Hook DLL
REM ============================================================================
echo [3/7] Building C++ hook library...

set HOOK_BUILD_DIR=%HOOK_DIR%\build_%BUILD_TYPE%

if exist "%HOOK_BUILD_DIR%" (
    echo   Cleaning previous C++ build...
    rmdir /s /q "%HOOK_BUILD_DIR%"
)

mkdir "%HOOK_BUILD_DIR%"
cd "%HOOK_BUILD_DIR%"

echo   Configuring CMake...
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE% .. 
if errorlevel 1 (
    echo ERROR: CMake configuration failed
    cd "%PROJECT_ROOT%"
    exit /b 1
)

echo   Building C++ library...
cmake --build . --config %BUILD_TYPE%
if errorlevel 1 (
    echo ERROR: C++ build failed
    cd "%PROJECT_ROOT%"
    exit /b 1
)

REM Copy DLL to bin directory
mkdir "%BIN_DIR%" 2>nul
copy "%HOOK_BUILD_DIR%\bin\%BUILD_TYPE%\MubaddilCore.dll" "%BIN_DIR%\" >nul 2>&1
if errorlevel 1 (
    echo WARNING: Could not copy MubaddilCore.dll to bin directory
) else (
    echo   ✓ MubaddilCore.dll copied to bin/
)

cd "%PROJECT_ROOT%"
echo   ✓ C++ hook built successfully

echo.

REM ============================================================================
REM Step 4: Build Rust Core Module
REM ============================================================================
echo [4/7] Building Rust core module...

cd "%RUST_DIR%"

REM Build with maturin
if "%BUILD_TYPE%"=="Release" (
    echo   Building Rust module in Release mode...
    maturin build --release
) else (
    echo   Building Rust module in Debug mode...
    maturin build
)

if errorlevel 1 (
    echo ERROR: Rust build failed
    cd "%PROJECT_ROOT%"
    exit /b 1
)

REM Find and copy the built module
for /f "delims=" %%i in ('dir /b target\wheels\*.whl 2^>nul') do set WHEEL_FILE=%%i

if defined WHEEL_FILE (
    echo   Installing Rust wheel...
    pip install "target\wheels\%WHEEL_FILE%" --force-reinstall
    if errorlevel 1 (
        echo WARNING: Could not install Rust wheel
    ) else (
        echo   ✓ Rust module installed
    )
) else (
    echo WARNING: No wheel file found, trying develop mode...
    maturin develop
    if errorlevel 1 (
        echo WARNING: Rust develop mode also failed
    )
)

cd "%PROJECT_ROOT%"
echo   ✓ Rust core build complete

echo.

REM ============================================================================
REM Step 5: Prepare Assets and Resources
REM ============================================================================
echo [5/7] Preparing assets...

REM Create Release directory
mkdir "%RELEASE_DIR%" 2>nul

REM Copy icon files
if exist "%PROJECT_ROOT%mubaddil.ico" (
    copy "%PROJECT_ROOT%mubaddil.ico" "%RELEASE_DIR%\" >nul
    echo   ✓ Icon copied
)

REM Copy any additional assets
if exist "%PROJECT_ROOT%assets" (
    xcopy /E /I /Y "%PROJECT_ROOT%assets" "%RELEASE_DIR%\assets" >nul
    echo   ✓ Assets copied
)

echo   ✓ Assets prepared

echo.

REM ============================================================================
REM Step 6: Generate PyInstaller Spec File
REM ============================================================================
echo [6/7] Generating PyInstaller specification...

(
echo # -*- mode: python ; coding: utf-8 -*-
echo.
echo block_cipher = None
echo.
echo a = Analysis(
echo     ['main.py'],
echo     pathex=[],
echo     binaries=[
echo         %%(
echo             'bin/MubaddilCore.dll', 'MubaddilCore.dll'
echo         ) if exists('bin/MubaddilCore.dll') else (),
echo     ],
echo     datas=[
echo         ('mubaddil.ico', '.'),
echo         ('requirements.txt', '.'),
echo     ],
echo     hiddenimports=[
echo         'PySide6',
echo         'PySide6.QtCore',
echo         'PySide6.QtGui',
echo         'PySide6.QtWidgets',
echo         'core',
echo         'ui',
echo         'mubaddil_core',
echo         'ctypes',
echo         'json',
echo         'threading',
echo         'logging',
echo     ],
echo     hookspath=[],
echo     hooksconfig={},
echo     runtime_hooks=[],
echo     excludes=[],
echo     win_no_prefer_redirects=False,
echo     win_private_assemblies=False,
echo     cipher=block_cipher,
echo     noarchive=False,
echo )
echo.
echo pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)
echo.
echo exe = EXE(
echo     pyz,
echo     a.scripts,
echo     a.binaries,
echo     a.zipfiles,
echo     a.datas,
echo     [],
echo     name='Mubaddil',
echo     debug=False,
echo     bootloader_ignore_signals=False,
echo     strip=False,
echo     upx=True,
echo     upx_exclude=[],
echo     runtime_tmpdir=None,
echo     console=False,
echo     disable_windowed_traceback=False,
echo     argv_emulation=False,
echo     target_arch=None,
echo     codesign_identity=None,
echo     entitlements_file=None,
echo     icon='mubaddil.ico',
echo )
) > "%RELEASE_DIR%\mubaddil.spec"

echo   ✓ PyInstaller spec generated

echo.

REM ============================================================================
REM Step 7: Build Executable with PyInstaller
REM ============================================================================
echo [7/7] Building Mubaddil.exe with PyInstaller...

cd "%RELEASE_DIR%"

REM Clean previous builds
if exist "build" rmdir /s /q "build"
if exist "dist" rmdir /s /q "dist"

REM Run PyInstaller
pyinstaller --clean mubaddil.spec
if errorlevel 1 (
    echo ERROR: PyInstaller build failed
    cd "%PROJECT_ROOT%"
    exit /b 1
)

REM Verify executable was created
if exist "dist\Mubaddil.exe" (
    echo   ✓ Mubaddil.exe created successfully!
    echo.
    echo ============================================================================
    echo                         BUILD SUCCESSFUL
    echo ============================================================================
    echo.
    echo Executable location: %RELEASE_DIR%\dist\Mubaddil.exe
    echo.
    echo IMPORTANT: Run Mubaddil.exe as Administrator to enable keyboard hooks.
    echo.
) else (
    echo ERROR: Mubaddil.exe was not created
    cd "%PROJECT_ROOT%"
    exit /b 1
)

cd "%PROJECT_ROOT%"

REM Cleanup
deactivate 2>nul

exit /b 0
