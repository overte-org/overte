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


CrashHandler& CrashHandler::getInstance() {
    static CrashHandler sharedInstance;
    return sharedInstance;
}

bool CrashHandler::start(const QString &path) {
    if (isStarted()) {
        qCWarning(crash_handler) << "Crash handler already started";
        return false;
    }

    auto started = startCrashHandler(path.toStdString());
    setStarted(started);

    if ( started ) {
        qCInfo(crash_handler) << "Crash handler started";
    } else {
        qCWarning(crash_handler) << "Crash handler failed to start";
    }

    return started;
}

void CrashHandler::startMonitor(QCoreApplication *app) {
    startCrashHookMonitor(app);
}

void CrashHandler::setEnabled(bool enabled) {
    if (enabled != _crashReportingEnabled.get()) {
        _crashReportingEnabled.set(enabled);

        setCrashReportingEnabled(enabled);
    }
}

void CrashHandler::setAnnotation(const std::string &key, const char *value) {
    setCrashAnnotation(key, std::string(value));
}

void CrashHandler::setAnnotation(const std::string &key, const QString &value) {
    setCrashAnnotation(key, value.toStdString());
}

void CrashHandler::setAnnotation(const std::string &key, const std::string &value) {
    setCrashAnnotation(key, value);
}