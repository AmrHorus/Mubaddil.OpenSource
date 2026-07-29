/**
 * @file language.cpp
 * @brief Implementation of language and keyboard layout management
 */

#include "language.h"
#include <algorithm>

namespace mubaddil {

LanguageManager::LanguageManager()
    : m_hklEnglish(nullptr)
    , m_hklArabic(nullptr)
    , m_switchCount(0)
    , m_initialized(false)
{
}

LanguageManager::~LanguageManager() {
    Shutdown();
}

LanguageManager& LanguageManager::Instance() {
    static LanguageManager instance;
    return instance;
}

bool LanguageManager::Initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true; // Already initialized
    }
    
    // Load English US keyboard layout
    m_hklEnglish = LoadKeyboardLayoutA("00000409", KLF_ACTIVATE);
    
    // Load Arabic keyboard layout
    m_hklArabic = LoadKeyboardLayoutA("00000401", KLF_ACTIVATE);
    
    m_initialized = (m_hklEnglish != nullptr || m_hklArabic != nullptr);
    
    return m_initialized;
}

void LanguageManager::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_hklEnglish) {
        UnloadKeyboardLayout(static_cast<HKL>(m_hklEnglish));
        m_hklEnglish = nullptr;
    }
    
    if (m_hklArabic) {
        UnloadKeyboardLayout(static_cast<HKL>(m_hklArabic));
        m_hklArabic = nullptr;
    }
    
    m_initialized = false;
}

LayoutInfo LanguageManager::GetCurrentLayout(HWND hwnd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    LayoutInfo info;
    
    HWND targetWnd = GetTargetWindow(hwnd);
    if (!targetWnd) {
        return info;
    }
    
    info.handle = GetKeyboardLayout(targetWnd);
    info.isActive = true;
    
    if (info.handle) {
        // Extract language ID from HKL
        unsigned long langId = LOWORD(reinterpret_cast<uintptr_t>(info.handle));
        
        if (langId == 0x0409) {
            info.languageId = KeyboardLayoutId::EnglishUS;
            info.name = L"English (US)";
        } else if (langId == 0x0401) {
            info.languageId = KeyboardLayoutId::Arabic;
            info.name = L"Arabic";
        } else {
            info.languageId = KeyboardLayoutId::Unknown;
            info.name = L"Unknown";
        }
    }
    
    return info;
}

bool LanguageManager::SetLayout(KeyboardLayoutId layoutId, HWND hwnd) {
    void* hkl = GetLayoutHandle(layoutId);
    if (!hkl) {
        return false;
    }
    
    HWND targetWnd = GetTargetWindow(hwnd);
    if (!targetWnd) {
        return false;
    }
    
    HKL result = ActivateKeyboardLayout(static_cast<HKL>(hkl), KLF_SETFORPROCESS);
    
    if (result) {
        m_switchCount++;
        return true;
    }
    
    return false;
}

bool LanguageManager::SwitchToEnglish(HWND hwnd) {
    return SetLayout(KeyboardLayoutId::EnglishUS, hwnd);
}

bool LanguageManager::SwitchToArabic(HWND hwnd) {
    return SetLayout(KeyboardLayoutId::Arabic, hwnd);
}

bool LanguageManager::IsEnglishActive(HWND hwnd) {
    return GetCurrentLanguageId(hwnd) == 0x0409;
}

bool LanguageManager::IsArabicActive(HWND hwnd) {
    return GetCurrentLanguageId(hwnd) == 0x0401;
}

unsigned long LanguageManager::GetCurrentLanguageId(HWND hwnd) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    HWND targetWnd = GetTargetWindow(hwnd);
    if (!targetWnd) {
        return 0;
    }
    
    void* hkl = GetKeyboardLayout(targetWnd);
    if (hkl) {
        return LOWORD(reinterpret_cast<uintptr_t>(hkl));
    }
    
    return 0;
}

bool LanguageManager::LoadLayout(KeyboardLayoutId layoutId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    char layoutName[16];
    snprintf(layoutName, sizeof(layoutName), "%08lu", static_cast<unsigned long>(layoutId));
    
    void* hkl = LoadKeyboardLayoutA(layoutName, 0);
    
    if (hkl) {
        if (layoutId == KeyboardLayoutId::EnglishUS) {
            m_hklEnglish = hkl;
        } else if (layoutId == KeyboardLayoutId::Arabic) {
            m_hklArabic = hkl;
        }
        return true;
    }
    
    return false;
}

bool LanguageManager::UnloadLayout(KeyboardLayoutId layoutId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    void** pHkl = nullptr;
    if (layoutId == KeyboardLayoutId::EnglishUS) {
        pHkl = &m_hklEnglish;
    } else if (layoutId == KeyboardLayoutId::Arabic) {
        pHkl = &m_hklArabic;
    }
    
    if (pHkl && *pHkl) {
        UnloadKeyboardLayout(static_cast<HKL>(*pHkl));
        *pHkl = nullptr;
        return true;
    }
    
    return false;
}

bool LanguageManager::IsLayoutAvailable(KeyboardLayoutId layoutId) {
    // Check if we have the handle loaded
    void* hkl = GetLayoutHandle(layoutId);
    if (hkl) {
        return true;
    }
    
    // Try to load it temporarily
    return LoadLayout(layoutId);
}

size_t LanguageManager::GetSwitchCount() const {
    return m_switchCount.load();
}

void LanguageManager::ResetSwitchCount() {
    m_switchCount = 0;
}

void* LanguageManager::GetLayoutHandle(KeyboardLayoutId layoutId) {
    // Note: This is called with lock already held in most cases
    if (layoutId == KeyboardLayoutId::EnglishUS) {
        if (!m_hklEnglish) {
            LoadLayout(KeyboardLayoutId::EnglishUS);
        }
        return m_hklEnglish;
    } else if (layoutId == KeyboardLayoutId::Arabic) {
        if (!m_hklArabic) {
            LoadLayout(KeyboardLayoutId::Arabic);
        }
        return m_hklArabic;
    }
    
    return nullptr;
}

HWND LanguageManager::GetTargetWindow(HWND hwnd) {
    if (hwnd) {
        return hwnd;
    }
    
    return GetForegroundWindow();
}

} // namespace mubaddil
