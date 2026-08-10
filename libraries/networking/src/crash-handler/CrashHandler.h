//
//  CrashHandler.cpp
//
//
//  Created by Dale Glass on 25/06/2023.
//  Copyright 2023-2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <QObject>
#include <QCoreApplication>
#include <SettingHandle.h>
#include <QLoggingCategory>
#include <atomic>
#include <unordered_map>
#include <mutex>

#include <string_view>



#ifndef overte_CrashHandler_h
#define overte_CrashHandler_h


Q_DECLARE_LOGGING_CATEGORY(crash_handler)


// We get these from CMake if crash reporting is enabled

#ifndef OVERTE_BACKTRACE_URL
#define OVERTE_BACKTRACE_URL ""
#endif

#ifndef OVERTE_BACKTRACE_TOKEN
#define OVERTE_BACKTRACE_TOKEN ""
#endif




/**
 * @brief The global object in charge of setting up and controlling crash reporting.
 *
 * This object initializes and talks to crash reporting backends. For those, see
 * CrashHandlerBackend.h and the .cpp files that implement that interface.
 *
 * The crash URL and token can only be passed to the underlying system on start, so
 * things should be set up in such a way that startup is only done after those are set.
 *
 * start() will be automatically called when setEnabled() is called with true.
 * setAnnotation() can only be called after start.
 *
 *
 * To use, follow this general pattern in an application:
 *
 * @code {.cpp}
 * auto &ch = CrashHandler::getInstance();
 * ch.setPath(...);
 * ch.setUrl("https://server.com/crash-reports");
 * ch.setToken("1.2beta");
 * ch.setEnabled(true);
 * ch.setAnnotation("version", "1.3"); // Needs a started handler to work
 * @endcode
 *
 * For an assignment client, there are two potential ways to start, through the command-line
 * and through the settings system. Since the path, URL and token only apply on startup, the
 * code must be written such that if command arguments are not given, setEnabled() or start()
 * are not called until receiving the settings from the domain.
 *
 */
class CrashHandler : public QObject {
    Q_OBJECT

public:
    static CrashHandler& getInstance();

    QString path() const { return _path; }
    QString url() const { return _crashUrl; }
    QString token() const { return _crashToken; }


public slots:


    /**
     * @brief Set the directory for the crash reports
     *
     * This sets the path for writing crash reports. This should be done on application startup.
     *
     * @param path Directory where to store crash reports. It's allowed to set this to argv[0],
     * if the path is a filename, then the base directory will be automatically used.
     */
    void setPath(const QString &path);



    /**
     * @brief Start the crash handler
     *
     * This is called automatically if it wasn't started yet when setEnabled() is called.
     *
     * @param path Path where to store the crash database
     * @return true Started successfully
     * @return false Failed to start
     */
    bool start();


    /**
     * @brief Starts the unhandled exception monitor.
     *
     * On Windows, it's possible for the unhandled exception handler to be reset. This starts a timer
     * to periodically set it back.
     *
     * On non-Windows systems this has no effect.
     *
     * @param app Main application
     */
    void startMonitor(QCoreApplication *app);



    /**
     * @brief Whether the crash monitor has been successfully started
     *
     * Reasons for it failing to start include:
     *
     *  * Not having a crash reporter for the platform
     *  * Crash reporter not being configured with reporting URLs (OVERTE_BACKTRACE_TOKEN and OVERTE_BACKTRACE_URL)
     *  * Crash reporter is present and configured, but failed to initialize for some reason
     *
     * @return true Crash reporter is present, configured and working.
     * @return false Crash reporter has not been started for one of the above reasons.
     */
    bool isStarted() const { return _crashMonitorStarted; }


    /**
     * @brief Whether the crash monitor will report crashes if they occur
     *
     * This setting is independent of isCrashMonitorStarted() -- crash reporting may be enabled but fail to work
     * due to the crash reporting component being missing or failing to initialize.
     *
     * @return true Crashes will be reported to OVERTE_BACKTRACE_URL
     * @return false Crashes will not be reported
     */
    bool isEnabled() const { return _crashReportingEnabled; }

    /**
     * @brief Set whether we want to submit crash reports to the report server
     *
     * The report server is configured with OVERTE_BACKTRACE_URL.
     * Emits crashReportingEnabledChanged signal.
     *
     * @note This automatically calls start(), so it should be called after setPath(), setUrl() and setToken()
     * @param enabled Whether it's enabled.
     */
    void setEnabled(bool enabled);



    /**
     * @brief Whether logs are streamed to the server for debugging.
     *
     * Logs are sent even in the absence of a crash if this is enabled.
     *
     * @return true
     * @return false
     */
    bool isLogStreamingEnabled() const { return _logStreamingEnabled; }

    /**
     * @brief Set whether logs are streamed to the server for debugging.
     *
     * Logs are sent even in the absence of a crash if this is enabled.
     *
     * Emits enabledChanged signal.
     *
     * @param enabled
     */
    void setLogStreamingEnabled(bool enabled) {
        _logStreamingEnabled = enabled;
        emit logStreamingChanged(enabled);
    }

    /**
     * @brief Whether stats are streamed to the server for debugging.
     *
     * @return true
     * @return false
     */
    bool isStatsStreamingEnabled() const { return _statsStreamingEnabled; }

    /**
     * @brief Set whether stats are streamed to the server for debugging.
     *
     * Emits enabledChanged signal.
     * @param enabled
     */
    void setStatsStreamingEnabled(bool enabled) {
        _statsStreamingEnabled = enabled;
        emit statsStreamingChanged(enabled);
    }

