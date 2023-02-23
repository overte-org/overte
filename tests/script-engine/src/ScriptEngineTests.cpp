#include <QSignalSpy>
#include <QDebug>

#include "ScriptEngineTests.h"
#include "DependencyManager.h"

#include "ScriptEngines.h"
#include "ScriptEngine.h"
#include "ScriptCache.h"
#include "ResourceManager.h"
#include "ResourceRequestObserver.h"
#include "StatTracker.h"

#include "NodeList.h"
#include "../../../libraries/entities/src/EntityScriptingInterface.h"
//#include "../../../libraries/entities/src/EntityScriptingInterface.h"

QTEST_MAIN(ScriptEngineTests)


// script factory generates scriptmanager -- singleton
//
// default scripts -- all in one thread, but chat spawns a separate thread
// // https://apidocs.overte.org/Script.html#.executeOnScriptThread
//
// scriptmanager
// every thread has a manager, and its own engine
// provides non-qt interface?
//
// special threads for entity scripts -- 12 (fixed? dynamic?)




void ScriptEngineTests::initTestCase() {
    // AudioClient starts networking, but for the purposes of the tests here we don't care,
    // so just got to use some port.
    int listenPort = 10000;

    DependencyManager::registerInheritance<LimitedNodeList, NodeList>();
    DependencyManager::set<NodeList>(NodeType::Agent, listenPort);
    DependencyManager::set<ScriptEngines>(ScriptManager::CLIENT_SCRIPT, QUrl(""));
    DependencyManager::set<ScriptCache>();
    DependencyManager::set<ResourceManager>();
    DependencyManager::set<ResourceRequestObserver>();
    DependencyManager::set<StatTracker>();
    DependencyManager::set<ScriptInitializers>();
    DependencyManager::set<EntityScriptingInterface>(true);

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

    // TODO: can we execute test scripts in serial way rather than parallel
    /*QDir testScriptsDir("tests");
    QStringList testScripts = testScriptsDir.entryList(QStringList() << "*.js", QDir::Files);
    testScripts.sort();

    for(QString script : testScripts) {
        script = "tests/" + script;
        qInfo() << "Running test script: " << script;
        ac->loadOneScript(script);
    }*/
    //ac->loadOneScript("tests/003_vector_math.js");
    ac->loadOneScript("tests/005_include.js");

    qDebug() << ac->getRunning();

    // TODO: if I don't have infinite loop here, it exits before scripts finish. It also reports: QSignalSpy: No such signal: 'scriptCountChanged'
    for (int n = 0; n > -1; n++) {
        QSignalSpy spy(ac.get(), SIGNAL(scriptCountChanged));
        spy.wait(100000);
        qDebug() << "Signal happened";
    }
    //spy.wait(5000);
    //ac->shutdownScripting();



    //TODO: Add a test for Script.require(JSON)
}
