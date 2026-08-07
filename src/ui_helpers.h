#pragma once

/**
 * @file ui_helpers.h
 * @brief Helper UI components
 */

#ifndef UI_HELPERS_H
#define UI_HELPERS_H

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QString>

namespace mubaddil {

/**
 * @class Card
 * @brief Styled card container with shadow effect
 */
class Card : public QFrame {
    Q_OBJECT

public:
    explicit Card(QWidget* parent = nullptr);
};

/**
 * @class StatusIndicator
 * @brief Shows active/inactive status with colored dot
 */
class StatusIndicator : public QWidget {
    Q_OBJECT

public:
    explicit StatusIndicator(QWidget* parent = nullptr);
    
    /**
     * @brief Set active state
     * @param active Whether status is active
     */
    void set_active(bool active);

private:
    QLabel* m_dot{nullptr};
    QLabel* m_text{nullptr};
};

/**
 * @class StatLabel
 * @brief Displays a statistic with title and value
 */
class StatLabel : public QWidget {
    Q_OBJECT

public:
    StatLabel(const QString& title, const QString& value = "0", 
              const QString& color = "#A0A0B8", QWidget* parent = nullptr);
    
    /**
     * @brief Set the value
     * @param value New value to display
     */
    void setValue(const QString& value);

private:
    QLabel* m_title{nullptr};
    QLabel* m_value{nullptr};
};

} // namespace mubaddil

#endif // UI_HELPERS_H
