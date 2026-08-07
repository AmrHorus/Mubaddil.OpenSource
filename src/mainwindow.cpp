#include "mainwindow.h"
#include "core_engine.h"
#include "config.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDebug>

namespace mubaddil {

class MainWindow::Impl {
public:
    CoreEngine* engine{nullptr};
    ConfigManager* config{nullptr};
    
    // UI Components
    QWidget* centralWidget{nullptr};
    QVBoxLayout* mainLayout{nullptr};
    
    // Header
    QLabel* titleLabel{nullptr};
    StatusIndicator* statusIndicator{nullptr};
    
    // Stats Card
    Card* statsCard{nullptr};
    StatLabel* totalStat{nullptr};
    StatLabel* enToArStat{nullptr};
    StatLabel* arToEnStat{nullptr};
    
    // Last Correction Card
    Card* lastCorrectionCard{nullptr};
    QLabel* lastCorrectionLabel{nullptr};
    
    // Controls
    QPushButton* toggleButton{nullptr};
    QPushButton* historyButton{nullptr};
    QPushButton* rejectedButton{nullptr};
    QPushButton* clearButton{nullptr};
    
    // Settings
    Card* settingsCard{nullptr};
    QCheckBox* showSuggestionsCheck{nullptr};
    QCheckBox* autoCorrectCheck{nullptr};
    QCheckBox* minimizeToTrayCheck{nullptr};
    QCheckBox* startMinimizedCheck{nullptr};
    
    // System Tray
    QSystemTrayIcon* trayIcon{nullptr};
    QMenu* trayMenu{nullptr};
    
    bool monitoring{false};
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), d(std::make_unique<Impl>()) {
    
    setWindowTitle("مُبَدِّلْ - Mubaddil");
    setMinimumSize(500, 600);
    
    d->engine = new CoreEngine(this);
    d->config = new ConfigManager();
    
    setupUI();
    setupConnections();
    createSystemTray();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    d->centralWidget = new QWidget();
    setCentralWidget(d->centralWidget);
    
    d->mainLayout = new QVBoxLayout(d->centralWidget);
    d->mainLayout->setSpacing(16);
    d->mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    d->titleLabel = new QLabel("مُبَدِّلْ");
    d->titleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: " + Theme::TEXT_PRIMARY + ";");
    d->titleLabel->setAlignment(Qt::AlignCenter);
    d->mainLayout->addWidget(d->titleLabel);
    
    // Status Indicator (to be implemented)
    d->statusIndicator = new StatusIndicator();
    d->mainLayout->addWidget(d->statusIndicator);
    
    // Stats Card
    d->statsCard = new Card();
    auto statsLayout = new QHBoxLayout(d->statsCard);
    statsLayout->setSpacing(20);
    
    d->totalStat = new StatLabel("الإجمالي", "0", Theme::ACCENT_LIGHT);
    d->enToArStat = new StatLabel("إنجليزي → عربي", "0", Theme::SUCCESS);
    d->arToEnStat = new StatLabel("عربي → إنجليزي", "0", Theme::INFO);
    
    statsLayout->addWidget(d->totalStat);
    statsLayout->addWidget(d->enToArStat);
    statsLayout->addWidget(d->arToEnStat);
    d->mainLayout->addWidget(d->statsCard);
    
    // Last Correction Card
    d->lastCorrectionCard = new Card();
    auto correctionLayout = new QVBoxLayout(d->lastCorrectionCard);
    
    auto correctionTitle = new QLabel("آخر تصحيح");
    correctionTitle->setStyleSheet("font-size: 14px; color: " + Theme::TEXT_SECONDARY + ";");
    correctionLayout->addWidget(correctionTitle);
    
    d->lastCorrectionLabel = new QLabel("لا توجد تصحيحات بعد");
    d->lastCorrectionLabel->setStyleSheet("font-size: 16px; color: " + Theme::TEXT_PRIMARY + ";");
    d->lastCorrectionLabel->setWordWrap(true);
    correctionLayout->addWidget(d->lastCorrectionLabel);
    
    correctionLayout->addStretch();
    d->mainLayout->addWidget(d->lastCorrectionCard);
    
    // Control Buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    
    d->toggleButton = new QPushButton("⏸ إيقاف المراقبة");
    d->toggleButton->setMinimumHeight(44);
    d->toggleButton->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #EF5350, stop:1 #FF7043);"
        "color: white; border: none; border-radius: 8px;"
        "font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;"
    );
    buttonLayout->addWidget(d->toggleButton);
    
