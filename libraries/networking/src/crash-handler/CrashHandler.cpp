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

#include "CrashHandler.h"
#include "CrashHandlerNone.h"
#include "CrashHandlerSentry.h"

#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

#include "LogHandler.h"

#ifdef Q_OS_LINUX
#include <unistd.h>
#endif

Q_LOGGING_CATEGORY(crash_handler, "overte.crash_handler")


static void crashHandlerLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);


CrashHandler& CrashHandler::getInstance() {
#if HAS_SENTRY
    static CrashHandlerSentry sharedInstance;
#else
    static CrashHandlerNone sharedInstance;
#endif

    return sharedInstance;
}

CrashHandler::CrashHandler(QObject *parent) : QObject(parent) {

}


void CrashHandler::setPath(const QString &path) {
    QFileInfo fi(path);

    if (isStarted()) {
        qCWarning(crash_handler) << "Crash handler already started, too late to set the path.";
    }

    if (fi.isFile()) {
        _path = fi.absolutePath();
    } else {
        _path = path;
    }
}

bool CrashHandler::start() {
    if (isStarted()) {
        //qCWarning(crash_handler) << "Crash handler already started";
        return false;
    }

    auto started = startCrashHandler();
    setStarted(started);

    if ( started ) {
        qCInfo(crash_handler) << "Crash handler started";
        std::size_t countAdded = 0;

        {
            std::lock_guard<std::mutex> lock(_annotationsMutex);
            for(const auto &item : _annotations) {
                setCrashAnnotation(item.first, item.second);
            }

            countAdded = _annotations.size();
            _annotations.clear();
        }

        qCDebug(crash_handler) << "Forwarded" << countAdded << "annotations";


        qInstallMessageHandler(crashHandlerLogMessage);
        qCInfo(crash_handler) << "Installed crash handler log message handler";


    } else {
        qCWarning(crash_handler) << "Crash handler failed to start";
    }

    return started;
}

void CrashHandler::startMonitor(QCoreApplication *app) {
   // startCrashHookMonitor(app);
}

void CrashHandler::setEnabled(bool enabled) {
    start();

    if (enabled != _crashReportingEnabled) {
        _crashReportingEnabled = enabled;
        setCrashReportingEnabled(enabled);

        emit enabledChanged(enabled);
    }
}

void CrashHandler::setUrl(const QString &url) {
    // This can be called both from the settings system in an assignment client
    // and from the commandline parser. We only emit a warning if the commandline
    // argument causes the domain setting to be ignored.

    if (isStarted() && url != _crashUrl) {
        qCWarning(crash_handler) << "Setting crash reporting URL to " << url << "after the crash handler is already running has no effect";
    } else {
        _crashUrl = url;
    }
}

void CrashHandler::setToken(const QString &token) {
    if (isStarted() && token != _crashToken) {
        qCWarning(crash_handler) << "Setting crash reporting token to " << token << "after the crash handler is already running has no effect";
    } else {
        _crashToken = token;
    }
}

void CrashHandler::setAnnotation(const std::string &key, const char *value) {
    setAnnotation(key, std::string(value));
}

void CrashHandler::setAnnotation(const std::string &key, const QString &value) {
    setAnnotation(key, value.toStdString());
}

void CrashHandler::setAnnotation(const std::string &key, const std::string &value) {
    if (!isStarted()) {
        std::lock_guard<std::mutex> lock(_annotationsMutex);
        _annotations[key] = value;
        return;
    }

    setCrashAnnotation(key, value);
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
            sendLogMessage(QtMsgType::QtWarningMsg, QMessageLogContext(), QString("Previous log message repeated %1 times").arg(_repeatCount));
        }

        _previousMessage = msg;
        _repeatCount = 0;
    }

    sendLogMessage(type, context, msg);
}

// Locate the full path to the binary's directory
QString CrashHandler::findBinaryDir() {
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


static void crashHandlerLogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    CrashHandler::getInstance().logMessage(type, context, msg);

    // We only get one log handler, so call the normal one here since we replaced it
    LogHandler::getInstance().verboseMessageHandler(type, context, msg);
}
