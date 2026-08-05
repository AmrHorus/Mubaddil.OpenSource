@echo off
REM =============================================================================
REM Mubaddil Standalone EXE Builder (Windows)
REM =============================================================================
REM This script creates a fully standalone executable (.exe) from the Rust core
REM by bundling Python, the compiled .pyd module, and a launcher script using
REM PyInstaller.
REM
REM Prerequisites:
REM   1. Rust installed (rustup)
REM   2. Python with pip
REM   3. maturin installed (pip install maturin)
REM   4. pyinstaller installed (pip install pyinstaller)
REM   5. Visual Studio Build Tools with C++ workload
REM
REM Usage: build_exe.bat [release|debug]
REM =============================================================================

setlocal EnableDelayedExpansion

REM Configuration
set PROJECT_DIR=%~dp0
set BUILD_TYPE=release
set OUTPUT_DIR=%PROJECT_DIR%dist
set BUILD_DIR=%PROJECT_DIR%build_temp
set PYTHON_MODULE=mubaddil_core
set EXE_NAME=MubaddilEngine

REM Parse arguments
if not "%1"=="" set BUILD_TYPE=%1

echo.
echo =============================================================================
echo  Mubaddil Standalone EXE Builder
echo =============================================================================
echo  Build Type: %BUILD_TYPE%
echo  Output Dir: %OUTPUT_DIR%
echo.

REM Check if required tools are installed
where pyinstaller >nul 2>nul
if errorlevel 1 (
    echo [ERROR] PyInstaller not found. Installing...
    pip install pyinstaller
    if errorlevel 1 (
        echo [ERROR] Failed to install PyInstaller. Please run: pip install pyinstaller
        exit /b 1
    )
)

where maturin >nul 2>nul
if errorlevel 1 (
    echo [ERROR] maturin not found. Installing...
    pip install maturin
    if errorlevel 1 (
        echo [ERROR] Failed to install maturin.
        exit /b 1
    )
)

where cargo >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Rust/Cargo not found. Please install Rust from https://rustup.rs
    exit /b 1
)

REM Create directories
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Step 1: Build the Rust module
echo [INFO] Step 1/4: Building Rust core module...
echo.

if "%BUILD_TYPE%"=="release" (
    maturin build --release --manifest-path "%PROJECT_DIR%Cargo.toml" --out "%BUILD_DIR%"
) else (
    maturin build --manifest-path "%PROJECT_DIR%Cargo.toml" --out "%BUILD_DIR%"
)

if errorlevel 1 (
    echo.
    echo [ERROR] Rust build failed!
    exit /b 1
)

REM Find the .pyd file
set PYD_FILE=
for %%f in ("%BUILD_DIR%\*.pyd") do (
    set PYD_FILE=%%f
    goto :found_pyd
)

:found_pyd
if "!PYD_FILE!"=="" (
    echo [ERROR] No .pyd file found
    exit /b 1
)

echo [INFO] Built module: !PYD_FILE!
echo.

REM Step 2: Create launcher Python script
echo [INFO] Step 2/4: Creating launcher script...
echo.

(
echo import sys
echo import os
echo.
echo # Add current directory to path for finding the .pyd module
echo script_dir = os.path.dirname(os.path.abspath(sys.executable))
echo os.chdir(script_dir)
echo sys.path.insert(0, script_dir)
echo.
echo try:
echo     import mubaddil_core
echo     print("Mubaddil Engine initialized successfully!")
echo     print(f"Module location: {mubaddil_core.__file__}")
echo.
echo     # Start the engine
echo     core = mubaddil_core.MubaddilCore()
echo     print("Starting keyboard hook engine...")
echo     core.start()
echo     print(f"Engine running: {core.is_running()}")
echo.
echo     print("")
echo     print("=" * 60)
echo     print("Mubaddil Engine is now active in the background.")
echo     print("Press Ctrl+C to stop and exit.")
echo     print("=" * 60)
echo.
echo     # Keep running until interrupted
echo     import time
echo     while True:
echo         time.sleep(1)
echo.
except KeyboardInterrupt:
echo     print("\n\nStopping engine...")
echo     if 'core' in locals():
echo         core.stop()
echo         print("Engine stopped.")
echo     sys.exit(0)
except Exception as e:
echo     print(f"Error: {e}")
echo     import traceback
echo     traceback.print_exc()
echo     sys.exit(1)
) > "%BUILD_DIR%\launcher.py"

echo [INFO] Launcher script created: %BUILD_DIR%\launcher.py
echo.

REM Step 3: Copy .pyd to build directory
echo [INFO] Step 3/4: Preparing bundle...
copy /Y "!PYD_FILE!" "%BUILD_DIR%\mubaddil_core.pyd" >nul
echo [INFO] Module copied to build directory

REM Step 4: Build EXE with PyInstaller
echo.
echo [INFO] Step 4/4: Building standalone EXE with PyInstaller...
echo.

cd /d "%BUILD_DIR%"

pyinstaller ^
    --onefile ^
    --name "%EXE_NAME%" ^
    --icon=NONE ^
    --console ^
    --hidden-import=mubaddil_core ^
    --add-data "mubaddil_core.pyd;." ^
    launcher.py

if errorlevel 1 (
    echo.
    echo [ERROR] PyInstaller build failed!
    cd /d "%PROJECT_DIR%"
    exit /b 1
)

cd /d "%PROJECT_DIR%"

REM Copy EXE to output directory
if exist "%BUILD_DIR%\dist\%EXE_NAME%.exe" (
    copy /Y "%BUILD_DIR%\dist\%EXE_NAME%.exe" "%OUTPUT_DIR%\%EXE_NAME%.exe" >nul
    echo.
    echo [SUCCESS] Standalone EXE created successfully!
    echo.
    
    REM Display information
    echo =============================================================================
    echo  Build Artifacts
    echo =============================================================================
    dir /B "%OUTPUT_DIR%\*.exe"
    echo.
    for %%A in ("%OUTPUT_DIR%\%EXE_NAME%.exe") do (
        echo  File: %%~nxA
        echo  Size: %%~zA bytes
        echo  Path: %%~fA
    )
    echo.
    
    echo =============================================================================
    echo  Usage Instructions
    echo =============================================================================
    echo  1. Run the executable as Administrator (required for low-level hooks):
    echo     Right-click "%EXE_NAME%.exe" ^> Run as Administrator
    echo.
    echo  2. The engine will start automatically and run in the background
    echo  3. Press Ctrl+C in the console window to stop the engine
    echo.
    echo  IMPORTANT: Windows SmartScreen may warn about unsigned executables.
    echo             This is normal for self-built applications.
    echo.
    echo =============================================================================
) else (
    echo [ERROR] EXE file not found after PyInstaller build
    exit /b 1
)

REM Cleanup temporary files (optional - comment out to keep build artifacts)
echo.
set /p CLEANUP="Cleanup temporary build files? (Y/N): "
if /i "!CLEANUP!"=="Y" (
    echo [INFO] Cleaning up temporary files...
    rmdir /S /Q "%BUILD_DIR%"
    echo [INFO] Temporary files removed
)

endlocal
exit /b 0
