//
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include <QSignalSpy>
#include <QDebug>
#include <QFile>
#include <QTextStream>


#include "ScriptEngineNetworkedTests.h"
#include "DependencyManager.h"

#include "ScriptEngines.h"
#include "ScriptEngine.h"
#include "ScriptCache.h"
#include "ScriptManager.h"
#include "ScriptEngines.h"
#include "AddressManager.h"
#include "AccountManager.h"
#include "DomainAccountManager.h"
#include "ResourceManager.h"
#include "ResourceRequestObserver.h"
#include "StatTracker.h"
#include "MessagesClient.h"
#include "ResourceScriptingInterface.h"
#include "UserActivityLogger.h"
#include "UserActivityLoggerScriptingInterface.h"
#include "EntityScriptingInterface.h"
#include "ResourceManager.h"
#include "NodeList.h"
#include "../../../libraries/entities/src/EntityScriptingInterface.h"

QTEST_MAIN(ScriptEngineNetworkedTests)



void ScriptEngineNetworkedTests::initTestCase() {
    // AudioClient starts networking, but for the purposes of the tests here we don't care,
    // so just got to use some port.
    int listenPort = 10000;

    DependencyManager::registerInheritance<LimitedNodeList, NodeList>();

    DependencyManager::set<AccountManager>(true); // use the default user agent getter
    DependencyManager::set<DomainAccountManager>();
    DependencyManager::set<AddressManager>();
    DependencyManager::set<NodeList>(NodeType::Agent, listenPort);

    DependencyManager::set<ScriptEngines>(ScriptManager::CLIENT_SCRIPT, QUrl(""));
    DependencyManager::set<ScriptCache>();
   // DependencyManager::set<ResourceManager>();
   // DependencyManager::set<ResourceRequestObserver>();
    DependencyManager::set<StatTracker>();
    DependencyManager::set<ScriptInitializers>();
   // DependencyManager::set<EntityScriptingInterface>(true);

    DependencyManager::set<MessagesClient>();
    DependencyManager::set<ResourceScriptingInterface>();
    DependencyManager::set<UserActivityLoggerScriptingInterface>();
    DependencyManager::set<EntityScriptingInterface>(true);
    DependencyManager::set<ResourceManager>();
    DependencyManager::set<ResourceRequestObserver>();


    auto nodeList = DependencyManager::get<NodeList>();
    nodeList->startThread();
    nodeList->setFlagTimeForConnectionStep(true);

}

ScriptManagerPointer ScriptEngineNetworkedTests::makeManager(const QString &scriptSource, const QString &scriptFilename) {
    ScriptManagerPointer sm = scriptManagerFactory(ScriptManager::CLIENT_SCRIPT, scriptSource, scriptFilename);


    sm->setAbortOnUncaughtException(true);

    connect(sm.get(), &ScriptManager::scriptLoaded, [](const QString& filename){
        qWarning() << "Loaded script" << filename;
    });


    connect(sm.get(), &ScriptManager::errorLoadingScript, [](const QString& filename){
        qWarning() << "Failed to load script" << filename;
    });

    connect(sm.get(), &ScriptManager::printedMessage, [](const QString& message, const QString& engineName){
        qDebug() << "Printed message from engine" << engineName << ": " << message;
    });

    connect(sm.get(), &ScriptManager::infoMessage, [](const QString& message, const QString& engineName){
        qInfo() << "Info message from engine" << engineName << ": " << message;
    });

    connect(sm.get(), &ScriptManager::warningMessage, [](const QString& message, const QString& engineName){
        qWarning() << "Warning from engine" << engineName << ": " << message;
    });

    connect(sm.get(), &ScriptManager::errorMessage, [](const QString& message, const QString& engineName){
        qCritical() << "Error from engine" << engineName << ": " << message;
    });

    connect(sm.get(), &ScriptManager::finished, [](const QString& fileNameString, ScriptManagerPointer smp){
        qInfo() << "Finished running script" << fileNameString;
    });

    connect(sm.get(), &ScriptManager::runningStateChanged, [sm](){
        qInfo() << "Running state changed. Running = " << sm->isRunning() << "; Stopped = " << sm->isStopped() << "; Finished = " << sm->isFinished();
    });

    connect(sm.get(), &ScriptManager::unhandledException, [](std::shared_ptr<ScriptException> exception){
        qWarning() << "Exception from engine: " << exception;
    });


    return sm;
}

void ScriptEngineNetworkedTests::testRequire() {
    auto sm = makeManager(
        "print(\"Starting\");"
        "Script.require('./tests/c.js');"
        "print(\"Done\");"
        "Script.stop(true);", "testRequire.js");
    QStringList printed;
    QStringList expected {"Starting", "Value from A: 6", "Value from B: 6", "Done"};


    QVERIFY(!sm->isRunning());
    QVERIFY(!sm->isStopped());
    QVERIFY(!sm->isFinished());

    connect(sm.get(), &ScriptManager::printedMessage, [&printed](const QString& message, const QString& engineName){
        printed.append(message);
    });


    qInfo() << "About to run script";
    sm->run();

    QVERIFY(!sm->isRunning());
    QVERIFY(!sm->isStopped());
    QVERIFY(sm->isFinished());

    QVERIFY(printed.length() == expected.length());
    for(int i=0;i<printed.length();i++) {
        QString nomatch = QString("Result '%1' didn't match expected '%2'").arg(printed[i]).arg(expected[i]);
        QVERIFY2(printed[i] == expected[i], qPrintable(nomatch));
    }
}