    /**
     * @brief Set the URL where to send crash reports to
     *
     * If not set, a predefined URL specified at compile time via OVERTE_BACKTRACE_URL
     * will be used.
     *
     * @param url URL
     */
    void setUrl(const QString &url);

    /**
     * @brief Set the token for the crash reporter
     *
     * This is an identifier in the crash collection service, such as Sentry, and may contain
     * a branch name or a version number.
     *
     * If not set, a predefined token specified at compile time via OVERTE_BACKTRACE_TOKEN
     * will be used.
     *
     * @param token Token
     */
    void setToken(const QString &token);



    /**
     * @brief Set an annotation to be added to a crash
     *
     * Annotations add extra information, such as the application's version number,
     * the current user, or any other information of interest.
     *
     * In Sentry, this creates a tag and they are searchable in the Sentry web interface.
     *
     * @note Annotations made before the crash handler are remembered, and sent to the
     * crash handler as soon as it's initialized.
     *
     * @param key Key
     * @param value Value
     */
    void setAnnotation(const std::string &key, const char *value);

    /**
     * @brief Set an annotation to be added to a crash
     *
     * Annotations add extra information, such as the application's version number,
     * the current user, or any other information of interest.
     *
     * In Sentry, this creates a tag and they are searchable in the Sentry web interface.
     *
     * @note Annotations made before the crash handler are remembered, and sent to the
     * crash handler as soon as it's initialized.
     *
     * @param key Key
     * @param value Value
     */
    void setAnnotation(const std::string &key, const QString &value);

    /**
     * @brief Set an annotation to be added to a crash
     *
     * Annotations add extra information, such as the application's version number,
     * the current user, or any other information of interest.
     *
     * In Sentry, this creates a tag and they are searchable in the Sentry web interface.
     *
     * @note Annotations made before the crash handler are remembered, and sent to the
     * crash handler as soon as it's initialized.
     *
     *
     * @param key Key
     * @param value Value
     */
    void setAnnotation(const std::string &key, const std::string &value);

    /**
     * @brief Attach a tag to the crash report.
     *
     * Tags are always string values, and are searchable.
     *
     * @param name Key
     * @param value Value
     */
    virtual void setTag(std::string name, std::string value) = 0;

    /**
     * @brief Add context to the crash report
     *
     * Contexts in Sentry are informative, and not normally searchable. This should be used
     * to provide detailed information that won't normally be used for locating reports.
     *
     * Unlike tags, contexts can be other things than string types.
     *
     * @param sectionName
     * @param key
     * @param value
     */
    virtual void setContext(const QString &sectionName, const QString &key, const QVariant &value) = 0;


    /**
     * @brief Log a message to the crash handler
     *
     * This can be called from the logging system, to add log messages
     * to crash reports.
     *
     * @param type Qt Log message type
     * @param context Qt logging context
     * @param msg Message to log
     */
    void logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    /**
     * @brief Returns the support tag.
     *
     * This is a randomly generated per program launch tag that can be used
     * to aid in user support and debugging. The user can be asked to give
     * this tag to a developer. It will be recorded in the crash reports.
     *
     * @return QString
     */
    QString getSupportTag() const { return _supportTag; }

signals:

    /**
     * @brief Emitted when the enabled/disabled state of the crash handler changes
     *
     * This can be used to store it as a setting.
     *
     * @param enabled Whether the crash handler is now enabled
     */
    void enabledChanged(bool enabled);

    /**
     * @brief Emitted when the enabled/disabled state of the log streaming changes
     *
     * This can be used to store it as a setting.
     *
     * @param enabled Whether the log streaming is now enabled
     */
    void logStreamingChanged(bool enabled);

    /**
     * @brief Emitted when the enabled/disabled state of the stats streaming changes
     *
     * This can be used to store it as a setting.
     *
     * @param enabled Whether the stats streaming is now enabled
     */
    void statsStreamingChanged(bool enabled);
protected:

    /**
     * @brief Start the crash handler
     *
     * This is implementation dependent, and is called by start().
     *
     */
    virtual bool startCrashHandler() = 0;

    /**
     * @brief Set whether to enable crash reporting
     *
     * This is the implementation dependent part, called by setEnabled().
     *
     * @param value Whether to enable crash reporting
     */
    virtual void setCrashReportingEnabled(bool value) = 0;


    /**
     * @brief Send a log message to the crash handler.
     *
     * This will be logged even without a crash.
     *
     * @param type
     * @param context
     * @param msg
     */
    virtual void sendLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) = 0;


    /**
     * @brief Marks the crash monitor as started
     *
     * @warning Only to be used as part of the startup process
     *
     * @param started
     */
    void setStarted(bool started) { _crashMonitorStarted = started; }

    // Move this to PathUtils
    static QString findBinaryDir();

    CrashHandler(QObject *parent = nullptr);


private:



    std::atomic<bool> _crashMonitorStarted {false};
    std::atomic<bool> _crashReportingEnabled {false};
    std::atomic<bool> _logStreamingEnabled {false};
    std::atomic<bool> _statsStreamingEnabled {false};

    std::unordered_map<std::string, std::string> _annotations{};
    std::mutex _annotationsMutex{};
    std::mutex _logMutex{};

    QString _path;
    QString _crashUrl{OVERTE_BACKTRACE_URL};
    QString _crashToken{OVERTE_BACKTRACE_TOKEN};

    QString _previousMessage{};
    int _repeatCount { 0 };

    QString _supportTag{};

    static const QStringList EFF_SHORT_WORDLIST;

};

#endif


