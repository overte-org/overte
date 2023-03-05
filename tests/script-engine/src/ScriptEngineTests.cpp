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


}

ScriptManagerPointer ScriptEngineTests::makeManager(const QString &scriptSource, const QString &scriptFilename) {
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

    connect(sm->engine().get(), &ScriptEngine::exception, [](std::shared_ptr<ScriptException> exception){
        qWarning() << "Exception from engine (direct): " << exception;
    });


    connect(sm.get(), &ScriptManager::unhandledException, [](std::shared_ptr<ScriptException> exception){
        qWarning() << "Exception from engine: " << exception;
    });


    return sm;
}

void ScriptEngineTests::testTrivial() {
    auto sm = makeManager("print(\"script works!\"); Script.stop(true);", "testTrivial.js");
    QString printed;

    QVERIFY(!sm->isRunning());
    QVERIFY(!sm->isStopped());
    QVERIFY(!sm->isFinished());


    connect(sm.get(), &ScriptManager::printedMessage, [&printed](const QString& message, const QString& engineName){
        printed.append(message);
    });


    sm->run();

    QVERIFY(!sm->isRunning());
    QVERIFY(!sm->isStopped());
    QVERIFY(sm->isFinished());
    QVERIFY(printed == "script works!");

}

void ScriptEngineTests::testSyntaxError() {
    auto sm = makeManager("this is not good syntax", "testSyntaxError.js");
    bool exceptionHappened = false;


    //QSignalSpy spy(sm.get(), &ScriptManager::unhandledException);


    connect(sm.get(), &ScriptManager::unhandledException, [&exceptionHappened](std::shared_ptr<ScriptException> exception){
        exceptionHappened = true;
    });


    sm->run();
    //spy.wait(1000);

    std::shared_ptr<ScriptException> ex = sm->getUncaughtException();

    qDebug() << "Exception:" << ex;

    QVERIFY(exceptionHappened);
    QVERIFY(ex);

    //QVERIFY(spy.count() == 1);
}

void ScriptEngineTests::scriptTest() {
    return;

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

        ScriptManagerPointer sm = makeManager(scriptSource, scriptFilename);
        sm->run();
    }

    //TODO: Add a test for Script.require(JSON)
}