    d->historyButton = new QPushButton("📜 السجل");
    d->historyButton->setMinimumHeight(44);
    buttonLayout->addWidget(d->historyButton);
    
    d->rejectedButton = new QPushButton("❌ المرفوضة");
    d->rejectedButton->setMinimumHeight(44);
    buttonLayout->addWidget(d->rejectedButton);
    
    d->mainLayout->addLayout(buttonLayout);
    
    d->clearButton = new QPushButton("🗑 مسح السجل");
    d->clearButton->setMinimumHeight(40);
    d->clearButton->setStyleSheet(
        "background-color: " + Theme::BG_CARD_ALT + ";"
        "color: " + Theme::TEXT_SECONDARY + "; border: 1px solid " + Theme::BORDER + ";"
        "border-radius: 8px; font-family: 'Segoe UI'; font-size: 12px;"
    );
    d->mainLayout->addWidget(d->clearButton);
    
    // Settings Card
    d->settingsCard = new Card();
    auto settingsLayout = new QVBoxLayout(d->settingsCard);
    settingsLayout->setSpacing(12);
    
    auto settingsTitle = new QLabel("الإعدادات");
    settingsTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: " + Theme::TEXT_PRIMARY + ";");
    settingsLayout->addWidget(settingsTitle);
    
    d->showSuggestionsCheck = new QCheckBox("إظهار الاقتراحات");
    d->showSuggestionsCheck->setChecked(true);
    d->showSuggestionsCheck->setStyleSheet("color: " + Theme::TEXT_PRIMARY + "; font-size: 13px;");
    settingsLayout->addWidget(d->showSuggestionsCheck);
    
    d->autoCorrectCheck = new QCheckBox("التصحيح التلقائي");
    d->autoCorrectCheck->setStyleSheet("color: " + Theme::TEXT_PRIMARY + "; font-size: 13px;");
    settingsLayout->addWidget(d->autoCorrectCheck);
    
    d->minimizeToTrayCheck = new QCheckBox("التصغير إلى منطقة الإشعارات");
    d->minimizeToTrayCheck->setChecked(true);
    d->minimizeToTrayCheck->setStyleSheet("color: " + Theme::TEXT_PRIMARY + "; font-size: 13px;");
    settingsLayout->addWidget(d->minimizeToTrayCheck);
    
    d->startMinimizedCheck = new QCheckBox("البدء مصغراً");
    d->startMinimizedCheck->setStyleSheet("color: " + Theme::TEXT_PRIMARY + "; font-size: 13px;");
    settingsLayout->addWidget(d->startMinimizedCheck);
    
    settingsLayout->addStretch();
    d->mainLayout->addWidget(d->settingsCard);
    
    d->mainLayout->addStretch();
}

void MainWindow::setupConnections() {
    connect(d->toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleMonitoring);
    connect(d->historyButton, &QPushButton::clicked, this, &MainWindow::onShowHistory);
    connect(d->rejectedButton, &QPushButton::clicked, this, &MainWindow::onShowRejected);
    connect(d->clearButton, &QPushButton::clicked, this, &MainWindow::onClearHistory);
    
    connect(d->showSuggestionsCheck, &QCheckBox::toggled, this, [this](bool checked) {
        emit settingChanged(ConfigManager::KEY_SHOW_SUGGESTIONS, checked);
    });
    
    connect(d->autoCorrectCheck, &QCheckBox::toggled, this, [this](bool checked) {
        emit settingChanged(ConfigManager::KEY_AUTO_CORRECT, checked);
    });
    
    connect(d->minimizeToTrayCheck, &QCheckBox::toggled, this, [this](bool checked) {
        emit settingChanged(ConfigManager::KEY_MINIMIZE_TO_TRAY, checked);
    });
    
    connect(d->startMinimizedCheck, &QCheckBox::toggled, this, [this](bool checked) {
        emit settingChanged(ConfigManager::KEY_START_MINIMIZED, checked);
    });
    
    connect(d->engine, &CoreEngine::suggestionReady, this, [](const CorrectionData& data) {
        qDebug() << "Suggestion:" << data.original << "->" << data.suggested;
    });
    
    connect(d->engine, &CoreEngine::correctedReady, this, [this](const CorrectionData& data) {
        updateLastCorrection(data.original, data.suggested, 
                            data.direction == Direction::EnToAr ? "EN→AR" : "AR→EN", "الآن");
    });
}

