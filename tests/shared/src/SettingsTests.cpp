//
//  Created by Dale Glass on 2022/10/22
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include "SettingsTests.h"

#include <QtTest/QtTest>
#include <SettingHandle.h>
#include <SettingManager.h>
#include <DependencyManager.h>
#include <QDebug>
#include <QCoreApplication>
#include "SettingInterface.h"
#include "SettingHandle.h"



QTEST_MAIN(SettingsTests)

void SettingsTestsWorker::saveSettings() {
    auto sm = DependencyManager::get<Setting::Manager>();
    QThread *thread = QThread::currentThread();

    while(! thread->isInterruptionRequested() ) {
        //qDebug() << "Thread is saving config";
        sm->forceSave();

        // Not having any wait here for some reason locks up the benchmark.
        // Logging a message also does the trick.
        //
        // This looks like a bug somewhere and needs investigating.
        thread->yieldCurrentThread();
    }

    thread->exit(0);
}

void SettingsTests::initTestCase() {
    QCoreApplication::setOrganizationName("OverteTest");

    DependencyManager::set<Setting::Manager>();

    Setting::init();
}

void SettingsTests::cleanupTestCase() {
  //  Setting::cleanupSettingsSaveThread();
}

void SettingsTests::loadSettings() {
    Settings s;
    qDebug() << "Loaded" << s.fileName();
}

void SettingsTests::saveSettings() {
    Settings s;
    s.setValue("TestValue", "Hello");

    auto sm = DependencyManager::get<Setting::Manager>();
    sm->setValue("TestValueSM", "Hello");

    // There seems to be a bug here, data gets lost without this call here.
    sm->forceSave();
    qDebug() << "Wrote" << s.fileName();
}

void SettingsTests::testSettings() {
    auto sm = DependencyManager::get<Setting::Manager>();
    Settings s;

    s.setValue("settingsTest", 1);
    QVERIFY(sm->value("settingsTest") == 1);
}

void SettingsTests::testGroups() {
    auto sm = DependencyManager::get<Setting::Manager>();
    Settings s;

    s.setValue("valueNotInGroupBefore", 1);
    s.beginGroup("testGroup");
    s.setValue("valueInGroup", 2);
    s.endGroup();

    s.beginGroup("testGroupFirst");
    s.beginGroup("testGroupSecond");
    s.setValue("valueInGroup", 44);
    s.endGroup();
    s.endGroup();

    s.setValue("valueNotInGroupAfter", 3);

    QVERIFY(sm->value("valueNotInGroupBefore") == 1);
    QVERIFY(sm->value("testGroup/valueInGroup") == 2);
    QVERIFY(sm->value("testGroupFirst/testGroupSecond/valueInGroup") == 44);
    QVERIFY(sm->value("valueNotInGroupAfter") == 3);
}

void SettingsTests::testArray() {
    auto sm = DependencyManager::get<Setting::Manager>();
    Settings s;

    s.beginWriteArray("testArray", 2);
    s.setValue("A", 1);
    s.setValue("B", 2);
    s.setArrayIndex(1);
    s.setValue("A", 11);
    s.setValue("B", 22);
    s.endArray();

    s.setValue("valueNotInArray", 6);

    QVERIFY(sm->value("testArray/size") == 2);
    QVERIFY(sm->value("testArray/1/A") == 1);
    QVERIFY(sm->value("testArray/1/B") == 2);
    QVERIFY(sm->value("testArray/2/A") == 11);
    QVERIFY(sm->value("testArray/2/B") == 22);
    QVERIFY(sm->value("valueNotInArray") == 6);
}

void SettingsTests::testArrayInGroup() {
    auto sm = DependencyManager::get<Setting::Manager>();
    Settings s;

    s.beginGroup("groupWithArray");
    s.beginWriteArray("arrayInGroup", 2);
    s.setValue("X", 10);
    s.setArrayIndex(1);
    s.setValue("X", 20);
    s.endArray();
    s.endGroup();

    s.setValue("valueNotInArrayOrGroup", 8);

    QVERIFY(sm->value("groupWithArray/arrayInGroup/size") == 2);
    QVERIFY(sm->value("groupWithArray/arrayInGroup/1/X") == 10);
    QVERIFY(sm->value("groupWithArray/arrayInGroup/2/X") == 20);
    QVERIFY(sm->value("valueNotInArrayOrGroup") == 8);
}

void SettingsTests::benchmarkSetValue() {
    auto sm = DependencyManager::get<Setting::Manager>();
    int i = 0;

    QBENCHMARK {
        sm->setValue("BenchmarkSetValue", ++i);
    }

}


void SettingsTests::benchmarkSaveSettings() {
    auto sm = DependencyManager::get<Setting::Manager>();
    int i = 0;

    QBENCHMARK {
        sm->setValue("BenchmarkSave", ++i);
        sm->forceSave();
    }

}


void SettingsTests::benchmarkSetValueConcurrent() {
    auto sm = DependencyManager::get<Setting::Manager>();
    int i = 0;

    _settingsThread = new QThread(qApp);
    _testWorker = new SettingsTestsWorker();

    _settingsThread->setObjectName("Save thread");
    _testWorker->moveToThread(_settingsThread);

    QObject::connect(_settingsThread, &QThread::started, _testWorker, &SettingsTestsWorker::saveSettings, Qt::QueuedConnection );

    _settingsThread->start();
    QBENCHMARK {
        sm->setValue("BenchmarkSetValueConcurrent", ++i);
    }

    _settingsThread->requestInterruption();
    _settingsThread->wait();

    delete _testWorker;
    delete _settingsThread;
}

