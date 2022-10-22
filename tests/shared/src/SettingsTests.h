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

class SettingsTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void loadSettings();
    void saveSettings();
    void benchmarkSaveSettings();

    void cleanupTestCase();
};

#endif // overte_SettingsTests_h
