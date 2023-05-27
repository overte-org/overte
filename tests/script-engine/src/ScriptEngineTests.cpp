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

    connect(sm.get(), &ScriptManager::unhandledException, [&exceptionHappened](std::shared_ptr<ScriptException> exception){
        exceptionHappened = true;
    });


    sm->run();

    std::shared_ptr<ScriptException> ex = sm->getUncaughtException();

    qDebug() << "Exception:" << ex;

    QVERIFY(exceptionHappened);
    QVERIFY(ex);
    QVERIFY(ex && ex->errorMessage.contains("SyntaxError"));
}


void ScriptEngineTests::testRuntimeError() {
    auto sm = makeManager("nonexisting();", "testRuntimeError.js");
    bool exceptionHappened = false;

    connect(sm.get(), &ScriptManager::unhandledException, [&exceptionHappened](std::shared_ptr<ScriptException> exception){
        exceptionHappened = true;
    });


    sm->run();

    std::shared_ptr<ScriptException> ex = sm->getUncaughtException();

    qDebug() << "Exception:" << ex;

    QVERIFY(exceptionHappened);
    QVERIFY(ex);
    QVERIFY(ex && ex->errorMessage.contains("ReferenceError"));

}

void ScriptEngineTests::testJSThrow() {
    auto sm = makeManager("throw(42);", "testThrow.js");
    sm->run();

    std::shared_ptr<ScriptException> ex = sm->getUncaughtException();

    qDebug() << "Exception:" << ex;

    auto runtime_ex = std::dynamic_pointer_cast<ScriptRuntimeException>(ex);

    QVERIFY(ex);
    QVERIFY(runtime_ex);
    QVERIFY(runtime_ex && runtime_ex->thrownValue.toInt32() == 42);
}

void ScriptEngineTests::testRegisterClass() {
    QString printed;
    auto sm = makeManager("print(testClass.invokableFunc(4)); Script.stop(true);", "testClass.js");
    connect(sm.get(), &ScriptManager::printedMessage, [&printed](const QString& message, const QString& engineName){
        printed.append(message);
    });

    sm->engine()->registerGlobalObject("testClass", new TestClass());

    sm->run();

    auto ex = sm->getUncaughtException();

    QVERIFY(!ex);
    QVERIFY(printed == "14");

}

void ScriptEngineTests::testInvokeNonInvokable() {
    auto sm = makeManager("print(testClass.nonInvokableFunc(4)); Script.stop(true);", "testClass.js");
    sm->engine()->registerGlobalObject("testClass", new TestClass());

    sm->run();
    auto ex = sm->getUncaughtException();

    QVERIFY(ex);
    QVERIFY(ex && ex->errorMessage.contains("TypeError"));
}

void ScriptEngineTests::testRaiseException() {
    auto sm = makeManager("testClass.doRaiseTest(); Script.stop(true);", "testRaise.js");
    sm->engine()->registerGlobalObject("testClass", new TestClass(sm->engine()));

    sm->run();
    auto ex = sm->getUncaughtException();

    QVERIFY(ex);
    QVERIFY(ex && ex->errorMessage.contains("Exception test"));
}

void ScriptEngineTests::testRaiseExceptionAndCatch() {
    QString script =
        "try {"
        "    testClass.doRaiseTest();"
        "} catch (err) {"
        "    if (err === \"Exception test!\") {"
        "        print(\"Caught!\");"
        "    }"
        "}"
        "Script.stop(true);";

    QString printed;
    auto sm = makeManager(script, "testRaiseCatch.js");

    connect(sm.get(), &ScriptManager::printedMessage, [&printed](const QString& message, const QString& engineName){
        printed.append(message);
    });


    sm->engine()->registerGlobalObject("testClass", new TestClass(sm->engine()));

    sm->run();
    auto ex = sm->getUncaughtException();

    QVERIFY(!ex);
    QVERIFY(printed == "Caught!");
}


void ScriptEngineTests::testSignal() {
    QString script =
        "var count = 0;"
        "Script.update.connect(function(deltaTime) {"
        "    count++;"
        "    print(deltaTime);"
        "    if (count >= 10) {"
        "        Script.stop(true);"
        "    }"
        "});";

    QStringList printed;
    auto sm = makeManager(script, "testSignal.js");

    connect(sm.get(), &ScriptManager::printedMessage, [&printed](const QString& message, const QString& engineName){
        printed.append(message);
    });

    sm->run();
    QVERIFY(printed.length() >= 10);
}

void ScriptEngineTests::testSignalWithException() {
    QString script =
        "var count = 0;"
        "Script.update.connect(function(deltaTime) {"
        "    count++;"
        "    print(deltaTime);"
        "    if (count >= 3) {"
        "        Script.stop(true);"
        "    }"
        "    nonExist();"
        "});";

    QStringList printed;
    int exceptionCount = 0;

    auto sm = makeManager(script, "testSignalWithException.js");

    connect(sm.get(), &ScriptManager::printedMessage, [&printed](const QString& message, const QString& engineName){
        printed.append(message);
    });

    connect(sm.get(), &ScriptManager::unhandledException, [&exceptionCount](std::shared_ptr<ScriptException> exception){
        exceptionCount++;
    });


    sm->run();
    QVERIFY(printed.length() >= 3);
    QVERIFY(exceptionCount >= 3);
}

void ScriptEngineTests::testQuat() {
    QString script =
        "var x;\n"
        "print(JSON.stringify(Quat.IDENTITY));\n"
        "print(JSON.stringify(Quat.safeEulerAngles(Quat.IDENTITY)));\n"
        "print(JSON.stringify(Quat.getUp(Quat.IDENTITY)));\n"
        "print(JSON.stringify(Quat.getUp(x)));\n"
        "Script.stop(true);\n";

    int printCount = 0;
    QStringList answers{
        "{\"x\":0,\"y\":0,\"z\":0,\"w\":1}",
        "{\"x\":0,\"y\":0,\"z\":0}",
        "{\"x\":0,\"y\":1,\"z\":0}",
        ""
    };

    auto sm = makeManager(script, "testQuat.js");

    connect(sm.get(), &ScriptManager::printedMessage, [&printCount, answers](const QString& message, const QString& engineName){
        QCOMPARE(message, answers[printCount++]);
    });

    connect(sm.get(), &ScriptManager::unhandledException, [](std::shared_ptr<ScriptException> exception){
        QVERIFY(exception->errorMessage.contains("undefined to glm::quat"));
    });


    sm->run();
}

