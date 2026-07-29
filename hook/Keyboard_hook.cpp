#include "keyboard_hook.h"
#include <iostream>

// تعريف المتغيرات الثابتة
HHOOK KeyboardHook::s_hook = nullptr;
KeyEventCallback KeyboardHook::s_callback = nullptr;
std::atomic<bool> KeyboardHook::s_installed{false};

KeyboardHook::KeyboardHook() {}

KeyboardHook::~KeyboardHook() {
    Uninstall();
}

bool KeyboardHook::Install(KeyEventCallback callback) {
    if (s_installed) return true;

    s_callback = callback;

    // تثبيت الـ Hook على مستوى النظام (Low-Level)
    s_hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookProc, GetModuleHandle(nullptr), 0);
    if (!s_hook) {
        std::cerr << "SetWindowsHookEx failed! Error: " << GetLastError() << std::endl;
        return false;
    }

    s_installed = true;
    return true;
}

void KeyboardHook::Uninstall() {
    if (s_hook) {
        UnhookWindowsHookEx(s_hook);
        s_hook = nullptr;
    }
    s_installed = false;
    s_callback = nullptr;
}

bool KeyboardHook::IsInstalled() const {
    return s_installed;
}

// دالة الـ Hook الثابتة
LRESULT CALLBACK KeyboardHook::HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            KBDLLHOOKSTRUCT* p = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
            if (s_callback) {
                // استدعاء دالة الـ Callback المسجلة من Python
                s_callback(p->vkCode, p->scanCode, p->flags, p->time);
            }
        }
    }
    return CallNextHookEx(s_hook, nCode, wParam, lParam);
}

// دوال التصدير للاستخدام من Python
extern "C" {

    __declspec(dllexport) bool InstallHook(KeyEventCallback callback) {
        KeyboardHook hook;
        return hook.Install(callback);
    }

    __declspec(dllexport) void UninstallHook() {
        KeyboardHook hook;
        hook.Uninstall();
    }

    __declspec(dllexport) bool IsHookInstalled() {
        KeyboardHook hook;
        return hook.IsInstalled();
    }
}
