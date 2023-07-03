//
//  main.cpp
//  tools/oven/src
//
//  Created by Stephen Birarda on 3/28/2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html

#include "OvenCLIApplication.h"
#include "OvenGUIApplication.h"

#include <BuildInfo.h>
#include <SettingInterface.h>
#include <SharedUtil.h>
#include <SettingManager.h>
#include <DependencyManager.h>
#include <crash-handler/CrashHandler.h>
#include <iostream>


// This needs to be run after a QApplication has been created
void postAppInit(QCoreApplication *app, bool enableCrashHandler) {
    Setting::init();

    auto &ch = CrashHandler::getInstance();



    QObject::connect(&ch, &CrashHandler::enabledChanged, [](bool enabled) {
        Settings s;
        s.beginGroup("Crash");
        s.setValue("ReportingEnabled", enabled);
        s.endGroup();
    });


    Settings crashSettings;
    crashSettings.beginGroup("Crash");
    ch.setEnabled(crashSettings.value("ReportingEnabled").toBool() || enableCrashHandler);
    ch.startMonitor(app);
}


int main (int argc, char** argv) {
    setupHifiApplication("Oven");


    DependencyManager::set<Setting::Manager>();

    auto &ch = CrashHandler::getInstance();
    ch.setPath(argv[0]);



    // figure out if we're launching our GUI application or just the simple command line interface
    bool enableCrashHandler = false;
    OvenCLIApplication::parseResult res = OvenCLIApplication::parseCommandLine(argc, argv, &enableCrashHandler);

    switch(res) {
        case OvenCLIApplication::CLIMode:
        {
            OvenCLIApplication app { argc, argv };
            postAppInit(&app, enableCrashHandler);
            return app.exec();
            break;
        }
        case OvenCLIApplication::GUIMode:
        {
            OvenGUIApplication app { argc, argv };
            postAppInit(&app, enableCrashHandler);
            return app.exec();
            break;
        }
    }
}
