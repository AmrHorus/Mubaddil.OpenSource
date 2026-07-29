#pragma once

/**
 * @file logger.h
 * @brief Thread-safe logging facility
 * 
 * Provides structured logging with file output support,
 * log levels, and automatic rotation.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>
#include <string>
#include <fstream>
#include <mutex>
#include <memory>
#include <functional>

namespace mubaddil {

/**
 * @enum LogLevel
 * @brief Severity levels for log messages
 */
enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Fatal = 4
};

/**
 * @class Logger
 * @brief Thread-safe singleton logger with file output
 * 
 * Provides centralized logging with configurable log levels,
 * file output, and automatic timestamp formatting.
 */
class Logger {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to singleton instance
     */
    static Logger& Instance();
    
    /**
     * @brief Initialize the logger with a log file path
     * @param logPath Path to the log file
     * @param level Minimum log level to record
     * @return true if initialization successful
     */
    bool Initialize(const std::wstring& logPath, LogLevel level = LogLevel::Info);
    
    /**
     * @brief Shutdown the logger and close file handles
     */
    void Shutdown();
    
    /**
     * @brief Set minimum log level
     * @param level Minimum level to log
     */
    void SetLogLevel(LogLevel level);
    
    /**
     * @brief Log a debug message
     * @param message Message to log
     * @param source Source file/function
     */
    void Debug(const std::wstring& message, const std::wstring& source = L"");
    
    /**
     * @brief Log an info message
     * @param message Message to log
     * @param source Source file/function
     */
    void Info(const std::wstring& message, const std::wstring& source = L"");
    
    /**
     * @brief Log a warning message
     * @param message Message to log
     * @param source Source file/function
     */
    void Warning(const std::wstring& message, const std::wstring& source = L"");
    
    /**
     * @brief Log an error message
     * @param message Message to log
     * @param source Source file/function
     */
    void Error(const std::wstring& message, const std::wstring& source = L"");
    
    /**
     * @brief Log a fatal message
     * @param message Message to log
     * @param source Source file/function
     */
    void Fatal(const std::wstring& message, const std::wstring& source = L"");
    
    /**
     * @brief Set callback for log events
     * @param callback Function to call on each log entry
     */
    void SetCallback(std::function<void(LogLevel, const std::wstring&)> callback);
    
    // Delete copy/move operations
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;
    
private:
    Logger() = default;
    ~Logger();
    
    /**
     * @brief Internal log method
     * @param level Log level
     * @param message Message content
     * @param source Source identifier
     */
    void Log(LogLevel level, const std::wstring& message, const std::wstring& source);
    
    /**
     * @brief Get string representation of log level
     * @param level Log level
     * @return Level as string
     */
    static const wchar_t* LevelToString(LogLevel level);
    
    /**
     * @brief Get current timestamp string
     * @return Formatted timestamp
     */
    static std::wstring GetTimestamp();
    
    mutable std::mutex m_mutex;                              ///< Thread synchronization
    std::unique_ptr<std::wofstream> m_fileStream;            ///< Log file stream
    LogLevel m_logLevel{LogLevel::Info};                     ///< Current log level
    bool m_initialized{false};                               ///< Initialization flag
    std::function<void(LogLevel, const std::wstring&)> m_callback;  ///< Log callback
};

// Convenience macros
#define LOG_DEBUG(msg) mubaddil::Logger::Instance().Debug(msg, L__FUNCTIONW__)
#define LOG_INFO(msg) mubaddil::Logger::Instance().Info(msg, L__FUNCTIONW__)
#define LOG_WARNING(msg) mubaddil::Logger::Instance().Warning(msg, L__FUNCTIONW__)
#define LOG_ERROR(msg) mubaddil::Logger::Instance().Error(msg, L__FUNCTIONW__)
#define LOG_FATAL(msg) mubaddil::Logger::Instance().Fatal(msg, L__FUNCTIONW__)

} // namespace mubaddil

#endif // LOGGER_H
