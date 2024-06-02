//
//  PacketTests.cpp
//  tests/networking/src
//
//  Created by Stephen Birarda on 07/14/15.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "QtNetworkTests.h"
#include <test-utils/QTestExtensions.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>



QTEST_MAIN(QtNetworkTests);

void QtNetworkTests::initTestCase() {
    qDebug() << "Init";
    qRegisterMetaType<QNetworkReply*>();

}
void QtNetworkTests::httpRequest() {
    auto manager = new QNetworkAccessManager();

    QSignalSpy spy(manager, &QNetworkAccessManager::finished);
    QNetworkRequest req(QUrl("http://google.com"));
    manager->get(req);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QNetworkReply *reply = arguments.at(0).value<QNetworkReply*>();
    QVERIFY(!reply->error());
    qDebug() << reply->readAll().length() << "Bytes received";
}

void QtNetworkTests::httpsRequest() {
    auto manager = new QNetworkAccessManager();

    QSignalSpy spy(manager, &QNetworkAccessManager::finished);
    QNetworkRequest req(QUrl("https://google.com"));
    manager->get(req);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QNetworkReply *reply = arguments.at(0).value<QNetworkReply*>();
    QVERIFY(!reply->error());
    qDebug() << reply->readAll().length() << "Bytes received";
}
