@echo off
REM =============================================================================
REM Mubaddil Rust Core Build Script (Windows)
REM =============================================================================
REM This script builds the optimized release version of the Mubaddil engine
REM and extracts the compiled binary (.pyd) to a standalone executable format.
REM
REM Prerequisites:
REM   1. Rust installed (rustup)
REM   2. Python with pip
REM   3. maturin installed (pip install maturin)
REM   4. Visual Studio Build Tools with C++ workload
REM
REM Usage: build.bat [release|debug]
REM =============================================================================

setlocal EnableDelayedExpansion

REM Configuration
set PROJECT_DIR=%~dp0
set BUILD_TYPE=release
set OUTPUT_DIR=%PROJECT_DIR%bin
set PYTHON_MODULE=mubaddil_core

REM Parse arguments
if not "%1"=="" set BUILD_TYPE=%1

echo.
echo =============================================================================
echo  Mubaddil Core Engine Builder
echo =============================================================================
echo  Build Type: %BUILD_TYPE%
echo  Output Dir: %OUTPUT_DIR%
echo.

REM Check if maturin is installed
where maturin >nul 2>nul
if errorlevel 1 (
    echo [ERROR] maturin not found. Installing...
    pip install maturin
    if errorlevel 1 (
        echo [ERROR] Failed to install maturin. Please run: pip install maturin
        exit /b 1
    )
)

REM Check if Rust is installed
where cargo >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Rust/Cargo not found. Please install Rust from https://rustup.rs
    exit /b 1
)

REM Create output directory
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM Clean previous builds (optional, comment out to speed up incremental builds)
echo [INFO] Cleaning previous builds...
cargo clean --manifest-path "%PROJECT_DIR%Cargo.toml"

REM Build with maturin
echo.
echo [INFO] Building %BUILD_TYPE% binary with maturin...
echo.

if "%BUILD_TYPE%"=="release" (
    maturin build --release --manifest-path "%PROJECT_DIR%Cargo.toml" --out "%OUTPUT_DIR%"
) else (
    maturin build --manifest-path "%PROJECT_DIR%Cargo.toml" --out "%OUTPUT_DIR%"
)

if errorlevel 1 (
    echo.
    echo [ERROR] Build failed! Check the errors above.
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully!
echo.

REM Find the generated .pyd file
set PYD_FILE=
for %%f in ("%OUTPUT_DIR%\*.pyd") do (
    set PYD_FILE=%%f
    goto :found_pyd
)

:found_pyd
if "!PYD_FILE!"=="" (
    echo [ERROR] No .pyd file found in %OUTPUT_DIR%
    exit /b 1
)

echo [INFO] Found compiled module: !PYD_FILE!
echo.

REM Extract and prepare for distribution
echo [INFO] Preparing distribution package...
echo.

REM Copy the .pyd to a distributable location
set DIST_FILE=%OUTPUT_DIR%\mubaddil_core.pyd
copy /Y "!PYD_FILE!" "%DIST_FILE%" >nul

if errorlevel 1 (
    echo [ERROR] Failed to copy .pyd file
    exit /b 1
)

echo [INFO] Distribution file created: %DIST_FILE%
echo.

REM Display file information
echo =============================================================================
echo  Build Artifacts
echo =============================================================================
dir /B "%OUTPUT_DIR%\*.pyd"
echo.
for %%A in ("%DIST_FILE%") do (
    echo  File: %%~nxA
    echo  Size: %%~zA bytes
    echo  Path: %%~fA
)
echo.

REM Instructions for using the built module
echo =============================================================================
echo  Next Steps
echo =============================================================================
echo  1. Copy '%DIST_FILE%' to your Python project directory
echo  2. Ensure you have the required Python dependencies:
echo     pip install pyo3
echo  3. Import and use in Python:
echo     import mubaddil_core
echo     core = mubaddil_core.MubaddilCore()
echo     core.start()
echo.
echo  NOTE: This is a Python extension module (.pyd), not a standalone .exe.
echo        To create a standalone .exe, you would need to:
echo        - Create a Python entry point script
echo        - Use PyInstaller or similar tool to bundle Python + .pyd
echo.
echo =============================================================================

endlocal
exit /b 0
