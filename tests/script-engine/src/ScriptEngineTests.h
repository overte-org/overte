//
//  AudioTests.h
//  tests/audio/src
//
//  Created by Dale Glass on 27/8/2022
//  Copyright 2022 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef overte_ScriptingEngineTests_h
#define overte_ScriptingEngineTests_h

#include <QtTest/QtTest>



class ScriptEngineTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void scriptTest();
};

#endif // overte_ScriptingEngineTests_h
