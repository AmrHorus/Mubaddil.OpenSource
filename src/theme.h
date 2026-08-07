#pragma once

/**
 * @file theme.h
 * @brief Application theme and styling
 */

#ifndef THEME_H
#define THEME_H

#include <QString>
#include <QPalette>
#include <QFont>

namespace mubaddil {

/**
 * @class Theme
 * @brief Provides application-wide theme colors and styles
 */
class Theme {
public:
    // Colors
    static const QString BG_DARK;
    static const QString BG_CARD;
    static const QString BG_CARD_ALT;
    static const QString ACCENT;
    static const QString ACCENT_LIGHT;
    static const QString TEXT_PRIMARY;
    static const QString TEXT_SECONDARY;
    static const QString TEXT_MUTED;
    static const QString SUCCESS;
    static const QString SUCCESS_LIGHT;
    static const QString ERROR;
    static const QString ERROR_LIGHT;
    static const QString WARNING;
    static const QString INFO;
    static const QString BORDER;
    
    /**
     * @brief Apply dark theme to application
     * @param app QApplication instance
     */
    static void applyDarkTheme();
    
    /**
     * @brief Get default font
     * @return QFont configured for the application
     */
    static QFont getDefaultFont();
    
    /**
     * @brief Create a dark palette
     * @return QPalette with dark theme colors
     */
    static QPalette createDarkPalette();
};

} // namespace mubaddil

#endif // THEME_H
