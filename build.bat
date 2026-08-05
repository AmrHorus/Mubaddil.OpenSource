@echo off
setlocal enabledelayedexpansion

title Mubaddil Build System

echo =========================================
echo        Mubaddil EXE Build Script
echo =========================================
echo.

REM -------------------------------
REM Configuration
REM -------------------------------

set APP_NAME=Mubaddil
set MAIN_SCRIPT=main.py
set BUILD_DIR=build
set DIST_DIR=dist

REM Change this if your main file is different
if not exist "%MAIN_SCRIPT%" (
    echo ERROR: %MAIN_SCRIPT% not found.
    echo Please edit MAIN_SCRIPT in build.bat
    pause
    exit /b 1
)


REM -------------------------------
REM Check Python
REM -------------------------------

echo Checking Python...

python --version >nul 2>&1

if errorlevel 1 (
    echo ERROR: Python not installed.
    pause
    exit /b 1
)

echo Python OK


REM -------------------------------
REM Create Virtual Environment
REM -------------------------------

if not exist ".venv" (
    echo Creating virtual environment...
    python -m venv .venv
)

call .venv\Scripts\activate


REM -------------------------------
REM Upgrade tools
REM -------------------------------

echo Updating build tools...

python -m pip install --upgrade pip
python -m pip install --upgrade setuptools wheel


REM -------------------------------
REM Install requirements
REM -------------------------------

if exist requirements.txt (
    echo Installing requirements...
    pip install -r requirements.txt
) else (
    echo WARNING: requirements.txt missing
)


REM -------------------------------
REM Build Native Engine (optional)
REM -------------------------------

if exist Native (
    echo.
    echo Native engine detected.

    where cmake >nul 2>&1

    if errorlevel 1 (
        echo WARNING: CMake not installed.
        echo Skipping native build.
    ) else (

        echo Building C++ engine...

        if not exist Native\build (
            mkdir Native\build
        )

        cd Native\build

        cmake ..
        cmake --build . --config Release

        cd ..\..
    )
)


REM -------------------------------
REM Install PyInstaller
REM -------------------------------

echo Installing PyInstaller...

pip install pyinstaller


REM -------------------------------
REM Clean old builds
REM -------------------------------

echo Cleaning old builds...

rmdir /s /q "%BUILD_DIR%" 2>nul
rmdir /s /q "%DIST_DIR%" 2>nul


REM -------------------------------
REM PyInstaller Build
REM -------------------------------

echo Building EXE...

pyinstaller ^
 --name "%APP_NAME%" ^
 --noconfirm ^
 --clean ^
 --windowed ^
 --onefile ^
 --icon assets\icon.ico ^
 --add-data "assets;assets" ^
 "%MAIN_SCRIPT%"


if errorlevel 1 (
    echo.
    echo BUILD FAILED
    pause
    exit /b 1
)


REM -------------------------------
REM Copy Native DLLs
REM -------------------------------

if exist Native\build\Release (
    echo Copying native DLLs...

    copy Native\build\Release\*.dll dist\ >nul
)


REM -------------------------------
REM Finished
REM -------------------------------

echo.
echo =========================================
echo BUILD COMPLETE
echo.
echo EXE location:
echo dist\%APP_NAME%.exe
echo =========================================

pause
