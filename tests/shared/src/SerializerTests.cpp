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
    SerDes s;
    QCOMPARE(s.length(), 0);
    QCOMPARE(s.capacity(), SerDes::DEFAULT_SIZE);
    QCOMPARE(s.isEmpty(), true);
}

void SerializerTests::testAdd() {
    SerDes s;
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
    SerDes s;
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

    s.rewind();

    s >> i8;
    s >> i16;
    s >> i32;
    s >> v3_b;
    s >> v4_b;
    s >> iv2_b;
    s >> f_b;

    qDebug() << s;

    QCOMPARE(i8, (qint8)1);
    QCOMPARE(i16, (qint16)0xaabb);
    QCOMPARE(i32, (qint32)0xccddeeff);
    QCOMPARE(v3_a, v3_b);
    QCOMPARE(v4_a, v4_b);
    QCOMPARE(iv2_a, iv2_b);
    QCOMPARE(f_a, f_b);
}

void SerializerTests::testReadPastEnd() {
    SerDes s;
    qint8 i8;
    qint16 i16;
    s << (qint8)1;
    s.rewind();
    s >> i8;
    QCOMPARE(s.pos(), 1);

    s.rewind();
    s >> i16;
    QCOMPARE(s.pos(), 0);
}

void SerializerTests::benchmarkEncodingDynamicAlloc() {
    QBENCHMARK {
        SerDes s;
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
        SerDes s(buf, sizeof(buf));
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
    SerDes s;
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
        s.rewind();
        s >> q8;
        s >> q16;
        s >> q32;
        s >> v3_a;
        s >> v4_a;
        s >> iv2_a;
    }
}


void SerializerTests::cleanupTestCase() {
}

