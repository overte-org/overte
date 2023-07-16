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
#include "CrashHandlerBackend.h"
#include <QFileInfo>
#include <QCoreApplication>


CrashHandler& CrashHandler::getInstance() {
    static CrashHandler sharedInstance;
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

    auto started = startCrashHandler(_path.toStdString(), _crashUrl.toStdString(), _crashToken.toStdString());
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

    } else {
        qCWarning(crash_handler) << "Crash handler failed to start";
    }

    return started;
}

void CrashHandler::startMonitor(QCoreApplication *app) {
    startCrashHookMonitor(app);
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