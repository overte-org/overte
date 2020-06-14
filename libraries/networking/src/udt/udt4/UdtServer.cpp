//
//  UdtServer.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "UdtServer.h"

#include "Multiplexer.h"
#include "../../NetworkLogging.h"
#include <QtCore/QDeadlineTimer>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMutexLocker>
#include <QtCore/QRandomGenerator>
#include <QtCore/QtEndian>
#include "UdtSocket.h"

using namespace udt4;

UdtServer::UdtServer(QObject* parent /*= nullptr*/) : QObject(parent) {
    connect(&_epochBumper, &QTimer::timeout, this, &UdtServer::onEpochBump);
}

UdtServer::~UdtServer() {
    close();
}

void UdtServer::close() {
    if (!_multiplexer.isNull()) {
        UdtMultiplexerPointer multiplexer = _multiplexer;
        _multiplexer.clear();
        _epochBumper.stop();
        moveToThread(QThread::currentThread());
        disconnect(&*_multiplexer, &UdtMultiplexer::readyServerHandshake, this, &UdtServer::readHandshake);
        _socketAvailable.wakeAll();
    }
}

bool UdtServer::hasPendingConnections() const {
    QMutexLocker locker(&_acceptedSocketsProtect);
    return !_acceptedSockets.empty();
}

bool UdtServer::listen(quint16 port, const QHostAddress& address /* = QHostAddress::Any */ ) {
    if (!_multiplexer.isNull()) {
        close();
    }
    _multiplexer = UdtMultiplexer::getInstance(port, address, &_serverError, &_errorString);
    if (_multiplexer.isNull()) {
        return false;
    }

    QRandomGenerator random;
    _synCookieSeed = random.generate();
    _synEpoch.store(random.generate());

    _multiplexer->moveToReadThread(this);
    connect(&*_multiplexer, &UdtMultiplexer::readyServerHandshake, this, &UdtServer::readHandshake);

    _epochBumper.start(64 * 1000);

    return true;
}

UdtSocketPointer UdtServer::nextPendingConnection() {
    QMutexLocker locker(&_acceptedSocketsProtect);
    if (_acceptedSockets.empty()) {
        return UdtSocketPointer();
    }
    return _acceptedSockets.dequeue();
}

QHostAddress UdtServer::serverAddress() const {
    if (_multiplexer.isNull()) {
        return QHostAddress(QHostAddress::SpecialAddress::Null);
    } else {
        return _multiplexer->serverAddress();
    }
}

quint16 UdtServer::serverPort() const {
    if (_multiplexer.isNull()) {
        return 0;
    } else {
        return _multiplexer->serverPort();
    }
}

void UdtServer::setMaxPendingConnections(int numConnections) {
    _maxPendingConn.store(std::max(0, numConnections));
}

bool UdtServer::waitForNewConnection(int msec /* = 0 */ , bool* timedOut /* = nullptr */ ) {

    if (isListening()) {
        QMutexLocker locker(&_acceptedSocketsProtect);

        // before we wait: is there a socket available?
        if (!_acceptedSockets.empty()) {
            return true;
        }

        // we're not waiting, let's not bother dealing with the wait condition
        if (msec != 0) {
            if (timedOut) {
                *timedOut = true;
            }
            return false;
        }

        // lets wait for a socket to become available (or until things are closed out)
        QDeadlineTimer deadline =
            msec < 0 ? QDeadlineTimer(QDeadlineTimer::Forever) : QDeadlineTimer(static_cast<qint64>(msec));
        if (!_socketAvailable.wait(&_acceptedSocketsProtect, deadline)) {
            if (timedOut) {
                *timedOut = true;
            }
            return false;
        }

        // after we wait: is there a socket available?
        if (!_acceptedSockets.empty()) {
            return true;
        }
    }

    // if we get to this point the socket was closed (either before or after we started waiting)
    if (timedOut) {
        *timedOut = false;
    }
    return false;
}

void UdtServer::onEpochBump() {
    _synEpoch.fetchAndAddRelaxed(1);
}

quint32 UdtServer::generateSynCookie(const QHostAddress& peerAddress, quint16 peerPort) {
    QCryptographicHash hasher(QCryptographicHash::Algorithm::Sha1);

    quint32 cookie_be = qToBigEndian<quint32>(_synCookieSeed);
    hasher.addData(reinterpret_cast<const char*>(&cookie_be), 4);

    quint16 port_be = qToBigEndian<quint16>(peerPort);
    hasher.addData(reinterpret_cast<const char*>(&port_be), 2);

    QByteArray hash_be = hasher.result();
    quint32 hash = qFromBigEndian<quint32>(hash_be.data());

	return ((_synEpoch.load() & 0x1f) << 11) | (hash & 0x07ff);
}

bool UdtServer::checkSynCookie(quint32 cookie, const QHostAddress& peerAddress, quint16 peerPort) {
    quint32 newCookie = generateSynCookie(peerAddress, peerPort);
	if((newCookie & 0x07ff) != (cookie & 0x07ff)) {
        return false;
	}

	quint32 theirEpoch = (cookie & 0xf100) >> 11;
    quint32 ourEpoch = _synEpoch.load();
	return (theirEpoch == (ourEpoch & 0x1f)) || (theirEpoch == ((ourEpoch - 1) & 0x1f));
}

