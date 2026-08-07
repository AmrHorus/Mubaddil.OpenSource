#pragma once

/**
 * @file dialogs.h
 * @brief Dialog windows for suggestions, history, and rejected words
 */

#ifndef DIALOGS_H
#define DIALOGS_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <memory>

namespace mubaddil {

class CoreEngine;

/**
 * @class SuggestionDialog
 * @brief Dialog showing correction suggestion
 */
class SuggestionDialog : public QDialog {
    Q_OBJECT

public:
    SuggestionDialog(const QString& original, const QString& suggested,
                    const QString& direction, QWidget* parent = nullptr);
    ~SuggestionDialog();

signals:
    void accepted(const QString& original, const QString& suggested, const QString& direction);
    void rejected(const QString& word);

private slots:
    void onAccept();
    void onReject();
    void onTimeout();

private:
    class Impl;
    std::unique_ptr<Impl> d;
    
    QTimer* m_timer{nullptr};
};

/**
 * @class HistoryDialog
 * @brief Dialog showing correction history
 */
class HistoryDialog : public QDialog {
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget* parent = nullptr);
    ~HistoryDialog();

private:
    class Impl;
    std::unique_ptr<Impl> d;
};

/**
 * @class RejectedDialog
 * @brief Dialog showing rejected words
 */
class RejectedDialog : public QDialog {
    Q_OBJECT

public:
    explicit RejectedDialog(const QStringList& rejectedWords, QWidget* parent = nullptr);
    ~RejectedDialog();

private slots:
    void onClear();

private:
    class Impl;
    std::unique_ptr<Impl> d;
};

} // namespace mubaddil

#endif // DIALOGS_H
