#pragma once

#include <windows.h>
#include <functional>
#include <atomic>
#include <thread>

// تعريف نوع الدالة التي سيستدعيها الـ Hook عند الضغط على مفتاح
typedef void (*KeyEventCallback)(int vkCode, int scanCode, int flags, int time);

class KeyboardHook {
public:
    KeyboardHook();
    ~KeyboardHook();

    bool Install(KeyEventCallback callback);
    void Uninstall();
    bool IsInstalled() const;

private:
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static KeyEventCallback s_callback;
    static HHOOK s_hook;
    static std::atomic<bool> s_installed;
};