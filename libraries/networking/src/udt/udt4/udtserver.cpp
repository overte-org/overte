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

#include "udtserver.h"

#include "multiplexer.h"
#include "../../NetworkLogging.h"
#include <QtCore/QDeadlineTimer>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMutexLocker>
#include <QtCore/QRandomGenerator>
#include <QtCore/QtEndian>

using namespace udt4;

UdtServer::UdtServer(QObject* parent /*= nullptr*/) : QObject(parent) {
    connect(&_epochBumper, SIGNAL(timeout), this, SLOT(onEpochBump));
}

UdtServer::~UdtServer() {
    close();
}

UdtServer::AcceptFlags UdtServer::acceptFlags() const {
    return _acceptFlags;
}

void UdtServer::close() {
    if (!_multiplexer.isNull()) {
        UdtMultiplexerPointer multiplexer = _multiplexer;
        _multiplexer.clear();
        _epochBumper.stop();
        multiplexer->stopListenUdt(this);
        _socketAvailable.wakeAll();
    }
}

QString UdtServer::errorString() const {
    return _errorString;
}

bool UdtServer::hasPendingConnections() const {
    QMutexLocker locker(&_acceptedSocketsProtect);
    return !_acceptedSockets.empty();
}

bool UdtServer::isListening() const {
    return !_multiplexer.isNull();
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

    if (!_multiplexer->startListenUdt(this)) {
        _serverError = QAbstractSocket::SocketError::AddressInUseError;
        _errorString = "Port in use";
        return false;
    }

    _epochBumper.start(64 * 1000);

    return true;
}

qint64 UdtServer::listenReplayWindow() const {
    return _listenReplayWindow;
}

int UdtServer::maxPendingConnections() const {
    return _maxPendingConn.load();
}

UdtSocketPointer UdtServer::nextPendingConnection() {
    QMutexLocker locker(&_acceptedSocketsProtect);
    if (_acceptedSockets.empty()) {
        return UdtSocketPointer();
    }
    return _acceptedSockets.dequeue();
}

void UdtServer::pauseAccepting() {
    _pausePendingConn.store(1);
}

void UdtServer::resumeAccepting() {
    _pausePendingConn.store(0);
}

QHostAddress UdtServer::serverAddress() const {
    if (_multiplexer.isNull()) {
        return QHostAddress(QHostAddress::SpecialAddress::Null);
    } else {
        return _multiplexer->serverAddress();
    }
}

QAbstractSocket::SocketError UdtServer::serverError() const {
    return _serverError;
}

quint16 UdtServer::serverPort() const {
    if (_multiplexer.isNull()) {
        return 0;
    } else {
        return _multiplexer->serverPort();
    }
}

void UdtServer::setAcceptFlags(AcceptFlags flags) {
    _acceptFlags = flags;
}

void UdtServer::setListenReplayWindow(qint64 msecs) {
    _listenReplayWindow = msecs;
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

bool UdtServer::rejectHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, quint16 peerPort) {
    qCInfo(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                       << " (listener) sending handshake(reject) to " << peerAddress << ":" << peerPort
                       << " (id=" << hsPacket._farSocketID << ")";

    HandshakePacket hsResponse;
    hsResponse._udtVer = hsPacket._udtVer;
    hsResponse._sockType = hsPacket._sockType;
    hsResponse._reqType = HandshakeRequestType::Refused;
    hsResponse._sockAddr = peerAddress;

    _multiplexer->sendPacket(peerAddress, peerPort, hsPacket._farSocketID, 0, hsResponse.toPacket());
}

bool UdtServer::readHandshake(UdtMultiplexer* multiplexer, const Packet& udtPacket, const QHostAddress& peerAddress, quint16 peerPort) {

    HandshakePacket hsPacket(udtPacket, peerAddress.protocol());
    if (hsPacket._reqType == HandshakeRequestType::Request) {
        quint32 newCookie = generateSynCookie(peerAddress, peerPort);
        qCDebug(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                            << " (listener) sending handshake(request) to " << peerAddress << ":" << peerPort
                            << " (id=" << hsPacket._farSocketID << ")";

        HandshakePacket hsResponse;
        hsResponse._udtVer = hsPacket._udtVer;
        hsResponse._sockType = hsPacket._sockType;
        hsResponse._initPktSeq = hsPacket._initPktSeq;
        hsResponse._reqType = HandshakeRequestType::Request;
        hsResponse._synCookie = newCookie;
        hsResponse._sockAddr = peerAddress;

        _multiplexer->sendPacket(peerAddress, peerPort, hsPacket._farSocketID, 0, hsResponse.toPacket());
		return true;
	}

	bool isSYN = checkSynCookie(hsPacket._synCookie, peerAddress, peerPort);
	if(!isSYN) {
		return false; // ignore packets with failed SYN checks
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

            if (socket.isNull()) {
                // this appears from a previously-connected socket that is no longer connected, discard the handshake
                return true;
            } else {
                // this appears from a previously-connected socket that is still connected, forward the handshake to the connection
                return socket->readHandshake(_multiplexer, hsPacket, peerAddress, peerPort);
            }
        }
    }

	if (!checkValidHandshake(hsPacket, peerAddress, peerPort)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}

	socket = _multiplexer->newSocket(peerAddress, peerPort, true, hsPacket._sockType == SocketType::DGRAM);

    {
        QMutexLocker locker(&_acceptedHistoryProtect);
        AcceptedSockInfo& newInfo = _acceptedHistory.insert(listenReplayTimer, acceptedSockInfo).value();
        newInfo.sockID = hsPacket._farSocketID;
        newInfo.initialSequenceNumber = hsPacket._initPktSeq;
        newInfo.socket = socket;
    }

	if (!socket->checkValidHandshake(_multiplexer, hsPacket, peerAddress, peerPort)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}
	if (!socket->readHandshake(_multiplexer, hsPacket, peerAddress, peerPort)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}

    {
        QMutexLocker locker(&_acceptedSocketsProtect);
        _acceptedSockets.enqueue(socket);
        _socketAvailable.wakeAll();
    }
    emit newConnection();

	return true;
}
