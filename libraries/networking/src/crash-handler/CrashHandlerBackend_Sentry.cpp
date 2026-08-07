//
//  CrashHandler_Sentry.cpp
//  interface/src
//
//  Created by Vadim Troschinsky on 05/08/2026.
//  Copyright 2018 High Fidelity, Inc.
//  Copyright 2025 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#define HAS_SENTRY 1

#if HAS_SENTRY


#include "sentry.h"
#include "CrashHandler.h"

Q_LOGGING_CATEGORY(crash_handler, "overte.crash_handler")

#include <assert.h>

#include <vector>
#include <string>

#include <QtCore/QAtomicInteger>
#include <QtCore/QDeadlineTimer>
#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QString>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++14-extensions"
#endif


#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include <BuildInfo.h>
#include "../FingerprintUtils.h"
#include "../UserActivityLogger.h"
#include <UUID.h>



/**
 * @brief Sentry implementation
 *
 * Sentry is a crash reporting and metrics system.
 *
 * Documentation for the API is at https://docs.sentry.io/platforms/native/usage/
 *
 * For reporting, we use the sentry DSN, not the minidump endpoint.
 *
 *
 */

static const std::string BACKTRACE_URL{ OVERTE_BACKTRACE_URL };
static const std::string BACKTRACE_TOKEN{ OVERTE_BACKTRACE_TOKEN };




#if defined(Q_OS_WIN)
static const QString CRASHPAD_HANDLER_NAME{ "crashpad_handler.exe" };
#else
static const QString CRASHPAD_HANDLER_NAME{ "crashpad_handler" };
#endif

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif



// Locate the full path to the binary's directory
static QString findBinaryDir() {
    // Normally we'd just use QCoreApplication::applicationDirPath(), but we can't.
    // That function needs the QApplication to be created first, and Crashpad is initialized as early as possible,
    // which is well before QApplication, so that function throws out a warning and returns ".".
    //
    // So we must do things the hard way here. In particular this is needed to correctly handle things in AppImage
    // on Linux. On Windows and MacOS falling back to argv[0] should be fine.

#ifdef Q_OS_LINUX
    // Find outselves by looking at /proc/<PID>/exe
    pid_t ourPid = getpid();
    QString exeLink = QString("/proc/%1/exe").arg(ourPid);
    qCDebug(crash_handler) << "Looking at" << exeLink;

    QFileInfo exeLinkInfo(exeLink);
    if (exeLinkInfo.isSymLink()) {
        QFileInfo exeInfo(exeLinkInfo.symLinkTarget());
        qCDebug(crash_handler) << "exe symlink points at" << exeInfo;
        return exeInfo.absoluteDir().absolutePath();
    } else {
        qCWarning(crash_handler) << exeLink << "isn't a symlink. /proc not mounted?";
    }

#endif

    return QString();
}

bool startCrashHandler(std::string appPath, std::string crashURL, std::string crashToken) {
    if (crashURL.empty()) {
        crashURL = BACKTRACE_URL;
    }

    if (crashToken.empty()) {
        crashToken = BACKTRACE_TOKEN;
    }

    if (crashURL.empty() || crashToken.empty()) {
        qCCritical(crash_handler) << "Backtrace URL or token not set, crash handler disabled.";
        return false;
    }

    sentry_options_t *options = sentry_options_new();
    sentry_options_set_dsn(options, crashURL.c_str());
    sentry_options_set_release(options, crashToken.c_str());
    sentry_options_set_debug(options, 1); // Debug for Sentry Native SDK itself
    sentry_options_set_enable_logs(options, 1);
    sentry_options_set_minidump_mode(options, SENTRY_MINIDUMP_MODE_FULL); // TODO: a way to choose SENTRY_MINIDUMP_MODE_SMART, which sends smaller reports

    if (sentry_init(options) == 0) {
        qCInfo(crash_handler) << "Sentry init successful";
    } else {
        qCCritical(crash_handler) << "Sentry init failed";
        return false;
    }


    sentry_value_t release_info = sentry_value_new_object();
    sentry_value_set_by_key(release_info, "version", sentry_value_new_string(BuildInfo::VERSION.toStdString().c_str()));
    sentry_value_set_by_key(release_info, "build", sentry_value_new_string(BuildInfo::BUILD_NUMBER.toStdString().c_str()));

    sentry_value_t machine_info = sentry_value_new_object();
    sentry_value_set_by_key(machine_info, "fingerprint", sentry_value_new_string(uuidStringWithoutCurlyBraces(FingerprintUtils::getMachineFingerprint()).toStdString().c_str()))    ;


    sentry_set_context("Release", release_info);
    sentry_set_context("Machine", machine_info);


    qCInfo(crash_handler) << "Sentry crash handler initialized. SDK version" << sentry_sdk_version() << ". Will send reports to " << crashURL.c_str() << " with token " << crashToken.c_str();
    return true;

    //qCInfo(crash_handler) << "Crashpad uploads " << (enabled ? QString("enabled") : QString("disabled"));
}

void setCrashAnnotation(std::string name, std::string value) {

}

void startCrashHookMonitor(QCoreApplication* app) {

}


void CrashHandler::logMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    // Some of the logging is very spammy and Sentry seems to have a problem with shutting down
    // in that case.
    {
        std::lock_guard<std::mutex> lock(_logMutex);

        if (msg == _previousMessage) {
            _repeatCount++;
            return;
        }

        if (_repeatCount > 0) {
            sentry_log_warn("Previous log message repeated %i times", _repeatCount);
        }

        _previousMessage = msg;
        _repeatCount = 0;
    }


    switch(type) {
        case QtMsgType::QtFatalMsg:
            sentry_log_fatal(msg.toStdString().c_str());
            break;
        case QtMsgType::QtCriticalMsg:
            sentry_log_error(msg.toStdString().c_str());
            break;
        case QtMsgType::QtWarningMsg:
            sentry_log_warn(msg.toStdString().c_str());
            break;
        case QtMsgType::QtInfoMsg:
            sentry_log_info(msg.toStdString().c_str());
            break;
        case QtMsgType::QtDebugMsg:
            sentry_log_debug(msg.toStdString().c_str());
            break;
        default:
            sentry_log_warn(msg.toStdString().c_str());
            sentry_log_warn("Previous log message was of unknown type %i", (int)type);
    }
}

#endif  // HAS_SENTRY
