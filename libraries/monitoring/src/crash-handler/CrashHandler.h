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
 * This object initializes and talks to crash reporting backends.
 *
 */
class CrashHandler : public QObject {
    Q_OBJECT

public:
    static CrashHandler& getInstance();

public slots:

    /**
     * @brief Start the crash handler
     *
     * @param path Database path
     * @return true Started successfully
     * @return false Failed to start
     */
    bool start(const QString &path);


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
    bool isEnabled() const { return _crashReportingEnabled.get(); }

    /**
     * @brief Set whether we want to submit crash reports to the report server
     *
     * The report server is configured with CMAKE_BACKTRACE_URL.
     * Emits crashReportingEnabledChanged signal.
     *
     * @param enabled Whether it's enabled.
     */
    void setEnabled(bool enabled);


    void setAnnotation(const std::string &key, const char *value);
    void setAnnotation(const std::string &key, const QString &value);
    void setAnnotation(const std::string &key, const std::string &value);


private:
    /**
     * @brief Marks the crash monitor as started
     *
     * @warning Only to be used as part of the startup process
     *
     * @param started
     */
    void setStarted(bool started) { _crashMonitorStarted = started; }



    Setting::Handle<bool> _crashReportingEnabled { "CrashReportingEnabled", false };
    bool _crashMonitorStarted {false};
};


