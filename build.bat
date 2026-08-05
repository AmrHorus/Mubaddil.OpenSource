@echo off
setlocal EnableDelayedExpansion
title Mubaddil Build

echo ==========================================
echo          Mubaddil Build System
echo ==========================================
echo.

:: -------------------------------------------------
:: Check Python
:: -------------------------------------------------
where python >nul 2>&1
if errorlevel 1 (
    echo [ERROR] Python not found.
    pause
    exit /b 1
)

:: -------------------------------------------------
:: Create virtual environment
:: -------------------------------------------------
if not exist ".venv" (
    echo Creating virtual environment...
    python -m venv .venv
)

call .venv\Scripts\activate

:: -------------------------------------------------
:: Upgrade pip
:: -------------------------------------------------
python -m pip install --upgrade pip

:: -------------------------------------------------
:: Install dependencies
:: -------------------------------------------------
echo Installing Python packages...
pip install -r requirements.txt

pip install pyinstaller

:: -------------------------------------------------
:: Build C++ Hook
:: -------------------------------------------------
echo.
echo Building native hook...

if not exist hook (
    echo [WARNING] hook folder not found.
) else (

    if not exist hook\build (
        mkdir hook\build
    )

    pushd hook\build

    cmake ..

    if errorlevel 1 (
        echo.
        echo CMake configuration failed.
        pause
        exit /b 1
    )

    cmake --build . --config Release

    if errorlevel 1 (
        echo.
        echo Native build failed.
        pause
        exit /b 1
    )

    popd
)

:: -------------------------------------------------
:: Clean
:: -------------------------------------------------
echo.
echo Cleaning previous builds...

if exist build rmdir /s /q build
if exist dist rmdir /s /q dist
if exist __pycache__ rmdir /s /q __pycache__

del /q *.spec >nul 2>&1

:: -------------------------------------------------
:: Detect native DLL
:: -------------------------------------------------
set DLL=

for %%F in (
    hook\build\Release\*.dll
    hook\build\*.dll
) do (
    if exist "%%F" (
        set DLL=%%F
        goto dll_found
    )
)

:dll_found

if defined DLL (
    echo Found native library:
    echo %DLL%

    pyinstaller ^
        --clean ^
        --noconfirm ^
        --onefile ^
        --windowed ^
        --name Mubaddil ^
        --icon mubaddil.ico ^
        --add-data "mubaddil.ico;." ^
        --add-data "%DLL%;." ^
        main.py
) else (
    echo No native DLL found.
    echo Building Python-only package...

    pyinstaller ^
        --clean ^
        --noconfirm ^
        --onefile ^
        --windowed ^
        --name Mubaddil ^
        --icon mubaddil.ico ^
        --add-data "mubaddil.ico;." ^
        main.py
)

:: -------------------------------------------------
:: Finished
:: -------------------------------------------------
if exist dist\Mubaddil.exe (
    echo.
    echo ==========================================
    echo Build Successful!
    echo.
    echo EXE:
    echo %CD%\dist\Mubaddil.exe
    echo ==========================================
) else (
    echo.
    echo Build failed.
)

pause
