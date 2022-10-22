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

void SettingsTests::benchmarkSaveSettings() {
    auto sm = DependencyManager::get<Setting::Manager>();
    int i = 0;

    QBENCHMARK {
        sm->setValue("Benchmark", ++i);
        sm->forceSave();
    }

}
