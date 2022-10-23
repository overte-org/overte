//
//  Created by Bradley Austin Davis on 2017/11/08
//  Copyright 2013-2017 High Fidelity, Inc.
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

