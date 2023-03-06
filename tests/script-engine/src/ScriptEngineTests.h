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
#include "ScriptEngine.h"

using ScriptManagerPointer = std::shared_ptr<ScriptManager>;


class TestClass : public QObject {
    Q_OBJECT

    public:
        TestClass() {};

        TestClass(ScriptEnginePointer ptr) : _engine(ptr) {};

        Q_INVOKABLE int invokableFunc(int val) {
            qDebug() << "invokableFunc called with value" << val;
            return val + 10;
        }

        Q_INVOKABLE void doRaiseTest() {
            qDebug() << "About to raise an exception";
            _engine->raiseException("Exception test!");
        }


        int nonInvokableFunc(int val) {
            qCritical() << "nonInvokableFunc called with value" << val;
            return val + 20;
        }

    private:
        ScriptEnginePointer _engine;

};


class ScriptEngineTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void scriptTest();
    void testTrivial();
    void testSyntaxError();
    void testRuntimeError();
    void testJSThrow();
    void testRegisterClass();
    void testInvokeNonInvokable();
    void testRaiseException();
    void testRaiseExceptionAndCatch();
    void testSignal();
    void testSignalWithException();



private:
    ScriptManagerPointer makeManager(const QString &source, const QString &filename);

};

#endif // overte_ScriptingEngineTests_h
