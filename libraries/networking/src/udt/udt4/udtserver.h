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
#include <QtCore/QCryptographicHash>
#include <QtNetwork/QHostAddress>
#include <QtCore/QSharedPointer>
#include <QtCore/QTimer>

namespace udt4 {

class UdtMultiplexer;
class UdtSocket;
typedef QSharedPointer<UdtMultiplexer> UdtMultiplexerPointer;
typedef QSharedPointer<UdtSocket> UdtSocketPointer;

class UdtServer : public QObject {
    Q_OBJECT
public:
    explicit UdtServer(QObject* parent = nullptr);

    void close();
    QString errorString() const;
    virtual bool hasPendingConnections() const;
    bool isListening() const;
    bool listen(quint16 port, const QHostAddress& address = QHostAddress::Any);
    int maxPendingConnections() const;
    virtual UdtSocketPointer nextPendingConnection();
    void pauseAccepting();
    void resumeAccepting();
    QHostAddress serverAddress() const;
    QAbstractSocket::SocketError serverError() const;
    quint16 serverPort() const;
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
    UdtMultiplexerPointer _multiplexer;
    quint32 _synEpoch{ 0 };
    quint32 _synCookie{ 0 };
    QString _errorString;
    QAbstractSocket::SocketError _serverError{ QAbstractSocket::SocketError::UnknownSocketError };
    QTimer _epochBumper;

    /*
    	accept         chan *udtSocket
	closed         chan struct{}
	acceptHist     acceptSockHeap
	acceptHistProt sync.Mutex
	config         *Config
*/
};

}  // namespace udt4


#endif /* hifi_udt4_UdtServer_h */