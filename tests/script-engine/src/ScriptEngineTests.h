//
//  SciptEngineTests.h
//  tests/script-engine/src
//
//  Created by Dale Glass
//  Copyright 2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#ifndef overte_ScriptingEngineTests_h
#define overte_ScriptingEngineTests_h

#include <QtTest/QtTest>
#include "ScriptManager.h"

using ScriptManagerPointer = std::shared_ptr<ScriptManager>;

class ScriptEngineTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void scriptTest();
    void testTrivial();
    void testSyntaxError();
    void testRuntimeError();
    void testJSThrow();


private:
    ScriptManagerPointer makeManager(const QString &source, const QString &filename);

};

#endif // overte_ScriptingEngineTests_h