void MainWindow::createSystemTray() {
    d->trayIcon = new QSystemTrayIcon(this);
    d->trayIcon->setIcon(QIcon(":/icons/app.ico"));
    d->trayIcon->setToolTip("مُبَدِّلْ - Mubaddil");
    
    d->trayMenu = new QMenu(this);
    
    auto showAction = d->trayMenu->addAction("إظهار النافذة");
    connect(showAction, &QAction::triggered, this, [this]() {
        showNormal();
        raise();
        activateWindow();
    });
    
    auto toggleAction = d->trayMenu->addAction("تبديل المراقبة");
    connect(toggleAction, &QAction::triggered, this, &MainWindow::onToggleMonitoring);
    
    d->trayMenu->addSeparator();
    
    auto quitAction = d->trayMenu->addAction("خروج");
    connect(quitAction, &QAction::triggered, this, &MainWindow::closeRequested);
    
    d->trayIcon->setContextMenu(d->trayMenu);
    d->trayIcon->show();
}

void MainWindow::setMonitoringStatus(bool active) {
    d->monitoring = active;
    d->statusIndicator->set_active(active);
    
    if (active) {
        d->toggleButton->setText("⏸ إيقاف المراقبة");
        d->toggleButton->setStyleSheet(
            "background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #EF5350, stop:1 #FF7043);"
            "color: white; border: none; border-radius: 8px;"
            "font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;"
        );
    } else {
        d->toggleButton->setText("▶ بدء المراقبة");
        d->toggleButton->setStyleSheet(
            "background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #4CAF50, stop:1 #66BB6A);"
            "color: white; border: none; border-radius: 8px;"
            "font-family: 'Segoe UI'; font-size: 14px; font-weight: bold;"
        );
    }
}

void MainWindow::updateStats(int total, int enToAr, int arToEn) {
    d->totalStat->setValue(QString::number(total));
    d->enToArStat->setValue(QString::number(enToAr));
    d->arToEnStat->setValue(QString::number(arToEn));
}

void MainWindow::updateLastCorrection(const QString& original, const QString& corrected,
                                      const QString& direction, const QString& time) {
    d->lastCorrectionLabel->setText(
        QString("<b>%1</b> → <b style='color: %2'>%3</b> <span style='color: %4'>(%5)</span>")
            .arg(original)
            .arg(Theme::SUCCESS)
            .arg(corrected)
            .arg(Theme::TEXT_MUTED)
            .arg(time)
    );
}

void MainWindow::resetLastCorrection() {
    d->lastCorrectionLabel->setText("لا توجد تصحيحات بعد");
}

void MainWindow::setSettingsState(const QMap<QString, QVariant>& settings) {
    d->showSuggestionsCheck->setChecked(settings.value(ConfigManager::KEY_SHOW_SUGGESTIONS, true).toBool());
    d->autoCorrectCheck->setChecked(settings.value(ConfigManager::KEY_AUTO_CORRECT, false).toBool());
    d->minimizeToTrayCheck->setChecked(settings.value(ConfigManager::KEY_MINIMIZE_TO_TRAY, true).toBool());
    d->startMinimizedCheck->setChecked(settings.value(ConfigManager::KEY_START_MINIMIZED, false).toBool());
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (d->config->get(ConfigManager::KEY_MINIMIZE_TO_TRAY, true).toBool() && d->trayIcon->isVisible()) {
        event->ignore();
        hide();
        d->trayIcon->showMessage("مُبَدِّلْ", "البرنامج يعمل في الخلفية", QSystemTrayIcon::Information, 2000);
    } else {
        emit closeRequested();
        event->accept();
    }
}

void MainWindow::onToggleMonitoring() {
    d->monitoring = !d->monitoring;
    setMonitoringStatus(d->monitoring);
    
    if (d->monitoring) {
        d->engine->start();
    } else {
        d->engine->stop();
    }
}

void MainWindow::onShowHistory() {
    emit showHistoryRequested();
}

void MainWindow::onClearHistory() {
    emit clearHistoryRequested();
    resetLastCorrection();
    updateStats(0, 0, 0);
}

void MainWindow::onShowRejected() {
    emit showRejectedRequested();
}

void MainWindow::onMinimize() {
    showMinimized();
}

} // namespace mubaddil
