# Mubaddil - Complete C++ Application
# Professional build system for Windows keyboard language switcher
cmake_minimum_required(VERSION 3.20)

project(Mubaddil
    VERSION 1.0.0
    DESCRIPTION "Mubaddil - Arabic/English Keyboard Language Switcher"
    LANGUAGES CXX
)

# C++20 Standard Required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt6 Auto-MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# MSVC Specific Settings
if(MSVC)
    # Enable all warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    
    # UTF-8 source and execution charset
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    
    # Enable optimizations for Release
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL")
    
    # Static runtime library option
    option(USE_STATIC_RUNTIME "Use static runtime library" ON)
    if(USE_STATIC_RUNTIME)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# Windows SDK
find_package(WINDOWSSDK REQUIRED)

# Core Engine Source Files
set(CORE_SOURCES
    hook/Keyboard_hook.cpp
    hook/buffer.cpp
    hook/mapper.cpp
    hook/detector.cpp
    hook/clipboard.cpp
    hook/replacement.cpp
    hook/logger.cpp
    hook/bridge.cpp
    hook/exports.cpp
)

# Core Engine Header Files
set(CORE_HEADERS
    hook/Keyboard_hook.h
    hook/buffer.h
    hook/mapper.h
    hook/detector.h
    hook/clipboard.h
    hook/replacement.h
    hook/logger.h
    hook/bridge.h
)

# UI Source Files
set(UI_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/dialogs.cpp
    src/systemtray.cpp
    src/theme.cpp
    src/config.cpp
    src/core_engine.cpp
    src/ui_helpers.cpp
)

# UI Header Files
set(UI_HEADERS
    src/mainwindow.h
    src/dialogs.h
    src/systemtray.h
    src/theme.h
    src/config.h
    src/core_engine.h
    src/ui_helpers.h
)

# Create Executable
add_executable(${PROJECT_NAME} WIN32 ${UI_SOURCES} ${UI_HEADERS} ${CORE_SOURCES} ${CORE_HEADERS})

# Include Directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/hook
)

# Link Libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        user32
        kernel32
        ole32
        shlwapi
)

# Compile Definitions
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        UNICODE
        _UNICODE
        QT_USE_QSTRINGBUILDER
    PUBLIC
        $<$<CONFIG:Debug>:DEBUG>
)

# Output Directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
    
    # Set output name
    OUTPUT_NAME "Mubaddil"
)

# Generate compile_commands.json for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print Configuration Summary
message(STATUS "")
message(STATUS "=== Mubaddil Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "")
