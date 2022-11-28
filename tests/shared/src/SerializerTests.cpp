//
//  SerializerTests.cpp
//
//  Copyright 2022 Dale Glass
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#include "SerializerTests.h"
#include <SerDes.h>
#include <QDebug>
#include <glm/glm.hpp>

QTEST_GUILESS_MAIN(SerializerTests)


void SerializerTests::initTestCase() {
}

void SerializerTests::testCreate() {
    DataSerializer s;
    QCOMPARE(s.length(), 0);
    QCOMPARE(s.capacity(), DataSerializer::DEFAULT_SIZE);
    QCOMPARE(s.isEmpty(), true);


    DataDeserializer d(s);
    QCOMPARE(d.length(), 0);
}

void SerializerTests::testAdd() {
    DataSerializer s;
    s << (qint8)1;
    QCOMPARE(s.length(), 1);
    QCOMPARE(s.isEmpty(), false);

    s << (quint8)-1;
    QCOMPARE(s.length(), 2);

    s << (qint16)0xaabb;
    QCOMPARE(s.length(), 4);

    s << (quint16)-18000;
    QCOMPARE(s.length(), 6);

    s << (qint32)0xCCDDEEFF;
    QCOMPARE(s.length(), 10);

    s << (quint32)-1818000000;
    QCOMPARE(s.length(), 14);

    s << "Hello, world!";
    QCOMPARE(s.length(), 28);

    glm::vec3 v{1.f,2.f,3.f};
    s << v;
    QCOMPARE(s.length(), 40);

    s << 1.2345f;
    QCOMPARE(s.length(), 44);


    qDebug() << s;
}

void SerializerTests::testAddAndRead() {
    DataSerializer s;
    glm::vec3 v3_a{1.f, 3.1415f, 2.71828f};
    glm::vec3 v3_b;
    glm::vec4 v4_a{3.1415f, 2.71828f, 1.4142f, 1.6180f};
    glm::vec4 v4_b;
    glm::ivec2 iv2_a{10, 24};
    glm::ivec2 iv2_b;
    float f_a = 1.2345f;
    float f_b;

    s << (qint8)1;
    s << (qint16)0xaabb;
    s << (qint32)0xccddeeff;
    s << v3_a;
    s << v4_a;
    s << iv2_a;
    s << f_a;

    qint8 i8;
    qint16 i16;
    qint32 i32;

    DataDeserializer d(s);

    d >> i8;
    d >> i16;
    d >> i32;
    d >> v3_b;
    d >> v4_b;
    d >> iv2_b;
    d >> f_b;

    qDebug() << d;

    QCOMPARE(i8, (qint8)1);
    QCOMPARE(i16, (qint16)0xaabb);
    QCOMPARE(i32, (qint32)0xccddeeff);
    QCOMPARE(v3_a, v3_b);
    QCOMPARE(v4_a, v4_b);
    QCOMPARE(iv2_a, iv2_b);
    QCOMPARE(f_a, f_b);
}

void SerializerTests::testReadPastEnd() {
    DataSerializer s;
    qint8 i8;
    qint16 i16;
    s << (qint8)1;

    DataDeserializer d(s);
    d >> i8;
    QCOMPARE(d.pos(), 1);

    d.rewind();
    d >> i16;
    QCOMPARE(d.pos(), 0);
}

void SerializerTests::testWritePastEnd() {
    qint8 i8 = 255;
    qint16 i16 = 65535;


    char buf[16];


    // 1 byte buffer, we can write 1 byte
    memset(buf, 0, sizeof(buf));
    DataSerializer s1(buf, 1);
    s1 << i8;
    QCOMPARE(s1.pos(), 1);
    QCOMPARE(s1.isOverflow(), false);
    QCOMPARE(buf[0], i8);

    // 1 byte buffer, we can't write 2 bytes
    memset(buf, 0, sizeof(buf));
    DataSerializer s2(buf, 1);
    s2 << i16;
    QCOMPARE(s2.pos(), 0);
    QCOMPARE(s2.isOverflow(), true);
    QCOMPARE(buf[0], 0); // We didn't write
    QCOMPARE(buf[1], 0);
}




void SerializerTests::benchmarkEncodingDynamicAlloc() {
    QBENCHMARK {
        DataSerializer s;
        glm::vec3 v3_a{1.f, 3.1415f, 2.71828f};
        glm::vec3 v3_b;
        glm::vec4 v4_a{3.1415f, 2.71828f, 1.4142f, 1.6180f};
        glm::vec4 v4_b;
        glm::ivec2 iv2_a{10, 24};
        glm::ivec2 iv2_b;

        s << (qint8)1;
        s << (qint16)0xaabb;
        s << (qint32)0xccddeeff;
        s << v3_a;
        s << v4_a;
        s << iv2_a;
    }
}

void SerializerTests::benchmarkEncodingStaticAlloc() {
    char buf[1024];

    QBENCHMARK {
        DataSerializer s(buf, sizeof(buf));
        glm::vec3 v3_a{1.f, 3.1415f, 2.71828f};
        glm::vec3 v3_b;
        glm::vec4 v4_a{3.1415f, 2.71828f, 1.4142f, 1.6180f};
        glm::vec4 v4_b;
        glm::ivec2 iv2_a{10, 24};
        glm::ivec2 iv2_b;

        s << (qint8)1;
        s << (qint16)0xaabb;
        s << (qint32)0xccddeeff;
        s << v3_a;
        s << v4_a;
        s << iv2_a;
    }
}


void SerializerTests::benchmarkDecoding() {
    DataSerializer s;
    qint8 q8 = 1;
    qint16 q16 = 0xaabb;
    qint32 q32 = 0xccddeeff;

    glm::vec3 v3_a{1.f, 3.1415f, 2.71828f};
    glm::vec3 v3_b;
    glm::vec4 v4_a{3.1415f, 2.71828f, 1.4142f, 1.6180f};
    glm::vec4 v4_b;
    glm::ivec2 iv2_a{10, 24};
    glm::ivec2 iv2_b;

    s << q8;
    s << q16;
    s << q32;
    s << v3_a;
    s << v4_a;
    s << iv2_a;


    QBENCHMARK {
        DataDeserializer d(s);
        d >> q8;
        d >> q16;
        d >> q32;
        d >> v3_a;
        d >> v4_a;
        d >> iv2_a;
    }
}


void SerializerTests::cleanupTestCase() {
}

