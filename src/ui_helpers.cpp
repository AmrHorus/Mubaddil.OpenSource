#include "ui_helpers.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

namespace mubaddil {

// ============= Card =============

Card::Card(QWidget* parent) : QFrame(parent) {
    setFrameStyle(QFrame::StyledPanel);
    setStyleSheet(
        "background-color: " + Theme::BG_CARD + ";"
        "border-radius: 12px;"
        "border: 1px solid " + Theme::BORDER + ";"
    );
    setContentsMargins(16, 16, 16, 16);
    
    auto shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 80));
    shadow->setOffset(0, 4);
    setGraphicsEffect(shadow);
}

// ============= StatusIndicator =============

StatusIndicator::StatusIndicator(QWidget* parent) : QWidget(parent) {
    setFixedSize(80, 32);
    
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);
    
    m_dot = new QLabel("●");
    m_dot->setStyleSheet("color: " + Theme::SUCCESS + "; font-size: 14px;");
    layout->addWidget(m_dot);
    
    m_text = new QLabel("نشط");
    m_text->setStyleSheet("color: " + Theme::SUCCESS + "; font-family: 'Segoe UI'; font-size: 11px;");
    layout->addWidget(m_text);
    
    setStyleSheet(
        "background-color: " + Theme::BG_CARD + ";"
        "border-radius: 16px;"
        "border: 1px solid " + Theme::BORDER + ";"
    );
}

void StatusIndicator::set_active(bool active) {
    QString color = active ? Theme::SUCCESS : Theme::ERROR;
    m_dot->setStyleSheet("color: " + color + "; font-size: 14px;");
    m_text->setText(active ? "نشط" : "متوقف");
    m_text->setStyleSheet("color: " + color + "; font-family: 'Segoe UI'; font-size: 11px;");
}

// ============= StatLabel =============

StatLabel::StatLabel(const QString& title, const QString& value, 
                     const QString& color, QWidget* parent)
    : QWidget(parent) {
    
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    
    m_title = new QLabel(title);
    m_title->setStyleSheet("color: " + Theme::TEXT_SECONDARY + "; font-size: 12px;");
    layout->addWidget(m_title);
    
    m_value = new QLabel(value);
    m_value->setStyleSheet("color: " + color + "; font-size: 20px; font-weight: bold;");
    layout->addWidget(m_value);
    
    layout->addStretch();
}

void StatLabel::setValue(const QString& value) {
    m_value->setText(value);
}

} // namespace mubaddil
