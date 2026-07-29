/**
 * @file bridge.cpp
 * @brief Implementation of Python-C++ communication bridge
 */

#include "bridge.h"
#include "Keyboard_hook.h"
#include "buffer.h"
#include "mapper.h"
#include "detector.h"
#include "replacement.h"
#include "logger.h"
#include <chrono>

namespace mubaddil {

Bridge& Bridge::Instance() {
    static Bridge instance;
    return instance;
}

bool Bridge::Initialize(const std::wstring& logPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    // Initialize logger
    if (!logPath.empty()) {
        Logger::Instance().Initialize(logPath, LogLevel::Info);
    }
    
    LOG_INFO(L"Mubaddil Engine initializing...");
    
    // Initialize subsystems (they're singletons, so just access them)
    KeyboardMapper::Instance();
    LanguageDetector::Instance();
    ReplacementEngine::Instance();
    ClipboardManager::Instance();
    
    m_enabled = false;
    m_initialized = true;
    
    LOG_INFO(L"Mubaddil Engine initialized successfully");
    return true;
}

void Bridge::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    LOG_INFO(L"Mubaddil Engine shutting down...");
    
    // Stop hook first
    StopHook();
    
    // Shutdown logger
    Logger::Instance().Shutdown();
    
    m_initialized = false;
    
    LOG_INFO(L"Mubaddil Engine shutdown complete");
}

bool Bridge::StartHook() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        LOG_ERROR(L"Cannot start hook - not initialized");
        return false;
    }
    
    auto& hook = KeyboardHook::Instance();
    
    // Set up the key event callback
    bool result = hook.Install([this](int vkCode, int scanCode, int flags, DWORD time) {
        OnKeyEvent(vkCode, scanCode, flags, time);
    });
    
    if (result) {
        LOG_INFO(L"Keyboard hook started");
    } else {
        LOG_ERROR(L"Failed to install keyboard hook");
    }
    
    return result;
}

void Bridge::StopHook() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    KeyboardHook::Instance().Uninstall();
    LOG_INFO(L"Keyboard hook stopped");
}

bool Bridge::IsHookActive() const noexcept {
    return KeyboardHook::Instance().IsInstalled();
}

void Bridge::Enable() {
    m_enabled = true;
    LOG_INFO(L"Monitoring enabled");
}

void Bridge::Disable() {
    m_enabled = false;
    LOG_INFO(L"Monitoring disabled");
}

bool Bridge::IsEnabled() const noexcept {
    return m_enabled.load(std::memory_order_relaxed);
}

bool Bridge::ReplaceWord(const std::wstring& original,
                          const std::wstring& replacement,
                          HWND hwnd) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    bool result = ReplacementEngine::Instance().ReplaceWord(original, replacement, hwnd);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    // Update average replacement time (simple moving average)
    double newAvg = m_statistics.avgReplacementTimeMs;
    if (m_statistics.totalCorrections > 0) {
        newAvg = (newAvg * m_statistics.totalCorrections + duration.count() / 1000.0) 
                 / (m_statistics.totalCorrections + 1);
    } else {
        newAvg = duration.count() / 1000.0;
    }
    m_statistics.avgReplacementTimeMs = newAvg;
    
    if (result) {
        m_statistics.totalCorrections++;
        LOG_INFO(L"Word replaced: " + original + L" -> " + replacement);
    }
    
    return result;
}

const wchar_t* Bridge::GetVersion() noexcept {
    return L"1.0.0";
}

Statistics Bridge::GetStatistics() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_statistics;
}

void Bridge::SetCorrectionCallback(std::function<void(const CorrectionEvent&)> callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_correctionCallback = std::move(callback);
}

void Bridge::RecordAcceptance(bool isEnToAr) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_statistics.suggestionsAccepted++;
    if (isEnToAr) {
        m_statistics.enToArCorrections++;
    } else {
        m_statistics.arToEnCorrections++;
    }
}

void Bridge::RecordRejection() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_statistics.suggestionsRejected++;
}

Bridge::~Bridge() {
    Shutdown();
}

// Internal key event handler
void Bridge::OnKeyEvent(int vkCode, int scanCode, int flags, DWORD time) {
    if (!m_enabled) {
        return;
    }
    
    m_statistics.keysProcessed++;
    
    // Process through buffer
    KeyEvent event(vkCode, scanCode, flags, time);
    
    // Simple detection logic - in production this would be more sophisticated
    auto& detector = LanguageDetector::Instance();
    auto& mapper = KeyboardMapper::Instance();
    auto& replacement = ReplacementEngine::Instance();
    
    // Example: detect when space is pressed after typing
    if (vkCode == VK_SPACE) {
        // Get current word from buffer (would need proper buffer integration)
        // For now, this is a placeholder for the full implementation
    }
}

} // namespace mubaddil

// C-style exports implementation
extern "C" {

__declspec(dllexport) bool Mubaddil_Initialize(const wchar_t* logPath) {
    std::wstring path = logPath ? logPath : L"";
    return mubaddil::Bridge::Instance().Initialize(path);
}

__declspec(dllexport) void Mubaddil_Shutdown() {
    mubaddil::Bridge::Instance().Shutdown();
}

__declspec(dllexport) bool Mubaddil_StartHook() {
    return mubaddil::Bridge::Instance().StartHook();
}

__declspec(dllexport) void Mubaddil_StopHook() {
    mubaddil::Bridge::Instance().StopHook();
}

__declspec(dllexport) bool Mubaddil_IsHookActive() {
    return mubaddil::Bridge::Instance().IsHookActive();
}

__declspec(dllexport) void Mubaddil_Enable() {
    mubaddil::Bridge::Instance().Enable();
}

__declspec(dllexport) void Mubaddil_Disable() {
    mubaddil::Bridge::Instance().Disable();
}

__declspec(dllexport) bool Mubaddil_IsEnabled() {
    return mubaddil::Bridge::Instance().IsEnabled();
}

__declspec(dllexport) bool Mubaddil_ReplaceWord(const wchar_t* original,
                                                 const wchar_t* replacement,
                                                 HWND hwnd) {
    if (!original || !replacement) {
        return false;
    }
    return mubaddil::Bridge::Instance().ReplaceWord(original, replacement, hwnd);
}

__declspec(dllexport) const wchar_t* Mubaddil_GetVersion() {
    return mubaddil::Bridge::GetVersion();
}

__declspec(dllexport) uint64_t Mubaddil_GetTotalCorrections() {
    return mubaddil::Bridge::Instance().GetStatistics().totalCorrections;
}

__declspec(dllexport) uint64_t Mubaddil_GetEnToArCorrections() {
    return mubaddil::Bridge::Instance().GetStatistics().enToArCorrections;
}

__declspec(dllexport) uint64_t Mubaddil_GetArToEnCorrections() {
    return mubaddil::Bridge::Instance().GetStatistics().arToEnCorrections;
}

}
