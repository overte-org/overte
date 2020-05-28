//
//  UdtServer.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtServer_h
#define hifi_udt4_UdtServer_h

#include "packet.h"
#include <QtCore/QAtomicInt>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDeadlineTimer>
#include <QtCore/QFlags>
#include <QtNetwork/QHostAddress>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

namespace udt4 {

class UdtMultiplexer;
class UdtSocket;
typedef QSharedPointer<UdtMultiplexer> UdtMultiplexerPointer;
typedef QSharedPointer<UdtSocket> UdtSocketPointer;
typedef QWeakPointer<UdtSocket> UdtSocketWeakPointer;

class UdtServer : public QObject {
    Q_OBJECT
public:
    enum AcceptFlagValues
    {
        AcceptDatagramConnections = 1,
        AcceptStreamConnections = 2,
    };
    typedef QFlags<AcceptFlagValues> AcceptFlags;

public:
    explicit UdtServer(QObject* parent = nullptr);
    virtual ~UdtServer();

    AcceptFlags acceptFlags() const;
    void close();
    QString errorString() const;
    virtual bool hasPendingConnections() const;
    bool isListening() const;
    bool listen(quint16 port, const QHostAddress& address = QHostAddress::Any);
    qint64 listenReplayWindow() const;
    int maxPendingConnections() const;
    virtual UdtSocketPointer nextPendingConnection();
    void pauseAccepting();
    void resumeAccepting();
    QHostAddress serverAddress() const;
    QAbstractSocket::SocketError serverError() const;
    quint16 serverPort() const;
    void setAcceptFlags(AcceptFlags flags);
    void setListenReplayWindow(qint64 msecs);
    void setMaxPendingConnections(int numConnections);
    bool waitForNewConnection(int msec = 0, bool* timedOut = nullptr);

public: // internal use
    bool readHandshake(UdtMultiplexer* multiplexer, const Packet& udtPacket, const QHostAddress& peerAddress, quint16 peerPort);

signals:
    void acceptError(QAbstractSocket::SocketError socketError);
    void newConnection();

protected:
    virtual bool checkValidHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, quint16 peerPort);

private slots:
    void onEpochBump();

private:
    quint32 generateSynCookie(const QHostAddress& peerAddress, quint16 peerPort);
    bool checkSynCookie(quint32 cookie, const QHostAddress& peerAddress, quint16 peerPort);
    bool rejectHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, quint16 peerPort);

private:
    struct AcceptedSockInfo {
        quint32 sockID{ 0 };
        quint32 initialSequenceNumber{ 0 };
        UdtSocketWeakPointer socket;
    };
    typedef QMap<QDeadlineTimer, AcceptedSockInfo> AcceptSockInfoMap;
    typedef QQueue<UdtSocketPointer> UdtSocketQueue;

    AcceptFlags _acceptFlags{ AcceptDatagramConnections };
    QTimer _epochBumper;
    QString _errorString;
    qint64 _listenReplayWindow{ 5 * 60 * 1000 };
    QAtomicInteger<quint32> _maxPendingConn{ 30 };
    UdtMultiplexerPointer _multiplexer;
    QAtomicInteger<quint8> _pausePendingConn{ 0 };
    QAbstractSocket::SocketError _serverError{ QAbstractSocket::SocketError::UnknownSocketError };
    quint32 _synCookieSeed{ 0 };            // SYN cookie seed, randomized on socket open and used to generate a cookie based on peer port number
    QAtomicInteger<quint32> _synEpoch{ 0 }; // 9-bit integer contributes to SYN cookie - randomized but incremented every 64 seconds

    mutable QMutex _acceptedHistoryProtect;
    AcceptSockInfoMap _acceptedHistory;
    mutable QMutex _acceptedSocketsProtect;
    UdtSocketQueue _acceptedSockets;
    QWaitCondition _socketAvailable;
};

}  // namespace udt4


#endif /* hifi_udt4_UdtServer_h */