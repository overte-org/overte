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

#include <QtNetwork/QHostAddress>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QSharedPointer>
#include <QtCore/QThread>
#include <QtNetwork/QUdpSocket>

namespace udt4 {

class UdtSocket;

class UDTMultiplexer : public QObject {
    Q_OBJECT
public:
    typedef QSharedPointer<UDTMultiplexer> TSharedPointer;
    typedef QWeakPointer<UDTMultiplexer> TWeakPointer;

public:
    static TSharedPointer getInstance(quint16 port, const QHostAddress& localAddress = QHostAddress::Any);
    bool isLive() const;

private:
    UDTMultiplexer(quint16 port, const QHostAddress& localAddress);
    void create(quint16 port, const QHostAddress& localAddress);

    //	network       string
    //	laddr         *net.UDPAddr   // the local address handled by this multiplexer
    //	listenSock    *listener      // the server socket listening to incoming connections, if there is one
    //	servSockMutex sync.Mutex
    //	mtu           uint           // the Maximum Transmission Unit of packets sent from this address
    //	pktOut        chan packetWrapper // packets queued for immediate sending

private slots:
    void onPacketError(QUdpSocket::SocketError socketError);
    void onPacketReadReady();
    void onPacketWriteReady();

private:
    typedef QPair<quint16, QHostAddress> TLocalPortPair;
    typedef QMap<TLocalPortPair, TWeakPointer> TMultiplexerMap;
    typedef QList<UdtSocket*> TSocketList;
    typedef QMap<quint32, UdtSocket*> TSocketMap;

    QUdpSocket _udpSocket;  // the listening socket where we receive all our packets
    quint32 _nextSid;       // the SockID for the next socket created -- set to a random number on construction
    QThread _readThread;
    QThread _writeThread;

    mutable QMutex _rendezvousSocketsProtect;
    TSocketList _rendezvousSockets;
    mutable QMutex _connectedSocketsProtect;
    TSocketMap _connectedSockets;

    static QMutex gl_multiplexerMapProtect;
    static TMultiplexerMap gl_multiplexerMap;
};

} /* namespace udt4 */
#endif /* hifi_udt4_Multiplexer_h */