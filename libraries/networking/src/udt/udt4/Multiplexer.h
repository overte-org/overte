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
#include <QtCore/QElapsedTimer>
#include <QtCore/QEnableSharedFromThis>
#include <QtNetwork/QHostAddress>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QQueue>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtNetwork/QUdpSocket>

namespace udt4 {

class UdtMultiplexer;
class UdtSocket;
typedef QSharedPointer<UdtMultiplexer> UdtMultiplexerPointer;
typedef QWeakPointer<UdtMultiplexer> UdtMultiplexerWeakPointer;
typedef QSharedPointer<UdtSocket> UdtSocketPointer;

template<class P>
class PacketEvent {
public:
    inline PacketEvent(const P& packet, const QHostAddress& address, quint32 port);
public:
    P packet;
    QHostAddress peerAddress;
    quint32 peerPort;
    QElapsedTimer age;
};

template<class P>
using PacketEventPointer = QSharedPointer<PacketEvent<P>>;

// UdtMultiplexer
//
// Responsible for binding to a local port, receiving UDT packets, and routing them to the
// sockets and/or servers registered to the local port.  All established connections are
// identified on both the local and peer side by a unique sockID value (which may permit
// multiple independent connections to be established with the same addr+port)
//
// This class is not generally user-visible and is generated+maintained internally
// by the UDT connection classes

class UdtMultiplexer : public QObject {
    Q_OBJECT
public:
    virtual ~UdtMultiplexer();
    static UdtMultiplexerPointer getInstance(quint16 port,
                                             const QHostAddress& localAddress = QHostAddress::Any,
                                             QAbstractSocket::SocketError* serverError = nullptr,
                                             QString* errorString = nullptr);
    inline bool isLive() const;
    void sendPacket(const QHostAddress& destAddr, quint32 destPort, quint32 destSockID, quint32 timestamp, Packet packet);
    inline QHostAddress serverAddress() const;
    inline QAbstractSocket::SocketError serverError() const;
    inline quint16 serverPort() const;
    inline QString errorString() const;

    void newSocket(UdtSocketPointer socket);
    bool closeSocket(quint32 sockID);

    inline void moveToReadThread(QObject* object);
    inline void moveToWriteThread(QObject* object);

    PacketEventPointer<HandshakePacket> nextServerHandshake();
    PacketEventPointer<HandshakePacket> nextRendezvousHandshake(const QHostAddress& peerAddress, quint32 peerPort);

private:
    UdtMultiplexer();
    bool create(quint16 port, const QHostAddress& localAddress);
    static UdtMultiplexerPointer lookupInstance(quint16 port, const QHostAddress& localAddress);
    void pruneServerHandshakes();
    void pruneRendezvousHandshakes();

private slots:
    void onPacketReadReady();
    void onPacketWriteReady();

signals:
    void readyRendezvousHandshake();
    void readyServerHandshake();
signals: // private
    void readySendPacket(QPrivateSignal);

private:
    typedef QPair<quint16, QString> TLocalPortPair;
    typedef QMap<TLocalPortPair, UdtMultiplexerWeakPointer> TMultiplexerMap;
    typedef QMap<quint32, UdtSocket*> TSocketMap;
    typedef QQueue<PacketEventPointer<Packet>> TPacketQueue;
    typedef QQueue<PacketEventPointer<HandshakePacket>> THandshakeQueue;
    typedef QList<PacketEventPointer<HandshakePacket>> THandshakeList;

    enum
    {
        MAX_SERVER_HANDSHAKE_AGE = 500000000, // age in nsecs before discarding a server handshake = 500msec
        MAX_RENDEZVOUS_HANDSHAKE_AGE = 500000000,  // age in nsecs before discarding a rendezvous handshake = 500msec
    };

    QUdpSocket _udpSocket;  // the listening socket where we receive all our packets
    QAtomicInteger<quint32> _nextSid{ 0 };  // the SockID for the next socket created -- set to a random number on construction
    QThread _readThread;
    QThread _writeThread;
    quint16 _serverPort{ 0 };
    QHostAddress _serverAddress{ QHostAddress::Null };

    mutable QMutex _connectedSocketsProtect;
    TSocketMap _connectedSockets;

    mutable QMutex _serverHandshakesProtect;
    THandshakeQueue _serverHandshakes;

    mutable QMutex _rendezvousHandshakesProtect;
    THandshakeList _rendezvousHandshakes;

    mutable QMutex _sendPacketProtect;
    TPacketQueue _sendPacket;

    static QMutex gl_multiplexerMapProtect;
    static TMultiplexerMap gl_multiplexerMap;

private:
    Q_DISABLE_COPY(UdtMultiplexer)
};

} /* namespace udt4 */
#include "Multiplexer.inl"

#endif /* hifi_udt4_Multiplexer_h */