#include "core_engine.h"
#include "hook/bridge.h"
#include <QCoreApplication>
#include <QDebug>

namespace mubaddil {

class CoreEngine::Impl {
public:
    bool monitoring{false};
    int totalCorrections{0};
    int enToArCount{0};
    int arToEnCount{0};
    QStringList rejectedWords;
    EventCallback callback;
    
    Impl() = default;
};

CoreEngine::CoreEngine(QObject* parent) 
    : QObject(parent), d(std::make_unique<Impl>()) {
}

CoreEngine::~CoreEngine() {
    stop();
}

bool CoreEngine::start() {
    if (d->monitoring) {
        return true;
    }
    
    // Initialize the native hook
    auto result = mubaddil::InitializeHook();
    if (result) {
        d->monitoring = true;
        qDebug() << "Keyboard hook started successfully";
        return true;
    }
    
    qWarning() << "Failed to start keyboard hook";
    return false;
}

void CoreEngine::stop() {
    if (!d->monitoring) {
        return;
    }
    
    mubaddil::UninitializeHook();
    d->monitoring = false;
    qDebug() << "Keyboard hook stopped";
}

bool CoreEngine::toggleMonitoring() {
    if (d->monitoring) {
        stop();
        return false;
    } else {
        return start();
    }
}

bool CoreEngine::isMonitoring() const {
    return d->monitoring;
}

void CoreEngine::acceptCorrection(const QString& original, const QString& suggested,
                                  Direction direction, int hwnd) {
    // Convert QString to std::wstring
    auto origW = original.toStdWString();
    auto suggW = suggested.toStdWString();
    
    // Call native function
    mubaddil::AcceptCorrection(origW, suggW, static_cast<int>(direction), hwnd);
    
    // Update counts
    d->totalCorrections++;
    if (direction == Direction::EnToAr) {
        d->enToArCount++;
    } else {
        d->arToEnCount++;
    }
    
    // Emit signal
    CorrectionData data{original, suggested, direction, hwnd};
    emit correctedReady(data);
    
    if (d->callback) {
        d->callback(EventType::Corrected, data);
    }
}

void CoreEngine::rejectCorrection(const QString& word) {
    d->rejectedWords.append(word);
    
    // Call native function
    mubaddil::RejectCorrection(word.toStdWString());
    
    // Emit signal
    CorrectionData data{word, word, Direction::EnToAr, 0};
    emit rejectedReady(data);
    
    if (d->callback) {
        d->callback(EventType::Rejected, data);
    }
}

int CoreEngine::getTotalCorrections() const {
    return d->totalCorrections;
}

int CoreEngine::getEnToArCount() const {
    return d->enToArCount;
}

int CoreEngine::getArToEnCount() const {
    return d->arToEnCount;
}

QMap<QString, int> CoreEngine::getCounts() const {
    QMap<QString, int> counts;
    counts["total"] = d->totalCorrections;
    counts["en_to_ar"] = d->enToArCount;
    counts["ar_to_en"] = d->arToEnCount;
    return counts;
}

void CoreEngine::clearHistory() {
    d->totalCorrections = 0;
    d->enToArCount = 0;
    d->arToEnCount = 0;
    d->rejectedWords.clear();
    mubaddil::ClearHistory();
}

QStringList CoreEngine::getRejectedWords() const {
    return d->rejectedWords;
}

void CoreEngine::setEventCallback(EventCallback callback) {
    d->callback = std::move(callback);
}

void CoreEngine::onNativeEvent(EventType type, const CorrectionData& data) {
    switch (type) {
        case EventType::Suggestion:
            emit suggestionReady(data);
            break;
        case EventType::Corrected:
            emit correctedReady(data);
            break;
        case EventType::Rejected:
            emit rejectedReady(data);
            break;
    }
    
    if (d->callback) {
        d->callback(type, data);
    }
}

} // namespace mubaddil
