/**
 * @file logger.cpp
 * @brief Implementation of thread-safe logging
 */

#include "logger.h"
#include <sstream>
#include <iomanip>
#include <ctime>

namespace mubaddil {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

bool Logger::Initialize(const std::wstring& logPath, LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    m_fileStream = std::make_unique<std::wofstream>(logPath, std::ios::app);
    if (!m_fileStream->is_open()) {
        m_fileStream.reset();
        return false;
    }
    
    m_logLevel = level;
    m_initialized = true;
    
    Info(L"Logger initialized", L"Initialize");
    return true;
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized && m_fileStream) {
        Info(L"Logger shutting down", L"Shutdown");
        m_fileStream->close();
        m_fileStream.reset();
    }
    
    m_initialized = false;
}

void Logger::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logLevel = level;
}

void Logger::SetCallback(std::function<void(LogLevel, const std::wstring&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_callback = std::move(callback);
}

void Logger::Log(LogLevel level, const std::wstring& message, const std::wstring& source) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (level < m_logLevel || !m_initialized) {
        return;
    }
    
    std::wstringstream ss;
    ss << GetTimestamp() 
       << L" [" << LevelToString(level) << L"] ";
    
    if (!source.empty()) {
        ss << L"[" << source << L"] ";
    }
    
    ss << message;
    
    std::wstring logLine = ss.str();
    
    // Write to file
    if (m_fileStream && m_fileStream->is_open()) {
        *m_fileStream << logLine << std::endl;
        m_fileStream->flush();
    }
    
    // Output to debug console
    OutputDebugStringW((logLine + L"\n").c_str());
    
    // Invoke callback if set
    if (m_callback) {
        // Release lock before calling callback to prevent deadlock
        auto callbackCopy = m_callback;
        lock.~lock_guard();
        callbackCopy(level, logLine);
        std::construct_at(&lock, m_mutex);
    }
}

const wchar_t* Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return L"DEBUG";
        case LogLevel::Info:    return L"INFO";
        case LogLevel::Warning: return L"WARN";
        case LogLevel::Error:   return L"ERROR";
        case LogLevel::Fatal:   return L"FATAL";
        default:                return L"UNKNOWN";
    }
}

std::wstring Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::wstringstream ss;
    ss << std::put_time(std::localtime(&time), L"%Y-%m-%d %H:%M:%S");
    ss << L"." << std::setfill(L'0') << std::setw(3) << ms.count();
    
    return ss.str();
}

Logger::~Logger() {
    Shutdown();
}

void Logger::Debug(const std::wstring& message, const std::wstring& source) {
    Log(LogLevel::Debug, message, source);
}

void Logger::Info(const std::wstring& message, const std::wstring& source) {
    Log(LogLevel::Info, message, source);
}

void Logger::Warning(const std::wstring& message, const std::wstring& source) {
    Log(LogLevel::Warning, message, source);
}

void Logger::Error(const std::wstring& message, const std::wstring& source) {
    Log(LogLevel::Error, message, source);
}

void Logger::Fatal(const std::wstring& message, const std::wstring& source) {
    Log(LogLevel::Fatal, message, source);
}

} // namespace mubaddil
