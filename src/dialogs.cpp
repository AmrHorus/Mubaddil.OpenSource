#include "dialogs.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

namespace mubaddil {

// ============= SuggestionDialog =============

class SuggestionDialog::Impl {
public:
    QString original;
    QString suggested;
    QString direction;
    
    QLabel* messageLabel{nullptr};
    QPushButton* acceptButton{nullptr};
    QPushButton* rejectButton{nullptr};
};

SuggestionDialog::SuggestionDialog(const QString& original, const QString& suggested,
                                   const QString& direction, QWidget* parent)
    : QDialog(parent), d(std::make_unique<Impl>()) {
    
    d->original = original;
    d->suggested = suggested;
    d->direction = direction;
    
    setWindowTitle("اقتراح التصحيح");
    setModal(false);
    setFixedSize(400, 200);
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Message
    d->messageLabel = new QLabel(
        QString("هل تقبل التصحيح:<br><b style='color: %1'>%2</b> → <b style='color: %3'>%4</b>")
            .arg(Theme::TEXT_SECONDARY).arg(original)
            .arg(Theme::SUCCESS).arg(suggested)
    );
    d->messageLabel->setWordWrap(true);
    d->messageLabel->setStyleSheet("font-size: 14px; color: " + Theme::TEXT_PRIMARY + ";");
    layout->addWidget(d->messageLabel);
    
    // Buttons
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    
    d->acceptButton = new QPushButton("✅ قبول");
    d->acceptButton->setMinimumHeight(36);
    d->acceptButton->setStyleSheet(
        "background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #4CAF50, stop:1 #66BB6A);"
        "color: white; border: none; border-radius: 8px;"
        "font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
    );
    buttonLayout->addWidget(d->acceptButton);
    
    d->rejectButton = new QPushButton("❌ رفض");
    d->rejectButton->setMinimumHeight(36);
    d->rejectButton->setStyleSheet(
        "background-color: " + Theme::BG_CARD_ALT + ";"
        "color: " + Theme::ERROR + "; border: 1px solid " + Theme::BORDER + ";"
        "border-radius: 8px; font-family: 'Segoe UI'; font-size: 13px;"
    );
    buttonLayout->addWidget(d->rejectButton);
    
    layout->addLayout(buttonLayout);
    
    // Auto-close timer (5 seconds)
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &SuggestionDialog::onTimeout);
    m_timer->start(5000);
    
    connect(d->acceptButton, &QPushButton::clicked, this, &SuggestionDialog::onAccept);
    connect(d->rejectButton, &QPushButton::clicked, this, &SuggestionDialog::onReject);
}

SuggestionDialog::~SuggestionDialog() = default;

void SuggestionDialog::onAccept() {
    m_timer->stop();
    emit accepted(d->original, d->suggested, d->direction);
    accept();
}

void SuggestionDialog::onReject() {
    m_timer->stop();
    emit rejected(d->original);
    reject();
}

void SuggestionDialog::onTimeout() {
    reject();
}

// ============= HistoryDialog =============

class HistoryDialog::Impl {
public:
    QTextEdit* historyText{nullptr};
};

HistoryDialog::HistoryDialog(QWidget* parent)
    : QDialog(parent), d(std::make_unique<Impl>()) {
    
    setWindowTitle("سجل التصحيحات");
    setMinimumSize(500, 400);
    
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);
    
    auto title = new QLabel("📜 سجل التصحيحات");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: " + Theme::TEXT_PRIMARY + ";");
    layout->addWidget(title);
    
    d->historyText = new QTextEdit();
    d->historyText->setReadOnly(true);
    d->historyText->setStyleSheet(
        "background-color: " + Theme::BG_CARD_ALT + ";"
        "color: " + Theme::TEXT_PRIMARY + ";"
        "border: 1px solid " + Theme::BORDER + ";"
        "border-radius: 8px; padding: 10px;"
        "font-family: 'Segoe UI'; font-size: 13px;"
    );
    d->historyText->setPlainText("لا توجد تصحيحات بعد.");
    layout->addWidget(d->historyText);
    
    auto closeButton = new QPushButton("إغلاق");
    closeButton->setMinimumHeight(36);
    closeButton->setStyleSheet(
        "background-color: " + Theme::ACCENT + ";"
        "color: white; border: none; border-radius: 8px;"
        "font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
    );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
}

HistoryDialog::~HistoryDialog() = default;

// ============= RejectedDialog =============

class RejectedDialog::Impl {
public:
    QListWidget* listWidget{nullptr};
    QStringList words;
};

RejectedDialog::RejectedDialog(const QStringList& rejectedWords, QWidget* parent)
    : QDialog(parent), d(std::make_unique<Impl>()) {
    
    d->words = rejectedWords;
    
    setWindowTitle("الكلمات المرفوضة");
    setMinimumSize(400, 300);
    
    auto layout = new QVBoxLayout(this);
    layout->setSpacing(16);
    layout->setContentsMargins(20, 20, 20, 20);
    
    auto title = new QLabel("❌ الكلمات المرفوضة");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: " + Theme::TEXT_PRIMARY + ";");
    layout->addWidget(title);
    
    d->listWidget = new QListWidget();
    d->listWidget->setStyleSheet(
        "background-color: " + Theme::BG_CARD_ALT + ";"
        "color: " + Theme::TEXT_PRIMARY + ";"
        "border: 1px solid " + Theme::BORDER + ";"
        "border-radius: 8px; padding: 5px;"
        "font-family: 'Segoe UI'; font-size: 13px;"
    );
    d->listWidget->addItems(rejectedWords);
    layout->addWidget(d->listWidget);
    
    auto buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    
    auto clearButton = new QPushButton("🗑 مسح الكل");
    clearButton->setMinimumHeight(36);
    clearButton->setStyleSheet(
        "background-color: " + Theme::ERROR + ";"
        "color: white; border: none; border-radius: 8px;"
        "font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
    );
    connect(clearButton, &QPushButton::clicked, this, &RejectedDialog::onClear);
    buttonLayout->addWidget(clearButton);
    
    auto closeButton = new QPushButton("إغلاق");
    closeButton->setMinimumHeight(36);
    closeButton->setStyleSheet(
        "background-color: " + Theme::ACCENT + ";"
        "color: white; border: none; border-radius: 8px;"
        "font-family: 'Segoe UI'; font-size: 13px; font-weight: bold;"
    );
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(closeButton);
    
    layout->addLayout(buttonLayout);
}

RejectedDialog::~RejectedDialog() = default;

void RejectedDialog::onClear() {
    d->words.clear();
    d->listWidget->clear();
}

} // namespace mubaddil
