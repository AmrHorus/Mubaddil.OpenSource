#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"
#include "theme.h"
#include "core_engine.h"
#include "config.h"

using namespace mubaddil;

int main(int argc, char *argv[]) {
    // Enable High DPI support
    QApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    
    QApplication app(argc, argv);
    app.setApplicationName("مُبَدِّلْ");
    app.setOrganizationName("Mubaddil");
    app.setApplicationVersion("1.0.0");
    
    // Apply dark theme
    Theme::applyDarkTheme();
    
    // Create and show main window
    MainWindow window;
    
    // Connect close signal to quit
    QObject::connect(&window, &MainWindow::closeRequested, [&app]() {
        app.quit();
    });
    
    // Show window (or minimize if configured)
    ConfigManager config;
    if (!config.get(ConfigManager::KEY_START_MINIMIZED, false).toBool()) {
        window.show();
    } else {
        window.hide();
    }
    
    qDebug() << "Mubaddil started successfully";
    
    return app.exec();
}
