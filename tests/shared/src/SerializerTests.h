//
//  ResourceTests.h
//
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef overte_SerializerTests_h
#define overte_SerializerTests_h

#include <QtTest/QtTest>
#include <QtCore/QTemporaryDir>

class SerializerTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void testCreate();
    void testAdd();
    void testAddAndRead();
    void testReadPastEnd();
    void testWritePastEnd();
    void benchmarkEncodingDynamicAlloc();
    void benchmarkEncodingStaticAlloc();
    void benchmarkDecoding();
    void cleanupTestCase();
private:

};

#endif // overte_SerializerTests_h
