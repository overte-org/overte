//
//  Created by Dale Glass 2022/10/22
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef overte_SettingsTests_h
#define overte_SettingsTests_h

#include <QtCore/QObject>




class SettingsTestsWorker : public QObject {
    Q_OBJECT

public slots:
    void saveSettings();
};

class SettingsTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void loadSettings();
    void saveSettings();

    void testSettings();
    void testGroups();
    void testArray();
    void testArrayInGroup();

    void testHandleUnused();
    void testHandle();


    void benchmarkSetValue();
    void benchmarkSaveSettings();
    void benchmarkSetValueConcurrent();

    void cleanupTestCase();

private:
    QThread *_settingsThread = nullptr;
    SettingsTestsWorker *_testWorker = nullptr;

};

#endif // overte_SettingsTests_h
