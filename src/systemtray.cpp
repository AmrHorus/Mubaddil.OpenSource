#include "systemtray.h"
#include "theme.h"
#include <QAction>
#include <QMenu>

namespace mubaddil {

class SystemTray::Impl {
public:
    QSystemTrayIcon* trayIcon{nullptr};
    QMenu* contextMenu{nullptr};
    bool monitoring{false};
};

SystemTray::SystemTray(QObject* parent)
    : QObject(parent), d(std::make_unique<Impl>()) {
    
    d->trayIcon = new QSystemTrayIcon(this);
    d->trayIcon->setIcon(QIcon(":/icons/app.ico"));
    d->trayIcon->setToolTip("مُبَدِّلْ - Mubaddil");
    
    setupMenu();
}

SystemTray::~SystemTray() = default;

void SystemTray::setupMenu() {
    d->contextMenu = new QMenu();
    
    auto showAction = d->contextMenu->addAction("إظهار النافذة");
    connect(showAction, &QAction::triggered, this, &SystemTray::showWindow);
    
    auto statusAction = d->contextMenu->addAction("المراقبة نشطة");
    statusAction->setEnabled(false);
    
    d->contextMenu->addSeparator();
    
    auto toggleAction = d->contextMenu->addAction("تبديل المراقبة");
    connect(toggleAction, &QAction::triggered, this, &SystemTray::toggleMonitoring);
    
    d->contextMenu->addSeparator();
    
    auto quitAction = d->contextMenu->addAction("خروج");
    connect(quitAction, &QAction::triggered, this, &SystemTray::quitApp);
    
    d->trayIcon->setContextMenu(d->contextMenu);
}

void SystemTray::show() {
    d->trayIcon->show();
}

void SystemTray::hide() {
    d->trayIcon->hide();
}

void SystemTray::setMonitoringStatus(bool active) {
    d->monitoring = active;
    
    // Update tooltip
    d->trayIcon->setToolTip(active ? 
        "مُبَدِّلْ - المراقبة نشطة" : 
        "مُبَدِّلْ - المراقبة متوقفة");
    
    // Update menu status text if needed
    for (auto action : d->contextMenu->actions()) {
        if (action->text().startsWith("المراقبة")) {
            action->setText(active ? "المراقبة نشطة ✅" : "المراقبة متوقفة ⏸");
            break;
        }
    }
}

void SystemTray::showNotification(const QString& title, const QString& message, int timeout) {
    d->trayIcon->showMessage(title, message, QSystemTrayIcon::Information, timeout);
}

} // namespace mubaddil
