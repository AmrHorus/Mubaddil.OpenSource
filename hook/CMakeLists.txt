# Mubaddil Native Core - CMake Configuration
# Professional build system for Windows keyboard hook engine
cmake_minimum_required(VERSION 3.20)

project(MubaddilCore
    VERSION 1.0.0
    DESCRIPTION "Mubaddil Native Keyboard Engine"
    LANGUAGES CXX
)

# C++20 Standard Required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    
    # Disable specific warnings that are too noisy
    add_compile_options(/wd4267)  # Conversion warning
    add_compile_options(/wd4244)  # Conversion warning
endif()

# Windows SDK
find_package(WINDOWSSDK REQUIRED)

# Source Files
set(SOURCES
    Keyboard_hook.cpp
    buffer.cpp
    mapper.cpp
    detector.cpp
    clipboard.cpp
    replacement.cpp
    logger.cpp
    bridge.cpp
    exports.cpp
    language.cpp
    popup.cpp
)

# Header Files
set(HEADERS
    Keyboard_hook.h
    buffer.h
    mapper.h
    detector.h
    clipboard.h
    replacement.h
    logger.h
    bridge.h
    language.h
    popup.h
)

# Create Shared Library (DLL)
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Include Directories
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Link Libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        user32
        kernel32
        ole32
        dwmapi
        uxtheme
)

# Compile Definitions
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        UNICODE
        _UNICODE
        MUBADDIL_CORE_EXPORTS
    PUBLIC
        $<$<CONFIG:Debug>:DEBUG>
)

# Export Symbols
set_target_properties(${PROJECT_NAME} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)

# Output Directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/bin/RelWithDebInfo"
)

# Install Rules
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    EXPORT MubaddilCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mubaddil
)

# Generate compile_commands.json for IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Print Configuration Summary
message(STATUS "")
message(STATUS "=== Mubaddil Core Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "MSVC Runtime: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
