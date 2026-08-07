#pragma once

/**
 * @file config.h
 * @brief Application configuration management
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QVariant>
#include <QMap>
#include <QSettings>

namespace mubaddil {

/**
 * @class ConfigManager
 * @brief Manages application settings and preferences
 */
class ConfigManager {
public:
    ConfigManager();
    
    /**
     * @brief Get a configuration value
     * @param key Setting key
     * @param defaultValue Default value if not found
     * @return Configuration value
     */
    QVariant get(const QString& key, const QVariant& defaultValue = {}) const;
    
    /**
     * @brief Set a configuration value
     * @param key Setting key
     * @param value Value to set
     */
    void set(const QString& key, const QVariant& value);
    
    /**
     * @brief Get all settings
     * @return Map of all settings
     */
    QMap<QString, QVariant> all() const;
    
    // Common setting keys
    static const QString KEY_MONITORING_ENABLED;
    static const QString KEY_SHOW_SUGGESTIONS;
    static const QString KEY_AUTO_CORRECT;
    static const QString KEY_LANGUAGE_DIRECTION;
    static const QString KEY_MINIMIZE_TO_TRAY;
    static const QString KEY_START_MINIMIZED;
    static const QString KEY_THEME;
    
private:
    QSettings* m_settings;
    QMap<QString, QVariant> m_defaults;
    void loadDefaults();
};

} // namespace mubaddil

#endif // CONFIG_H
