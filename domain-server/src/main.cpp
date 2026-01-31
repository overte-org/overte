//
//  main.cpp
//  domain-server/src
//
//  Created by Philip Rosedale on 11/20/12.
//  Copyright 2012 High Fidelity, Inc.
//
//  The Domain Server keeps a list of nodes that have connected to it, and echoes that list of
//  nodes out to nodes when they check in.
//
//  The connection is stateless... the domain server will set you inactive if it does not hear from
//  you in LOGOFF_CHECK_INTERVAL milliseconds, meaning your info will not be sent to other users.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <BuildInfo.h>
#include <CrashAnnotations.h>
#include <LogHandler.h>
#include <SharedUtil.h>

#include "DomainServer.h"
#include <crash-handler/CrashHandler.h>


int main(int argc, char* argv[]) {
    setupHifiApplication(BuildInfo::DOMAIN_SERVER_NAME);

    QJsonObject domainSettingsToSet;
    DomainServer::parseCommandLine(argc, argv, domainSettingsToSet);

    Setting::init();

    int currentExitCode = 0;

    // use a do-while to handle domain-server restart
    auto &ch = CrashHandler::getInstance();
    ch.setPath(argv[0]);

    if ( DomainServer::forceCrashReporting() ) {
        ch.setEnabled(true);
    }

    ch.setAnnotation("program", "domain-server");


    do {
        crash::annotations::setShutdownState(false);
        DomainServer domainServer(argc, argv, domainSettingsToSet);
        domainSettingsToSet = QJsonObject();
        ch.startMonitor(&domainServer);

        currentExitCode = domainServer.exec();
    } while (currentExitCode == DomainServer::EXIT_CODE_REBOOT);

    qInfo() << "Quitting.";
    return currentExitCode;
}
