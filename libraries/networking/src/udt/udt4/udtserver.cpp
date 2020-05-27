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
#include <QtCore/QLoggingCategory>
#include <QtCore/QRandomGenerator>
#include <QtCore/QtEndian>

using namespace udt4;

UdtServer::UdtServer(QObject* parent /*= nullptr*/) : QObject(parent) {
    connect(&_epochBumper, SIGNAL(timeout), this, SLOT(onEpochBump));
}

UdtServer::~UdtServer() {
    close();
}

void UdtServer::close() {
    if (!_multiplexer.isNull()) {
        UdtMultiplexerPointer multiplexer = _multiplexer;
        _multiplexer.clear();
        _epochBumper.stop();
        multiplexer->stopListenUdt(this);
    }
}

QString UdtServer::errorString() const {
    return _errorString;
}
/*
virtual bool hasPendingConnections() const;
*/
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
    _synCookie = random.generate();
    _synEpoch = random.generate();

    if (!_multiplexer->startListenUdt(this)) {
        _serverError = QAbstractSocket::SocketError::AddressInUseError;
        _errorString = "Port in use";
        return false;
    }

    _epochBumper.start(64 * 1000);

    return true;
}
/*
qint64 listenReplayWindow() const;
*/
int UdtServer::maxPendingConnections() const {
    return _maxPendingConn.load();
}
/*
virtual UdtSocketPointer nextPendingConnection();
*/
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
/*
void setListenReplayWindow(qint64 msecs);
*/
void UdtServer::setMaxPendingConnections(int numConnections) {
    _maxPendingConn.store(std::max(0, numConnections));
}
/*
bool waitForNewConnection(int msec = 0, bool* timedOut = nullptr);
*/
void UdtServer::onEpochBump() {
    _synEpoch++;
}
/*
func (l *listener) Accept() (net.Conn, error) {
	socket, ok := <-l.accept
	if ok {
		return socket, nil
	}
	return nil, errors.New("Listener closed")
}
*/
quint32 UdtServer::generateSynCookie(const QHostAddress& peerAddress, quint16 peerPort) {
    QCryptographicHash hasher(QCryptographicHash::Algorithm::Sha1);

    quint32 cookie_be = qToBigEndian<quint32>(_synCookie);
    hasher.addData(reinterpret_cast<const char*>(&cookie_be), 4);

    quint16 port_be = qToBigEndian<quint16>(peerPort);
    hasher.addData(reinterpret_cast<const char*>(&port_be), 2);

    QByteArray hash_be = hasher.result();
    quint32 hash = qFromBigEndian<quint32>(hash_be.data());

	return ((_synEpoch & 0x1f) << 11) | (hash & 0x07ff);
}

bool UdtServer::checkSynCookie(quint32 cookie, const QHostAddress& peerAddress, quint16 peerPort) {
    quint32 newCookie = generateSynCookie(peerAddress, peerPort);
	if((newCookie & 0x07ff) != (cookie & 0x07ff)) {
        return false;
	}

	quint32 epoch = (cookie & 0xf100) >> 11;
	return (epoch == (_synEpoch & 0x1f)) || (epoch == ((_synEpoch - 1) & 0x1f));
}

// checkValidHandshake checks to see if we want to accept a new connection with this handshake.
bool UdtServer::checkValidHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, quint16 peerPort) {
    if (_pausePendingConn.load() != 0) {
        return false;
    }
    if (!_config.CanAcceptDgram && hsPacket._sockType == SocketType::DGRAM) {
        qCInfo(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                           << " Refusing new socket creation from listener requesting DGRAM";
        return false;
    }
    if (!_config.CanAcceptStream && hsPacket._sockType == SocketType::STREAM) {
        qCInfo(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort()
                           << " Refusing new socket creation from listener requesting STREAM";
        return false;
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
    _acceptedHistoryProtect.lock();
    while (!_acceptedHistory.empty()) {
        AcceptSockInfoMap::iterator first = _acceptedHistory.begin();
        if (first.key().hasExpired()) {
            _acceptedHistory.erase(first);
        } else {
            break;
        }
    }
    AcceptSockInfoMap::iterator lookup = _acceptedHistory.begin();
    while (lookup != _acceptedHistory.end() && (lookup.value().sockID != hsPacket._farSocketID || lookup.value().initialSequenceNumber != hsPacket._initPktSeq)) {
        lookup++;
    }
    QDeadlineTimer listenReplayTimer(_listenReplayWindow.fetch());
    AcceptedSockInfo acceptedSockInfo;
    UdtSocketPointer socket;
    if (lookup != _acceptedHistory.end()) {
        acceptedSockInfo = lookup.value();
        socket = acceptedSockInfo.socket.lock();
        _acceptedHistory.erase(lookup);
        if (!socket.isNull()) {
            _acceptedHistory.insert(listenReplayTimer, acceptedSockInfo);
            _acceptedHistoryProtect.unlock();
            return socket->readHandshake(_multiplexer, hsPacket, peerAddress, peerPort);
        }
    }
    _acceptedHistoryProtect.unlock();

	if (!checkValidHandshake(hsPacket, peerAddress, peerPort)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}

	socket = _multiplexer->newSocket(peerAddress, peerPort, true, hsPacket._sockType == SocketType::DGRAM);

    _acceptedHistoryProtect.lock();
    AcceptedSockInfo& newInfo = _acceptedHistory.insert(listenReplayTimer, acceptedSockInfo).value();
    newInfo.sockID = hsPacket._farSocketID;
    newInfo.initialSequenceNumber = hsPacket._initPktSeq;
    newInfo.socket = socket;
    _acceptedHistoryProtect.unlock();

	if (!socket->checkValidHandshake(_multiplexer, hsPacket, peerAddress, peerPort)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}
	if (!socket->readHandshake(_multiplexer, hsPacket, peerAddress, peerPort)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}

	_accept <- socket;
	return true;
}