// checkValidHandshake checks to see if we want to accept a new connection with this handshake.
bool UdtServer::checkValidHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, quint16 peerPort) {
    if (_pausePendingConn.load() != 0) {
        return false;
    }

    // we don't support any protocols other than UDT4 at this time
    if (hsPacket._udtVer != 4) {
        return false;
    }

    AcceptFlags acceptFlags = _acceptFlags;
    if (!acceptFlags.testFlag(AcceptDatagramConnections) && hsPacket._sockType == SocketType::DGRAM) {
        qCInfo(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                           << " Refusing new socket creation from listener requesting DGRAM";
        return false;
    }
    if (!acceptFlags.testFlag(AcceptStreamConnections) && hsPacket._sockType == SocketType::STREAM) {
        qCInfo(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                           << " Refusing new socket creation from listener requesting STREAM";
        return false;
    }

    {
        QMutexLocker locker(&_acceptedSocketsProtect);
        if (static_cast<quint32>(_acceptedSockets.size()) >= _maxPendingConn.load()) {
            return false;
        }
    }

    return true;
}

void UdtServer::rejectHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, quint16 peerPort) {
    qCInfo(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                       << " (listener) sending handshake(reject) to " << peerAddress << ":" << peerPort
                       << " (id=" << hsPacket._farSocketID << ")";

    HandshakePacket hsResponse;
    hsResponse._udtVer = hsPacket._udtVer;
    hsResponse._sockType = hsPacket._sockType;
    hsResponse._reqType = HandshakePacket::RequestType::Refused;
    hsResponse._sockAddr = _multiplexer->serverAddress();

    _multiplexer->sendPacket(peerAddress, peerPort, hsPacket._farSocketID, 0, hsResponse.toPacket());
}

void UdtServer::readHandshake() {
    for (;;) {

        PacketEventPointer<HandshakePacket> thisPacket = _multiplexer->nextServerHandshake();
        if (thisPacket.isNull()) {
            return;
        }
        const HandshakePacket& hsPacket = thisPacket->packet;
        const QHostAddress& peerAddress = thisPacket->peerAddress;
        quint32 peerPort = thisPacket->peerPort;

        if (hsPacket._reqType == HandshakePacket::RequestType::Request) {
            // initial packet received, generate a SYN cookie, send it back, and forget about it
            quint32 newCookie = generateSynCookie(peerAddress, peerPort);
            qCDebug(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                                << " (listener) sending handshake(request) to " << peerAddress << ":" << peerPort
                                << " (id=" << hsPacket._farSocketID << ")";

            HandshakePacket hsResponse;
            hsResponse._udtVer = hsPacket._udtVer;
            hsResponse._sockType = hsPacket._sockType;
            hsResponse._initPktSeq = hsPacket._initPktSeq;
            hsResponse._reqType = HandshakePacket::RequestType::Request;
            hsResponse._synCookie = newCookie;
            hsResponse._sockAddr = _multiplexer->serverAddress();

            _multiplexer->sendPacket(peerAddress, peerPort, hsPacket._farSocketID, 0, hsResponse.toPacket());
            return;
        }

        bool isSYN = checkSynCookie(hsPacket._synCookie, peerAddress, peerPort);
        if (!isSYN) {
            return;  // ignore packets with failed SYN checks
        }

        // have we received this handshake before? (is it just a lost/replayed packet?)
        QDeadlineTimer listenReplayTimer(_listenReplayWindow);
        AcceptedSockInfo acceptedSockInfo;

        UdtSocketPointer socket;
        {
            QMutexLocker locker(&_acceptedHistoryProtect);
            while (!_acceptedHistory.empty()) {
                AcceptSockInfoMap::iterator first = _acceptedHistory.begin();
                if (first.key().hasExpired()) {
                    _acceptedHistory.erase(first);
                } else {
                    break;
                }
            }
            AcceptSockInfoMap::iterator lookup = _acceptedHistory.begin();
            while (lookup != _acceptedHistory.end() && (lookup.value().sockID != hsPacket._farSocketID ||
                                                        lookup.value().initialSequenceNumber != hsPacket._initPktSeq)) {
                lookup++;
            }
            if (lookup != _acceptedHistory.end()) {
                acceptedSockInfo = lookup.value();
                socket = acceptedSockInfo.socket.lock();

                // we reset the discard clock whenever we see a replay from it
                _acceptedHistory.erase(lookup);
                _acceptedHistory.insert(listenReplayTimer, acceptedSockInfo);

                if (!socket.isNull()) {
                    // this appears from a previously-connected socket that is still connected, forward the handshake to the connection
                    socket->readHandshake(hsPacket, peerAddress, peerPort);
                }
                return;
            }
        }

        // This seems to be a brand new, valid handshake.  Shall we accept it?
        if (!checkValidHandshake(hsPacket, peerAddress, peerPort)) {
            rejectHandshake(hsPacket, peerAddress, peerPort);
            return;
        }

        // Seems good to us, create the socket and let them check it
        socket = UdtSocket::newServerSocket(_multiplexer, peerAddress, peerPort,
            hsPacket._farSocketID, hsPacket._sockType == SocketType::DGRAM);
        if (!socket->checkValidHandshake(hsPacket, peerAddress, peerPort)) {
            rejectHandshake(hsPacket, peerAddress, peerPort);
            return;
        }

        // They think it's good too, store this in the list of recently-accepted sockets
        {
            QMutexLocker locker(&_acceptedHistoryProtect);
            AcceptedSockInfo& newInfo = _acceptedHistory.insert(listenReplayTimer, acceptedSockInfo).value();
            newInfo.sockID = hsPacket._farSocketID;
            newInfo.initialSequenceNumber = hsPacket._initPktSeq;
            newInfo.socket = socket;
        }

        // Process the handshake (likely sending a response back to the peer)
        if (!socket->readHandshake(hsPacket, peerAddress, peerPort)) {
            rejectHandshake(hsPacket, peerAddress, peerPort);
            return;
        }

        // And let the listeners know that we have a new connection
        {
            QMutexLocker locker(&_acceptedSocketsProtect);
            _acceptedSockets.enqueue(socket);
            _socketAvailable.wakeAll();
        }
        emit newConnection();
    }
}
