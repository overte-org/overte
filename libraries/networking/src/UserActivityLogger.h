//
//  UserActivityLogger.h
//
//
//  Created by Clement on 5/21/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_UserActivityLogger_h
#define hifi_UserActivityLogger_h

#include "AccountManager.h"

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QNetworkReply>
#include <QElapsedTimer>

#include <SettingHandle.h>
#include "AddressManager.h"

const QString USER_ACTIVITY_URL = "/api/v1/user_activities";

class UserActivityLogger : public QObject {
    Q_OBJECT

public:
    static UserActivityLogger& getInstance();

public slots:
    bool isEnabled() { return !_disabled.get(); }
    bool isDisabledSettingSet() const { return _disabled.isSet(); }


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
    bool isCrashMonitorStarted() const { return _crashMonitorStarted; }


    /**
     * @brief Whether the crash monitor will report crashes if they occur
     *
     * This setting is independent of isCrashMonitorStarted() -- crash reporting may be enabled but fail to work
     * due to the crash reporting component being missing or failing to initialize.
     *
     * @return true Crashes will be reported to CMAKE_BACKTRACE_URL
     * @return false Crashes will not be reported
     */
    bool isCrashReportingEnabled() { return _crashReportingEnabled.get(); }

    /**
     * @brief Marks the crash monitor as started
     *
     * @warning Only to be used as part of the startup process
     *
     * @param started
     */
    void setCrashMonitorStarted(bool started) { _crashMonitorStarted = started; }

    /**
     * @brief Set whether we want to submit crash reports to the report server
     *
     * The report server is configured with CMAKE_BACKTRACE_URL.
     * Emits crashReportingEnabledChanged signal.
     *
     * @param enabled Whether it's enabled.
     */
    void setCrashReportingEnabled(bool enabled);

    void disable(bool disable);
    void logAction(QString action, QJsonObject details = QJsonObject(), JSONCallbackParameters params = JSONCallbackParameters());

    void launch(QString applicationVersion, bool previousSessionCrashed, int previousSessionRuntime);

    void insufficientGLVersion(const QJsonObject& glData);

    void changedDisplayName(QString displayName);
    void changedModel(QString typeOfModel, QString modelURL);
    void changedDomain(QString domainURL);
    void connectedDevice(QString typeOfDevice, QString deviceName);
    void loadedScript(QString scriptName);
    void wentTo(AddressManager::LookupTrigger trigger, QString destinationType, QString destinationName);

signals:

    /**
     * @brief The crash reporting setting has been changed.
     *
     * This signal is used by the crash reporter to enable/disable itself.
     *
     */
    void crashReportingEnabledChanged();

private slots:
    void requestError(QNetworkReply* errorReply);

private:
    UserActivityLogger();
    Setting::Handle<bool> _disabled { "UserActivityLoggerDisabled", true };
    Setting::Handle<bool> _crashReportingEnabled { "CrashReportingEnabled", false };

    bool _crashMonitorStarted {false};
    QElapsedTimer _timer;
};

#endif // hifi_UserActivityLogger_h
