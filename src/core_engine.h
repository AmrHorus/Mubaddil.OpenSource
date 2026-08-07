#pragma once

/**
 * @file core_engine.h
 * @brief Core engine wrapper for keyboard monitoring and correction
 * 
 * Provides high-level API for the Mubaddil keyboard language switcher.
 */

#ifndef CORE_ENGINE_H
#define CORE_ENGINE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QVariant>
#include <functional>
#include <memory>

namespace mubaddil {

enum class Direction {
    EnToAr,
    ArToEn
};

enum class EventType {
    Suggestion,
    Corrected,
    Rejected
};

/**
 * @struct CorrectionData
 * @brief Data structure for correction events
 */
struct CorrectionData {
    QString original;
    QString suggested;
    Direction direction;
    int hwnd{0};
};

/**
 * @class CoreEngine
 * @brief Main engine for keyboard monitoring and text correction
 * 
 * Wraps the native C++ keyboard hook engine and provides Qt signals
 * for UI integration.
 */
class CoreEngine : public QObject {
    Q_OBJECT

public:
    explicit CoreEngine(QObject* parent = nullptr);
    ~CoreEngine();

    /**
     * @brief Start keyboard monitoring
     * @return true if successful
     */
    bool start();

    /**
     * @brief Stop keyboard monitoring
     */
    void stop();

    /**
     * @brief Toggle monitoring on/off
     * @return current monitoring state
     */
    bool toggleMonitoring();

    /**
     * @brief Check if monitoring is active
     * @return true if monitoring
     */
    bool isMonitoring() const;

    /**
     * @brief Accept a correction suggestion
     * @param original Original text
     * @param suggested Suggested replacement
     * @param direction Conversion direction
     * @param hwnd Target window handle
     */
    void acceptCorrection(const QString& original, const QString& suggested, 
                         Direction direction, int hwnd);

    /**
     * @brief Reject a correction suggestion
     * @param word Word to reject
     */
    void rejectCorrection(const QString& word);

    /**
     * @brief Get correction history count
     * @return Total corrections made
     */
    int getTotalCorrections() const;

    /**
     * @brief Get English to Arabic correction count
     * @return Count of EN->AR corrections
     */
    int getEnToArCount() const;

    /**
     * @brief Get Arabic to English correction count
     * @return Count of AR->EN corrections
     */
    int getArToEnCount() const;

    /**
     * @brief Get all correction counts
     * @return Map with total, en_to_ar, ar_to_en counts
     */
    QMap<QString, int> getCounts() const;

    /**
     * @brief Clear correction history
     */
    void clearHistory();

    /**
     * @brief Get rejected words list
     * @return List of rejected words
     */
    QStringList getRejectedWords() const;

    /**
     * @brief Set callback for UI events
     * @param callback Function to call on events
     */
    using EventCallback = std::function<void(EventType, const CorrectionData&)>;
    void setEventCallback(EventCallback callback);

signals:
    /**
     * @brief Emitted when a suggestion is ready
     * @param data Correction data
     */
    void suggestionReady(const CorrectionData& data);

    /**
     * @brief Emitted when a correction is made
     * @param data Correction data
     */
    void correctedReady(const CorrectionData& data);

    /**
     * @brief Emitted when a correction is rejected
     * @param data Correction data
     */
    void rejectedReady(const CorrectionData& data);

private:
    class Impl;
    std::unique_ptr<Impl> d;
    
    void onNativeEvent(EventType type, const CorrectionData& data);
};

} // namespace mubaddil

#endif // CORE_ENGINE_H
