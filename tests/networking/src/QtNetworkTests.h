//
//  PacketTests.cpp
//  tests/networking/src
//
//  Created by Dale Glass on 02/06/2024
//  Copyright 2024 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//


#ifndef overte_QtNetworkTests_h
#define overte_QtNetworkTests_h

#pragma once

#include <QtTest/QtTest>

class QtNetworkTests : public QObject {
    Q_OBJECT
private slots:
    void initTestCase();
    void tcpSocket();
    void sslSocket();
    void httpRequest();
    void httpsRequest();
};

#endif // overte_QtNetworkTests_h
