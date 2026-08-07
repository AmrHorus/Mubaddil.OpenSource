#pragma once

/**
 * @file systemtray.h
 * @brief System tray icon and notifications
 */

#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <memory>

namespace mubaddil {

/**
 * @class SystemTray
 * @brief Manages system tray icon and context menu
 */
class SystemTray : public QObject {
    Q_OBJECT

public:
    explicit SystemTray(QObject* parent = nullptr);
    ~SystemTray();

    /**
     * @brief Show the tray icon
     */
    void show();

    /**
     * @brief Hide the tray icon
     */
    void hide();

    /**
     * @brief Set monitoring status
     * @param active Whether monitoring is active
     */
    void setMonitoringStatus(bool active);

    /**
     * @brief Show a notification message
     * @param title Notification title
     * @param message Notification message
     * @param timeout Display timeout in milliseconds
     */
    void showNotification(const QString& title, const QString& message, int timeout = 3000);

signals:
    /**
     * @brief Emitted when user requests to show window
     */
    void showWindow();

    /**
     * @brief Emitted when user toggles monitoring
     */
    void toggleMonitoring();

    /**
     * @brief Emitted when user requests to quit
     */
    void quitApp();

private:
    class Impl;
    std::unique_ptr<Impl> d;
    
    void setupMenu();
};

} // namespace mubaddil

#endif // SYSTEMTRAY_H
