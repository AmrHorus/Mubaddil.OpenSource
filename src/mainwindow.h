#pragma once

/**
 * @file mainwindow.h
 * @brief Main application window
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <memory>

namespace mubaddil {

class CoreEngine;
class ConfigManager;
class StatusIndicator;
class StatLabel;
class Card;

/**
 * @class MainWindow
 * @brief Main application window with statistics and controls
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    /**
     * @brief Set monitoring status indicator
     * @param active Whether monitoring is active
     */
    void setMonitoringStatus(bool active);

    /**
     * @brief Update statistics display
     * @param total Total corrections
     * @param enToAr English to Arabic corrections
     * @param arToEn Arabic to English corrections
     */
    void updateStats(int total, int enToAr, int arToEn);

    /**
     * @brief Update last correction display
     * @param original Original text
     * @param corrected Corrected text
     * @param direction Conversion direction
     * @param time Time of correction
     */
    void updateLastCorrection(const QString& original, const QString& corrected,
                             const QString& direction, const QString& time);

    /**
     * @brief Reset last correction display
     */
    void resetLastCorrection();

    /**
     * @brief Set settings state from config
     * @param settings Map of settings
     */
    void setSettingsState(const QMap<QString, QVariant>& settings);

signals:
    /**
     * @brief Emitted when toggle monitoring is requested
     */
    void toggleMonitoringRequested();

    /**
     * @brief Emitted when show history is requested
     */
    void showHistoryRequested();

    /**
     * @brief Emitted when clear history is requested
     */
    void clearHistoryRequested();

    /**
     * @brief Emitted when show rejected is requested
     */
    void showRejectedRequested();

    /**
     * @brief Emitted when a setting changes
     * @param key Setting key
     * @param value New value
     */
    void settingChanged(const QString& key, const QVariant& value);

    /**
     * @brief Emitted when close is requested
     */
    void closeRequested();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onToggleMonitoring();
    void onShowHistory();
    void onClearHistory();
    void onShowRejected();
    void onMinimize();

private:
    class Impl;
    std::unique_ptr<Impl> d;
    
    void setupUI();
    void setupConnections();
    void createSystemTray();
};

} // namespace mubaddil

#endif // MAINWINDOW_H
