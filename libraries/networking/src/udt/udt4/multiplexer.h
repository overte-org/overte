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
#include <QtNetwork/QHostAddress>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtNetwork/QUdpSocket>

namespace udt4 {

class UdtServer;
class UdtSocket;

class UdtMultiplexer : public QObject {
    Q_OBJECT
public:
    typedef QSharedPointer<UdtMultiplexer> TSharedPointer;
    typedef QWeakPointer<UdtMultiplexer> TWeakPointer;

public:
    static TSharedPointer getInstance(quint16 port, const QHostAddress& localAddress = QHostAddress::Any);
    bool isLive() const;
    void sendPacket(const QHostAddress& destAddr, quint32 destPort, quint32 destSockID, quint32 timestamp, Packet packet);
    QHostAddress serverAddress() const;
    quint16 serverPort() const;

    bool startListenUdt(UdtServer* server);
    bool stopListenUdt(UdtServer* server);

private:
    UdtMultiplexer(quint16 port, const QHostAddress& localAddress);
    void create(quint16 port, const QHostAddress& localAddress);
    void checkLive();

private slots:
    void onPacketError(QUdpSocket::SocketError socketError);
    void onPacketReadReady();
    void onPacketWriteReady(Packet packet, QHostAddress destAddr, quint32 destPort);

signals:
    void sendPacket(Packet packet, QHostAddress destAddr, quint32 destPort, QPrivateSignal);

private:
    typedef QPair<quint16, QHostAddress> TLocalPortPair;
    typedef QMap<TLocalPortPair, TWeakPointer> TMultiplexerMap;
    typedef QList<UdtSocket*> TSocketList;
    typedef QMap<quint32, UdtSocket*> TSocketMap;

    QUdpSocket _udpSocket;  // the listening socket where we receive all our packets
    quint32 _nextSid{ 0 };  // the SockID for the next socket created -- set to a random number on construction
    QThread _readThread;
    QThread _writeThread;
    quint16 _serverPort{ 0 };
    QHostAddress _serverAddress{ QHostAddress::Null };

    mutable QMutex _rendezvousSocketsProtect;
    TSocketList _rendezvousSockets;
    mutable QMutex _connectedSocketsProtect;
    TSocketMap _connectedSockets;
    mutable QMutex _serverSocketProtect;
    UdtServer* _serverSocket{ nullptr };  //	the server socket listening to incoming connections, if there is one

    static QMutex gl_multiplexerMapProtect;
    static TMultiplexerMap gl_multiplexerMap;
};

} /* namespace udt4 */
#endif /* hifi_udt4_Multiplexer_h */