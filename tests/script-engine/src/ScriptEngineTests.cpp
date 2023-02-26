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


#include "ScriptEngineTests.h"
#include "DependencyManager.h"

#include "ScriptEngines.h"
#include "ScriptEngine.h"
#include "ScriptCache.h"
#include "ScriptManager.h"

#include "ResourceManager.h"
#include "ResourceRequestObserver.h"
#include "StatTracker.h"

#include "NodeList.h"
#include "../../../libraries/entities/src/EntityScriptingInterface.h"
//#include "../../../libraries/entities/src/EntityScriptingInterface.h"

QTEST_MAIN(ScriptEngineTests)



void ScriptEngineTests::initTestCase() {
    // AudioClient starts networking, but for the purposes of the tests here we don't care,
    // so just got to use some port.
    //int listenPort = 10000;

    //DependencyManager::registerInheritance<LimitedNodeList, NodeList>();
    //DependencyManager::set<NodeList>(NodeType::Agent, listenPort);
    DependencyManager::set<ScriptEngines>(ScriptManager::NETWORKLESS_TEST_SCRIPT, QUrl(""));
    DependencyManager::set<ScriptCache>();
   // DependencyManager::set<ResourceManager>();
   // DependencyManager::set<ResourceRequestObserver>();
    DependencyManager::set<StatTracker>();
    DependencyManager::set<ScriptInitializers>();
   // DependencyManager::set<EntityScriptingInterface>(true);

    QSharedPointer<ScriptEngines> ac = DependencyManager::get<ScriptEngines>();
    QVERIFY(!ac.isNull());

    connect(ac.get(), &ScriptEngines::scriptLoadError, [](const QString& filename, const QString& error){
        qWarning() << "Failed to load script" << filename << ":" << error;
    });

    connect(ac.get(), &ScriptEngines::printedMessage, [](const QString& message, const QString& engineName){
        qDebug() << "Printed message from engine" << engineName << ": " << message;
    });

    connect(ac.get(), &ScriptEngines::infoMessage, [](const QString& message, const QString& engineName){
        qInfo() << "Info message from engine" << engineName << ": " << message;
    });

    connect(ac.get(), &ScriptEngines::warningMessage, [](const QString& message, const QString& engineName){
        qWarning() << "Warning from engine" << engineName << ": " << message;
    });

    connect(ac.get(), &ScriptEngines::errorMessage, [](const QString& message, const QString& engineName){
        qCritical() << "Error from engine" << engineName << ": " << message;
    });


}

void ScriptEngineTests::scriptTest() {
    QSharedPointer<ScriptEngines> ac = DependencyManager::get<ScriptEngines>();
    QVERIFY(!ac.isNull());


    QDir testScriptsDir("tests");
    QStringList testScripts = testScriptsDir.entryList(QStringList() << "*.js", QDir::Files);
    testScripts.sort();

    for(QString scriptFilename : testScripts) {
        scriptFilename = "tests/" + scriptFilename;
        qInfo() << "Running test script: " << scriptFilename;

        QString scriptSource;

        {
            QFile scriptFile(scriptFilename);
            scriptFile.open(QIODevice::ReadOnly);
            QTextStream scriptStream(&scriptFile);
            scriptSource.append(scriptStream.readAll());

            // Scripts keep on running until Script.stop() is called. For our tests here,
            // that's not desirable, so we append an automatic stop at the end of every
            // script.
            scriptSource.append("\nScript.stop(true);\n");
        }


        //qDebug() << "Source: " << scriptSource;

        ScriptManagerPointer sm = newScriptManager(ScriptManager::NETWORKLESS_TEST_SCRIPT, scriptSource, scriptFilename);


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


        sm->run();
    }

    //TODO: Add a test for Script.require(JSON)
}
