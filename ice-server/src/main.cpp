//
//  main.cpp
//  ice-server/src
//
//  Created by Stephen Birarda on 10/01/12.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QtCore/QCoreApplication>
#include <QDebug>

#include <SharedUtil.h>

#include "IceServer.h"
#include "PathUtils.h"

int main(int argc, char* argv[]) {
    setupHifiApplication("Ice Server");
    IceServer iceServer(argc, argv);

    if (!PathUtils::initialize()) {
        qCritical() << "Failed to initialize PathUtils";
        return 80; // 'P'
    }

    return iceServer.exec();
}
