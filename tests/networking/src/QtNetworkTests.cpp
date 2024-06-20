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

#include "QtNetworkTests.h"
#include <test-utils/QTestExtensions.h>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QHostInfo>
#include <QTcpSocket>
#include <QSslSocket>
#include <QSslCipher>

/**
 * @brief Test basic Qt networking functionality
 *
 * This test was created to test a problem found with the Conan PR.
 * Possibly some sort of library trouble.
 *
 * Normally there's no reason why this should go wrong, so it's mostly
 * a test of that Qt is deployed and working properly.
 *
 * Turns out it's some sort of initialization issue. It can be reproduced by calling the test as:
 *    ./networking-QtNetworkTests httpsRequestNoSSLVersion
 *
 * This test may get stuck on some systems. Running the full suite, or:
 *    ./networking-QtNetworkTests httpsRequestNoSSLVersion
 *
 * will work correctly because  QSslSocket::sslLibraryVersionString() initializes something.
 */
const QUrl HTTP_URL("http://ping.archlinux.org/");
const QUrl HTTPS_URL("https://ping.archlinux.org/");
const QString TCP_HOST("ping.archlinux.org");
const QString SSL_HOST("ping.archlinux.org");



QTEST_MAIN(QtNetworkTests);

void QtNetworkTests::initTestCase() {
    qDebug() << "Init";
    qRegisterMetaType<QNetworkReply*>();

}

void QtNetworkTests::tcpSocket() {
    QTcpSocket sock;
    QSignalSpy spy(&sock, &QTcpSocket::connected);

    qDebug() << "Connecting to" << TCP_HOST << "on port 80";
    sock.connectToHost(TCP_HOST, 80);
    spy.wait();
    QVERIFY(sock.waitForConnected());
    QVERIFY(sock.localPort() > 0);

    qDebug() << "Local address is" << sock.localAddress()  << ":" << sock.localPort();
}

void QtNetworkTests::sslSocket() {
    QSslSocket sock;
    QSignalSpy spy(&sock, &QSslSocket::connected);

    QVERIFY(QSslSocket::supportsSsl());
    qDebug() << "SSL library version: " << QSslSocket::sslLibraryVersionString();

    qDebug() << "Connecting to" << SSL_HOST << "on port 443";
    sock.connectToHostEncrypted(SSL_HOST, 443);
    spy.wait();
    QVERIFY(sock.waitForEncrypted());
    QVERIFY(sock.localPort() > 0);

    QVERIFY(!sock.sslConfiguration().isNull());
    QVERIFY(sock.sslHandshakeErrors().length() == 0);
    QVERIFY(sock.sessionProtocol() != QSsl::UnknownProtocol);

    qDebug() << "Local address is" << sock.localAddress()  << ":" << sock.localPort();
    qDebug() << "SSL protocol : " << sock.sessionProtocol();
    qDebug() << "SSL cipher   : " << sock.sessionCipher().protocolString();
    qDebug() << "SSL cert     : " << sock.peerCertificate();
}


void QtNetworkTests::httpRequest() {
    auto manager = new QNetworkAccessManager();

    qDebug() << "Making request to" << HTTP_URL;
    QSignalSpy spy(manager, &QNetworkAccessManager::finished);
    QNetworkRequest req(HTTP_URL);
    manager->get(req);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QNetworkReply *reply = arguments.at(0).value<QNetworkReply*>();
    QVERIFY(!reply->error());
    QVERIFY(!reply->sslConfiguration().isNull());

    QString data = reply->readAll();
    qDebug() << "DATA: " << data;
}


// Unlike the test below this works, because the sslLibraryVersionString call pokes something
// in the Qt guts to make things initialize properly.

void QtNetworkTests::httpsRequest() {
    auto manager = new QNetworkAccessManager();

    qDebug() << "SSL library version      : " << QSslSocket::sslLibraryVersionString();
    qDebug() << "SSL library version      : " << QSslSocket::sslLibraryVersionString();
    qDebug() << "SSL library build version: " << QSslSocket::sslLibraryBuildVersionString();


    qDebug() << "Making request to" << HTTPS_URL;
    QSignalSpy spy(manager, &QNetworkAccessManager::finished);
    QNetworkRequest req(HTTPS_URL);
    manager->get(req);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QNetworkReply *reply = arguments.at(0).value<QNetworkReply*>();
    QVERIFY(!reply->error());
    QVERIFY(!reply->sslConfiguration().isNull());
    qDebug() << "Peer cert:" << reply->sslConfiguration().peerCertificate();
    QString data = reply->readAll();
    qDebug() << "DATA: " << data;
    qDebug() << "SSL library version: " << QSslSocket::sslLibraryVersionString();

}


// On some systems, this hangs forever. Something in the Qt guts fails to initialize.

void QtNetworkTests::httpsRequestNoSSLVersion() {
    auto manager = new QNetworkAccessManager();

    qDebug() << "Making request to" << HTTPS_URL;
    QSignalSpy spy(manager, &QNetworkAccessManager::finished);
    QNetworkRequest req(HTTPS_URL);
    manager->get(req);

    spy.wait();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QNetworkReply *reply = arguments.at(0).value<QNetworkReply*>();
    QVERIFY(!reply->error());
    QVERIFY(!reply->sslConfiguration().isNull());
    qDebug() << "Peer cert:" << reply->sslConfiguration().peerCertificate();
    QString data = reply->readAll();
    qDebug() << "DATA: " << data;
    qDebug() << "SSL library version: " << QSslSocket::sslLibraryVersionString();

}
