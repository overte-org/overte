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

#include <chrono>
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

template <class P>
class PacketEvent {
public:
    inline PacketEvent(const P& p, const QHostAddress& address, quint32 port) : packet(p), peerAddress(address), peerPort(port) { age.start(); }

    template <class BaseP>
    inline explicit PacketEvent(const PacketEvent<BaseP>& src) :
        packet(src.packet), peerAddresS(src.peerAddress), peerPort(src.peerPort), age(src.age) {}

public:
    P packet;
    QHostAddress peerAddress;
    quint32 peerPort;
    QElapsedTimer age;
};

template <class P>
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
    inline bool isLive() const { return _udpSocket.isOpen(); }
    void sendPacket(const QHostAddress& destAddr,
                    quint32 destPort,
                    quint32 destSockID,
                    std::chrono::microseconds timestamp,
                    Packet packet);
    inline QHostAddress serverAddress() const { return _serverAddress; }
    inline QAbstractSocket::SocketError serverError() const { return _udpSocket.error(); }
    inline quint16 serverPort() const { return _serverPort; }
    inline QString errorString() const { return _udpSocket.errorString(); }

    void newSocket(UdtSocketPointer socket);
    bool closeSocket(quint32 sockID);

    inline void moveToReadThread(QObject* object) { object->moveToThread(&_readThread); }
    inline void moveToWriteThread(QObject* object) { object->moveToThread(&_writeThread); }

    PacketEventPointer<HandshakePacket> nextServerHandshake();
    PacketEventPointer<HandshakePacket> nextRendezvousHandshake(const QHostAddress& peerAddress, quint32 peerPort);
    PacketEventPointer<Packet> nextUndirectedPacket(const QHostAddress& peerAddress, quint32 peerPort);
    PacketEventPointer<Packet> nextUndirectedPacket();
    bool sendUndirectedPacket(const Packet& packet, const QHostAddress& peerAddress, quint32 peerPort);

private:
    UdtMultiplexer();
    bool create(quint16 port, const QHostAddress& localAddress);
    static UdtMultiplexerPointer lookupInstance(quint16 port, const QHostAddress& localAddress);
    void pruneServerHandshakes();
    void pruneRendezvousHandshakes();
    void pruneUndirectedPackets();

private slots:
    void onPacketReadReady();
    void onPacketWriteReady();

signals:
    void readyRendezvousHandshake();
    void readyServerHandshake();
    void readyUndirectedPacket();
signals:  // private
    void readySendPacket(QPrivateSignal);

private:
    typedef QPair<quint16, QString> TLocalPortPair;
    typedef QMap<TLocalPortPair, UdtMultiplexerWeakPointer> TMultiplexerMap;
    typedef QMap<quint32, UdtSocket*> TSocketMap;
    typedef QQueue<PacketEventPointer<Packet>> TPacketQueue;
    typedef QList<PacketEventPointer<Packet>> TPacketList;
    typedef QQueue<PacketEventPointer<HandshakePacket>> THandshakeQueue;
    typedef QList<PacketEventPointer<HandshakePacket>> THandshakeList;

    enum
    {
        MAX_SERVER_HANDSHAKE_AGE = 500000000,      // age in nsecs before discarding a server handshake = 500msec
        MAX_RENDEZVOUS_HANDSHAKE_AGE = 500000000,  // age in nsecs before discarding a rendezvous handshake = 500msec
        MAX_UNDIRECTED_PACKET_AGE = 1000000000,     // age in nsecs before discarding an undirected packet = 1000msec
        UDP_SEND_BUFFER_SIZE_BYTES = 1048576,
        UDP_RECEIVE_BUFFER_SIZE_BYTES = 1048576,
    };

    QUdpSocket _udpSocket;                  // the listening socket where we receive all our packets
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

    mutable QMutex _undirectedPacketProtect;
    TPacketList _undirectedPacket;

    mutable QMutex _sendPacketProtect;
    TPacketQueue _sendPacket;

    static QMutex _multiplexerMapProtect;
    static TMultiplexerMap _multiplexerMap;

private:
    Q_DISABLE_COPY(UdtMultiplexer)
};

} /* namespace udt4 */

#endif /* hifi_udt4_Multiplexer_h */