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
#include <QtNetwork/QHostAddress>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>

namespace udt4 {

class UdtMultiplexer;
class UdtSocket;
typedef QSharedPointer<UdtMultiplexer> UdtMultiplexerPointer;
typedef QSharedPointer<UdtSocket> UdtSocketPointer;
typedef QWeakPointer<UdtSocket> UdtSocketWeakPointer;

class UdtServer : public QObject {
    Q_OBJECT
public:
    explicit UdtServer(QObject* parent = nullptr);
    virtual ~UdtServer();

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

    UdtMultiplexerPointer _multiplexer;
    QTimer _epochBumper;
    QString _errorString;
    QAtomicInteger<qint64> _listenReplayWindow{ 5 * 60 * 1000 };
    QAtomicInteger<quint32> _maxPendingConn{ 30 };
    QAtomicInteger<quint8> _pausePendingConn{ 0 };
    QAbstractSocket::SocketError _serverError{ QAbstractSocket::SocketError::UnknownSocketError };
    quint32 _synCookie{ 0 };
    quint32 _synEpoch{ 0 };

    mutable QMutex _acceptedHistoryProtect;
    AcceptSockInfoMap _acceptedHistory;
    /*
    accept         chan *udtSocket
	closed         chan struct{}
	config         *Config
    */
};

}  // namespace udt4


#endif /* hifi_udt4_UdtServer_h */