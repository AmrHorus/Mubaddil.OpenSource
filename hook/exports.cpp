# Mubaddil - exports.cpp
# Python bridge exports for ctypes integration

#include "bridge.h"

/**
 * @file exports.cpp
 * @brief C-style exports for Python ctypes integration
 * 
 * This file provides a clean C API that can be called from Python
 * using ctypes without requiring pybind11.
 */

extern "C" {

/**
 * @brief Initialize the Mubaddil engine
 * @param logPath Path to log file (can be nullptr)
 * @return true if successful
 */
__declspec(dllexport) bool Mubaddil_Initialize(const wchar_t* logPath) {
    return mubaddil::Bridge::Instance().Initialize(logPath ? logPath : L"");
}

/**
 * @brief Shutdown the Mubaddil engine
 */
__declspec(dllexport) void Mubaddil_Shutdown() {
    mubaddil::Bridge::Instance().Shutdown();
}

/**
 * @brief Start the keyboard hook
 * @return true if successful
 */
__declspec(dllexport) bool Mubaddil_StartHook() {
    return mubaddil::Bridge::Instance().StartHook();
}

/**
 * @brief Stop the keyboard hook
 */
__declspec(dllexport) void Mubaddil_StopHook() {
    mubaddil::Bridge::Instance().StopHook();
}

/**
 * @brief Check if hook is currently active
 * @return true if hook is installed
 */
__declspec(dllexport) bool Mubaddil_IsHookActive() {
    return mubaddil::Bridge::Instance().IsHookActive();
}

/**
 * @brief Enable monitoring
 */
__declspec(dllexport) void Mubaddil_Enable() {
    mubaddil::Bridge::Instance().Enable();
}

/**
 * @brief Disable monitoring
 */
__declspec(dllexport) void Mubaddil_Disable() {
    mubaddil::Bridge::Instance().Disable();
}

/**
 * @brief Check if monitoring is enabled
 * @return true if enabled
 */
__declspec(dllexport) bool Mubaddil_IsEnabled() {
    return mubaddil::Bridge::Instance().IsEnabled();
}

/**
 * @brief Replace a word in the target window
 * @param original The word to replace
 * @param replacement The replacement word
 * @param hwnd Target window handle
 * @return true if successful
 */
__declspec(dllexport) bool Mubaddil_ReplaceWord(const wchar_t* original,
                                                 const wchar_t* replacement,
                                                 HWND hwnd) {
    if (!original || !replacement) {
        return false;
    }
    return mubaddil::Bridge::Instance().ReplaceWord(original, replacement, hwnd);
}

/**
 * @brief Get version string
 * @return Version as wide string
 */
__declspec(dllexport) const wchar_t* Mubaddil_GetVersion() {
    return mubaddil::Bridge::GetVersion();
}

/**
 * @brief Get total number of corrections
 * @return Total corrections count
 */
__declspec(dllexport) uint64_t Mubaddil_GetTotalCorrections() {
    return mubaddil::Bridge::Instance().GetStatistics().totalCorrections;
}

/**
 * @brief Get English to Arabic corrections count
 * @return EN->AR corrections count
 */
__declspec(dllexport) uint64_t Mubaddil_GetEnToArCorrections() {
    return mubaddil::Bridge::Instance().GetStatistics().enToArCorrections;
}

/**
 * @brief Get Arabic to English corrections count
 * @return AR->EN corrections count
 */
__declspec(dllexport) uint64_t Mubaddil_GetArToEnCorrections() {
    return mubaddil::Bridge::Instance().GetStatistics().arToEnCorrections;
}

/**
 * @brief Get suggestions shown count
 * @return Suggestions shown count
 */
__declspec(dllexport) uint64_t Mubaddil_GetSuggestionsShown() {
    return mubaddil::Bridge::Instance().GetStatistics().suggestionsShown;
}

/**
 * @brief Get suggestions accepted count
 * @return Suggestions accepted count
 */
__declspec(dllexport) uint64_t Mubaddil_GetSuggestionsAccepted() {
    return mubaddil::Bridge::Instance().GetStatistics().suggestionsAccepted;
}

/**
 * @brief Get suggestions rejected count
 * @return Suggestions rejected count
 */
__declspec(dllexport) uint64_t Mubaddil_GetSuggestionsRejected() {
    return mubaddil::Bridge::Instance().GetStatistics().suggestionsRejected;
}

/**
 * @brief Get keys processed count
 * @return Keys processed count
 */
__declspec(dllexport) uint64_t Mubaddil_GetKeysProcessed() {
    return mubaddil::Bridge::Instance().GetStatistics().keysProcessed;
}

/**
 * @brief Record an accepted suggestion
 * @param isEnToAr true if English to Arabic conversion
 */
__declspec(dllexport) void Mubaddil_RecordAcceptance(int isEnToAr) {
    mubaddil::Bridge::Instance().RecordAcceptance(isEnToAr != 0);
}

/**
 * @brief Record a rejected suggestion
 */
__declspec(dllexport) void Mubaddil_RecordRejection() {
    mubaddil::Bridge::Instance().RecordRejection();
}

} // extern "C"
