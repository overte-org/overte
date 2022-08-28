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


    ac->loadOneScript("test-missing.js");
    ac->loadOneScript("test-hello.js");
    ac->loadOneScript("test-divide-by-zero.js");
    qDebug() << ac->getRunning();


    QSignalSpy spy(ac.get(), SIGNAL(scriptCountChanged));
    spy.wait(5000);
    ac->shutdownScripting();


}
