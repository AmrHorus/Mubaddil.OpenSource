#include "theme.h"
#include <QApplication>

namespace mubaddil {

// Color definitions
const QString Theme::BG_DARK = "#0d0d1a";
const QString Theme::BG_CARD = "#1a1a2e";
const QString Theme::BG_CARD_ALT = "#12122a";
const QString Theme::ACCENT = "#6C3CE1";
const QString Theme::ACCENT_LIGHT = "#8B5CF6";
const QString Theme::TEXT_PRIMARY = "#FFFFFF";
const QString Theme::TEXT_SECONDARY = "#A0A0B8";
const QString Theme::TEXT_MUTED = "#555566";
const QString Theme::SUCCESS = "#4CAF50";
const QString Theme::SUCCESS_LIGHT = "#66BB6A";
const QString Theme::ERROR = "#EF5350";
const QString Theme::ERROR_LIGHT = "#FF7043";
const QString Theme::WARNING = "#FFA726";
const QString Theme::INFO = "#2196F3";
const QString Theme::BORDER = "#2a2a4a";

void Theme::applyDarkTheme() {
    QApplication::setPalette(createDarkPalette());
    QApplication::setFont(getDefaultFont());
    
    // Apply stylesheet for additional styling
    QApplication::setStyleSheet(
        "QToolTip {"
        "  color: #ffffff;"
        "  background-color: " + BG_CARD + ";"
        "  border: 1px solid " + BORDER + ";"
        "}"
        "QComboBox, QSpinBox, QDoubleSpinBox {"
        "  background-color: " + BG_CARD_ALT + ";"
        "  color: " + TEXT_PRIMARY + ";"
        "  border: 1px solid " + BORDER + ";"
        "  border-radius: 6px;"
        "  padding: 6px 10px;"
        "}"
        "QComboBox::drop-down {"
        "  border: none;"
        "  width: 20px;"
        "}"
        "QScrollBar:vertical {"
        "  background-color: " + BG_DARK + ";"
        "  width: 10px;"
        "  border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background-color: " + BORDER + ";"
        "  border-radius: 5px;"
        "  min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background-color: " + TEXT_MUTED + ";"
        "}"
        "QScrollBar:add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
    );
}

QFont Theme::getDefaultFont() {
    QFont font("Segoe UI", 10);
    font.setStyleHint(QFont::SansSerif);
    return font;
}

QPalette Theme::createDarkPalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(BG_DARK));
    palette.setColor(QPalette::WindowText, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::Base, QColor(BG_CARD));
    palette.setColor(QPalette::AlternateBase, QColor(BG_CARD_ALT));
    palette.setColor(QPalette::Text, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::Button, QColor(BG_CARD));
    palette.setColor(QPalette::ButtonText, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::Highlight, QColor(ACCENT));
    palette.setColor(QPalette::HighlightedText, QColor(TEXT_PRIMARY));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(TEXT_SECONDARY));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(TEXT_SECONDARY));
    return palette;
}

} // namespace mubaddil
