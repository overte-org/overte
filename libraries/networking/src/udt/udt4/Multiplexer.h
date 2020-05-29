//
//  Multiplexer.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#ifndef hifi_udt4_Multiplexer_h
#define hifi_udt4_Multiplexer_h

#include "Packet.h"
#include <QtCore/QAtomicInt>
#include <QtCore/QEnableSharedFromThis>
#include <QtNetwork/QHostAddress>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtNetwork/QUdpSocket>

namespace udt4 {

class UdtMultiplexer;
class UdtSocket;
typedef QSharedPointer<UdtMultiplexer> UdtMultiplexerPointer;
typedef QWeakPointer<UdtMultiplexer> UdtMultiplexerWeakPointer;
typedef QSharedPointer<UdtSocket> UdtSocketPointer;

class UdtMultiplexer : public QObject, public QEnableSharedFromThis<UdtMultiplexer> {
    Q_OBJECT
public:
    virtual ~UdtMultiplexer();
    static UdtMultiplexerPointer getInstance(quint16 port,
                                      const QHostAddress& localAddress = QHostAddress::Any,
                                      QAbstractSocket::SocketError* serverError = nullptr,
                                      QString* errorString = nullptr);
    bool isLive() const;
    void sendPacket(const QHostAddress& destAddr, quint32 destPort, quint32 destSockID, quint32 timestamp, Packet packet);
    QHostAddress serverAddress() const;
    QAbstractSocket::SocketError serverError() const;
    quint16 serverPort() const;
    QString errorString() const;

    UdtSocketPointer newSocket(const QHostAddress& peerAddress, quint16 peerPort, bool isServer, bool isDatagram);
    bool closeSocket(quint32 sockID);

    void moveToReadThread(QObject* object);
    void moveToWriteThread(QObject* object);

private:
    UdtMultiplexer();
    bool create(quint16 port, const QHostAddress& localAddress);

private slots:
    void onPacketReadReady();
    void onPacketWriteReady(Packet packet, QHostAddress destAddr, quint32 destPort);

signals:
    void rendezvousHandshake(HandshakePacket hsPacket, QHostAddress peerAddress, quint32 peerPort);
    void serverHandshake(HandshakePacket hsPacket, QHostAddress peerAddress, quint32 peerPort);

    void sendPacket(Packet packet, QHostAddress destAddr, quint32 destPort, QPrivateSignal);

private:
    typedef QPair<quint16, QString> TLocalPortPair;
    typedef QMap<TLocalPortPair, UdtMultiplexerWeakPointer> TMultiplexerMap;
    typedef QMap<quint32, UdtSocket*> TSocketMap;

    QUdpSocket _udpSocket;  // the listening socket where we receive all our packets
    QAtomicInteger<quint32> _nextSid{ 0 };  // the SockID for the next socket created -- set to a random number on construction
    QThread _readThread;
    QThread _writeThread;
    quint16 _serverPort{ 0 };
    QHostAddress _serverAddress{ QHostAddress::Null };

    mutable QMutex _connectedSocketsProtect;
    TSocketMap _connectedSockets;

    static QMutex gl_multiplexerMapProtect;
    static TMultiplexerMap gl_multiplexerMap;

private:
    Q_DISABLE_COPY(UdtMultiplexer)
};

} /* namespace udt4 */
#endif /* hifi_udt4_Multiplexer_h */