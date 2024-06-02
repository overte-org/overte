//
//  PacketTests.h
//  tests/networking/src
//
//  Created by Stephen Birarda on 07/14/15.
//  Copyright 2015 High Fidelity, Inc.
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
    void httpRequest();
    void httpsRequest();
};

#endif // overte_QtNetworkTests_h
