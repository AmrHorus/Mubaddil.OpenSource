#include "config.h"

namespace mubaddil {

// Setting key constants
const QString ConfigManager::KEY_MONITORING_ENABLED = "monitoring/enabled";
const QString ConfigManager::KEY_SHOW_SUGGESTIONS = "suggestions/show";
const QString ConfigManager::KEY_AUTO_CORRECT = "correction/auto";
const QString ConfigManager::KEY_LANGUAGE_DIRECTION = "language/direction";
const QString ConfigManager::KEY_MINIMIZE_TO_TRAY = "ui/minimizeToTray";
const QString ConfigManager::KEY_START_MINIMIZED = "ui/startMinimized";
const QString ConfigManager::KEY_THEME = "ui/theme";

ConfigManager::ConfigManager() 
    : m_settings(new QSettings("Mubaddil", "Mubaddil", this)) {
    loadDefaults();
}

void ConfigManager::loadDefaults() {
    m_defaults[KEY_MONITORING_ENABLED] = true;
    m_defaults[KEY_SHOW_SUGGESTIONS] = true;
    m_defaults[KEY_AUTO_CORRECT] = false;
    m_defaults[KEY_LANGUAGE_DIRECTION] = "auto";
    m_defaults[KEY_MINIMIZE_TO_TRAY] = true;
    m_defaults[KEY_START_MINIMIZED] = false;
    m_defaults[KEY_THEME] = "dark";
}

QVariant ConfigManager::get(const QString& key, const QVariant& defaultValue) const {
    if (m_settings->contains(key)) {
        return m_settings->value(key);
    }
    if (m_defaults.contains(key)) {
        return m_defaults[key];
    }
    return defaultValue;
}

void ConfigManager::set(const QString& key, const QVariant& value) {
    m_settings->setValue(key, value);
    m_settings->sync();
}

QMap<QString, QVariant> ConfigManager::all() const {
    QMap<QString, QVariant> result = m_defaults;
    
    m_settings->beginGroup("");
    for (const auto& key : m_settings->childKeys()) {
        result[key] = m_settings->value(key);
    }
    m_settings->endGroup();
    
    return result;
}

} // namespace mubaddil
