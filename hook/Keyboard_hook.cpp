/**
 * @file Keyboard_hook.cpp
 * @brief Implementation of low-level Windows keyboard hook
 */

#include "Keyboard_hook.h"
#include <iostream>

namespace mubaddil {

// Static hook procedure forward declaration
LRESULT CALLBACK KeyboardHook::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            
            // Get singleton instance and process the key event
            auto& instance = KeyboardHook::Instance();
            instance.ProcessKeyEvent(p->vkCode, p->scanCode, p->flags, p->time);
            
            // Update foreground window for context
            instance.SetForegroundWindow(::GetForegroundWindow());
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

KeyboardHook& KeyboardHook::Instance() {
    static KeyboardHook instance;
    return instance;
}

KeyboardHook::~KeyboardHook() {
    Uninstall();
}

bool KeyboardHook::Install(KeyEventCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_installed) {
        return true;
    }

    m_callback = std::move(callback);

    // Install low-level keyboard hook
    m_hook = SetWindowsHookExW(WH_KEYBOARD_LL, HookProc, nullptr, 0);
    if (!m_hook) {
        std::cerr << "[KeyboardHook] SetWindowsHookEx failed! Error: " 
                  << GetLastError() << std::endl;
        return false;
    }

    m_installed = true;
    std::cout << "[KeyboardHook] Hook installed successfully." << std::endl;
    return true;
}

void KeyboardHook::Uninstall() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
        std::cout << "[KeyboardHook] Hook uninstalled." << std::endl;
    }
    
    m_installed = false;
    m_callback = nullptr;
}

bool KeyboardHook::IsInstalled() const noexcept {
    return m_installed.load(std::memory_order_relaxed);
}

void KeyboardHook::ProcessKeyEvent(int vkCode, int scanCode, int flags, DWORD time) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_callback) {
        m_callback(vkCode, scanCode, flags, time);
    }
}

void KeyboardHook::SetForegroundWindow(HWND hwnd) noexcept {
    m_foregroundHwnd = hwnd;
}

HWND KeyboardHook::GetForegroundWindow() const noexcept {
    return m_foregroundHwnd;
}

} // namespace mubaddil

// C-style exports for Python bridge
extern "C" {

__declspec(dllexport) bool Mubaddil_Initialize() {
    return true;
}

__declspec(dllexport) bool Mubaddil_StartHook() {
    using namespace mubaddil;
    return KeyboardHook::Instance().Install([](int vkCode, int scanCode, int flags, DWORD time) {
        // Default callback - can be overridden by Python bridge
    });
}

__declspec(dllexport) void Mubaddil_StopHook() {
    using namespace mubaddil;
    KeyboardHook::Instance().Uninstall();
}

__declspec(dllexport) bool Mubaddil_IsHookInstalled() {
    using namespace mubaddil;
    return KeyboardHook::Instance().IsInstalled();
}

__declspec(dllexport) const char* Mubaddil_GetVersion() {
    return "1.0.0";
}

}
