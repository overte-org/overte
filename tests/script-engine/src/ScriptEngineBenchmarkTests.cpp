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


#include "ScriptEngineBenchmarkTests.h"
#include "DependencyManager.h"

#include "ScriptEngines.h"
#include "ScriptEngine.h"
#include "ScriptCache.h"
#include "ScriptManager.h"

#include "v8/ScriptObjectV8Proxy.h"
#include "v8/ScriptEngineV8.h"

#include "ResourceManager.h"
#include "ResourceRequestObserver.h"
#include "StatTracker.h"

#include "NodeList.h"
#include "../../../libraries/entities/src/EntityScriptingInterface.h"
//#include "../../../libraries/entities/src/EntityScriptingInterface.h"

QTEST_MAIN(ScriptEngineBenchmarkTests)






void ScriptEngineBenchmarkTests::initTestCase() {
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


}

ScriptManagerPointer ScriptEngineBenchmarkTests::makeManager(const QString &scriptSource, const QString &scriptFilename) {
    ScriptManagerPointer sm = newScriptManager(ScriptManager::NETWORKLESS_TEST_SCRIPT, scriptSource, scriptFilename);


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

void ScriptEngineBenchmarkTests::benchmarkSetProperty() {
    auto sm = makeManager("print(\"script works!\"); Script.stop(true);", "testTrivial.js");

    auto engine = sm->engine();
    auto obj = engine->newObject();

    QBENCHMARK {
        engine->setProperty("hello", QVariant(1));
    }


}

void ScriptEngineBenchmarkTests::benchmarkSetProperty1K() {
    auto sm = makeManager("print(\"script works!\"); Script.stop(true);", "testTrivial.js");

    auto engine = sm->engine();
    auto obj = engine->newObject();

    int i = 0;
    char buf[128];

    QBENCHMARK {
        for(i=0;i<1024;i++) {
            sprintf(buf, "%i", i);
            engine->setProperty(buf, QVariant(1));
        }
    }


}

void ScriptEngineBenchmarkTests::benchmarkSetProperty16K() {
    auto sm = makeManager("print(\"script works!\"); Script.stop(true);", "testTrivial.js");

    auto engine = sm->engine();
    auto obj = engine->newObject();

    int i = 0;
    char buf[128];

    QBENCHMARK {
        for(i=0;i<16384;i++) {
            sprintf(buf, "%i", i);
            engine->setProperty(buf, QVariant(1));
        }
    }


}


void ScriptEngineBenchmarkTests::benchmarkQueryProperty() {
    auto sm = makeManager("print(\"script works!\"); Script.stop(true);", "testTrivial.js");

    auto engine = sm->engine();
    auto obj = engine->newObject();



    int i = 0;
    char buf[128];

    QObject dummy;

    ScriptEngineV8 *v8_engine = dynamic_cast<ScriptEngineV8*>(engine.get());

    ScriptObjectV8Proxy proxy(v8_engine, &dummy, false, ScriptEngine::QObjectWrapOptions());

    for(i=0;i<16384;i++) {
        sprintf(buf, "%i", i);
        engine->setProperty(buf, QVariant(1));
    }

    QSKIP("Test not implemented yet");

//    QBENCHMARK {
//        engine->property()
//    }

}


void ScriptEngineBenchmarkTests::benchmarkSimpleScript() {

    QBENCHMARK {
        auto sm = makeManager("print(\"script works!\"); Script.stop(true);", "testTrivial.js");
        QString printed;
        connect(sm.get(), &ScriptManager::printedMessage, [&printed](const QString& message, const QString& engineName){
            printed.append(message);
        });

        sm->run();
    }

}