//
//  CrashHandler.cpp
//
//
//  Created by Dale Glass on 25/06/2023.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once

#include <QObject>
#include <QCoreApplication>
#include <SettingHandle.h>




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
     *  * Crash reporter not being configured with reporting URLs (CMAKE_BACKTRACE_TOKEN and CMAKE_BACKTRACE_URL)
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
     * @return true Crashes will be reported to CMAKE_BACKTRACE_URL
     * @return false Crashes will not be reported
     */
    bool isEnabled() const { return _crashReportingEnabled; }

    /**
     * @brief Set whether we want to submit crash reports to the report server
     *
     * The report server is configured with CMAKE_BACKTRACE_URL.
     * Emits crashReportingEnabledChanged signal.
     *
     * @note This automatically calls start(), so it should be called after setPath(), setUrl() and setToken()
     * @param enabled Whether it's enabled.
     */
    void setEnabled(bool enabled);

    /**
     * @brief Set the URL where to send crash reports to
     *
     * If not set, a predefined URL specified at compile time via CMAKE_BACKTRACE_URL
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
     * If not set, a predefined token specified at compile time via CMAKE_BACKTRACE_TOKEN
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
     * @param key Key
     * @param value Value
     */
    void setAnnotation(const std::string &key, const std::string &value);

signals:

    /**
     * @brief Emitted when the enabled/disabled state of the crash handler changes
     *
     * This can be used to store it as a setting.
     *
     * @param enabled Whether the crash handler is now enabled
     */
    void enabledChanged(bool enabled);

private:
    CrashHandler(QObject *parent = nullptr);


    /**
     * @brief Marks the crash monitor as started
     *
     * @warning Only to be used as part of the startup process
     *
     * @param started
     */
    void setStarted(bool started) { _crashMonitorStarted = started; }


    bool _crashMonitorStarted {false};
    bool _crashReportingEnabled {false};

    QString _path;
    QString _crashUrl;
    QString _crashToken;
};


