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
#include <QtCore/QRandomGenerator>
#include <QtCore/QtEndian>

using namespace udt4;

UdtServer::UdtServer(QObject* parent /*= nullptr*/) : QObject(parent) {
    connect(&_epochBumper, SIGNAL(timeout), this, SLOT(onEpochBump));
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
int maxPendingConnections() const;
virtual UdtSocketPointer nextPendingConnection();
void pauseAccepting();
void resumeAccepting();
*/
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
void setMaxPendingConnections(int numConnections);
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
    if (!_config.CanAcceptDgram && hsPacket.SockType == packet.TypeDGRAM) {
        log.Printf("Refusing new socket creation from listener requesting DGRAM");
        return false;
    }
    if (!_config.CanAcceptStream && hsPacket.SockType == packet.TypeSTREAM) {
        log.Printf("Refusing new socket creation from listener requesting STREAM");
        return false;
    }
    return true;
}

bool UdtServer::rejectHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, quint16 peerPort) {
    log.Printf("%s (listener) sending handshake(reject) to %s (id=%d)", l.m.laddr.String(), from.String(), hsPacket.SockID);

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
		log.Printf("%s (listener) sending handshake(request) to %s (id=%d)", l.m.laddr.String(), from.String(), hsPacket.SockID);

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

	now := time.Now();
	_acceptHistProt.Lock();
	if(_acceptHist != nullptr) {
		replayWindow := _config.ListenReplayWindow;
		if(replayWindow <= 0){
			replayWindow = DefaultConfig().ListenReplayWindow;
		}
		_acceptHist.Prune(time.Now().Add(-replayWindow));
		s, idx := _acceptHist.Find(hsPacket.SockID, hsPacket.InitPktSeq);
		if(s != nullptr) {
			_acceptHist[idx].lastTouch = now;
			_acceptHistProt.Unlock();
			return s.readHandshake(m, hsPacket, from);
		}
	}
	_acceptHistProt.Unlock();

	if (!checkValidHandshake(hsPacket, peerAddress, peerPort)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}

	UdtSocketPointer socket = _multiplexer->newSocket(peerAddress, peerPort, true, hsPacket._sockType == SocketType::DGRAM);
	_acceptHistProt.Lock();
	if(_acceptHist == nullptr) {
		_acceptHist = []acceptSockInfo{acceptSockInfo{
			sockID:    hsPacket.SockID,
			initSeqNo: hsPacket.InitPktSeq,
			lastTouch: now,
			sock:      socket,
		}};
		heap.Init(&_acceptHist);
	} else {
		heap.Push(&_acceptHist, acceptSockInfo{
			sockID:    hsPacket.SockID,
			initSeqNo: hsPacket.InitPktSeq,
			lastTouch: now,
			sock:      socket,
		});
	}
	_acceptHistProt.Unlock();
	if(!socket->checkValidHandshake(m, hsPacket, from)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}
	if(!socket->readHandshake(m, hsPacket, from)) {
		rejectHandshake(hsPacket, peerAddress, peerPort);
		return false;
	}

	_accept <- socket;
	return true;
}
