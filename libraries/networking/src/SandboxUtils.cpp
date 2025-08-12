//
//  SandboxUtils.cpp
//  libraries/networking/src
//
//  Created by Brad Hefta-Gaub on 2016-10-15.
//  Copyright 2016 High Fidelity, Inc.
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SandboxUtils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QProcess>

#if !defined(Q_OS_WIN)
#include <QMessageBox>
#include <csignal>
#endif

#include <NumericalConstants.h>
#include <SharedUtil.h>
#include <RunningMarker.h>

#include "NetworkAccessManager.h"
#include "NetworkLogging.h"
#include "NetworkingConstants.h"

namespace SandboxUtils {

QNetworkReply* getStatus() {
    auto& networkAccessManager = NetworkAccessManager::getInstance();
    QNetworkRequest sandboxStatus(SANDBOX_STATUS_URL);
    sandboxStatus.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    sandboxStatus.setHeader(QNetworkRequest::UserAgentHeader, NetworkingConstants::OVERTE_USER_AGENT);
    return networkAccessManager.get(sandboxStatus);
}

bool readStatus(QByteArray statusData) {
    auto statusJson = QJsonDocument::fromJson(statusData);

    if (!statusJson.isEmpty()) {
        auto statusObject = statusJson.object();
        auto serversValue = statusObject.value("servers");
        if (!serversValue.isUndefined() && serversValue.isObject()) {
            auto serversObject = serversValue.toObject();
            auto serversCount = serversObject.size();
            const int MINIMUM_EXPECTED_SERVER_COUNT = 5;
            if (serversCount >= MINIMUM_EXPECTED_SERVER_COUNT) {
                return true;
            }
        }
    }

    return false;
}

void runLocalSandbox(QString contentPath, bool autoShutdown, bool noUpdater) {
    // use the current behavior on windows to not break anyone's setup
#if defined (Q_OS_WIN)
    QString serverPath = "./server-console/server-console.exe";
    qCDebug(networking) << "Server path is: " << serverPath;
    qCDebug(networking) << "autoShutdown: " << autoShutdown;
    qCDebug(networking) << "noUpdater: " << noUpdater;

    bool hasContentPath = !contentPath.isEmpty();
    bool passArgs = autoShutdown || hasContentPath || noUpdater;

    QStringList args;

    if (passArgs) {
        args << "--";
    }

    if (hasContentPath) {
        QString serverContentPath = "./" + contentPath;
        args << "--contentPath" << serverContentPath;
    }

    if (autoShutdown) {
        auto pid = QCoreApplication::applicationPid();
        args << "--shutdownWith" << QString::number(pid);
    }

    if (noUpdater) {
        args << "--noUpdater";
    }

    qCDebug(networking) << "Launching sandbox with:" << args;
    qCDebug(networking) << QProcess::startDetached(serverPath, args);
#else
    auto myPid = QCoreApplication::applicationPid();
    QStringList domainArgs, assignmentArgs;

    qint64 domainPid = 0, assignmentPid = 0;

    qCDebug(networking) << "Launching sandbox";

    domainArgs << "--parent-pid" << QString::number(myPid);
    domainArgs << "--get-temp-name";

    auto domainSuccess = QProcess::startDetached(
        "./domain-server/domain-server",
        domainArgs,
        QString(),
        &domainPid
    );

    if (domainSuccess) {
        qCDebug(networking) << "Sandbox domain-server started";
    } else {
        qCCritical(networking) << "Sandbox domain-server couldn't be started";
        // safe to use QMessageBox because SandboxUtils is only used by interface
        QMessageBox::critical(nullptr, "Sandbox Server Error", "The domain-server executable couldn't be started. Overte will continue running without the sandbox server.");
        return;
    }

    assignmentArgs << "--parent-pid" << QString::number(myPid);
    assignmentArgs << "-n7";

    auto assignmentSuccess = QProcess::startDetached(
        "./assignment-client/assignment-client",
        assignmentArgs,
        QString(),
        &assignmentPid
    );

    if (assignmentSuccess) {
        qCDebug(networking) << "Sandbox assignment-client started";
    } else {
        qCCritical(networking) << "Sandbox assignment-client failed to start";

        // kill the domain server if we can't start the assignment client
        if (domainPid) { kill(domainPid, SIGINT); }

        // safe to use QMessageBox because SandboxUtils is only used by interface
        QMessageBox::critical(nullptr, "Sandbox Server Error", "The assignment-client executable couldn't be started. Overte will continue running without the sandbox server.");
    }
#endif
}

}
