//
//  UdtSocket.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "UdtSocket.h"

#include "Multiplexer.h"
#include "../../NetworkLogging.h"
#include <QtCore/QLoggingCategory>
#include <QtCore/QRandomGenerator>

#ifdef WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

using namespace udt4;

UdtSocket::UdtSocket(QObject* parent) : QIODevice(parent) {
    _connTimeout.setSingleShot(true);
    _connTimeout.setTimerType(Qt::PreciseTimer);
    connect(&_connTimeout, &QTimer::timeout, this, &UdtSocket::onConnectionTimeout);
    _connRetry.setSingleShot(true);
    _connRetry.setTimerType(Qt::PreciseTimer);
    connect(&_connRetry, &QTimer::timeout, this, &UdtSocket::onConnectionRetry);
}

UdtSocket::~UdtSocket() {
    setState(SocketState::Init);
}

void UdtSocket::setState(SocketState newState) {
    QMutexLocker locker(&_sockStateProtect);
    if (_sockState == newState) {
        return;
    }

    // stop listening for rendezvous packets
    if (_sockState == SocketState::Rendezvous && !_multiplexer.isNull()) {
        disconnect(_multiplexer.get(), &UdtMultiplexer::readyRendezvousHandshake, this, &UdtSocket::onRendezvousHandshake);
    }

    bool isFullClosed = true;
    bool isLookup = false;
    bool isConnecting = false;
    bool isError = false;

    switch (newState) {
    case SocketState::HostLookup:
        isLookup = true;
        break;
    case SocketState::Rendezvous:
    case SocketState::Connecting:
        isFullClosed = false;
        isConnecting = true;
        break;
    case SocketState::Connected:
    case SocketState::CloseLinger:
        isFullClosed = false;
        break;
    case SocketState::LookupFailure:
    case SocketState::Error:
    case SocketState::Refused:
    case SocketState::Corrupted:
    case SocketState::Timeout:
        isError = true;
        break;
    case SocketState::Init:
    case SocketState::Closed:
        break;
    }

    if (!isError) {
        _errorString.clear();
    }
    if (!isLookup && _hostLookupID != 0) {
        QHostInfo::abortHostLookup(_hostLookupID);
        _hostLookupID = 0;
    }
    if (!isConnecting) {
        if (_connTimeout.isActive()) {
            _connTimeout.stop();
        }
        if (_connRetry.isActive()) {
            _connRetry.stop();
        }
    }
    if (isFullClosed) {
        if (_monitorThread.isRunning()) {
            _monitorThread.quit();
            moveToThread(QThread::currentThread());
        }
        if (_multiplexer != nullptr && _socketID != 0) {
            _multiplexer->closeSocket(_socketID);
        }
        _multiplexer.reset();
        _socketID = 0;
        _farSocketID = 0;
        if (_offAxisUdpSocket.isOpen()) {
            _offAxisUdpSocket.close();
        }
    }

    SocketState oldState = _sockState;
    _sockState = newState;
    _sockStateCondition.wakeAll();
    locker.unlock();

    emit stateChanged(newState);
    if (oldState == SocketState::Connected) {
        emit disconnected();
    } else if (newState == SocketState::Connected) {
        emit connected();
    }
}

bool UdtSocket::isSequential() const {
    return true;
}

QHostAddress UdtSocket::localAddress() const {
    if (_multiplexer.isNull()) {
        return QHostAddress(QHostAddress::SpecialAddress::Null);
    } else {
        return _multiplexer->serverAddress();
    }
}

quint16 UdtSocket::localPort() const {
    if (_multiplexer.isNull()) {
        return 0;
    } else {
        return _multiplexer->serverPort();
    }
}

void UdtSocket::connectToHost(const QString& hostName, quint16 port, quint16 localPort, bool datagramMode) {
    setState(SocketState::Init);
    _isDatagram = datagramMode;
    _socketRole = SocketRole::Client;
    startNameConnect(hostName, port, localPort);
}

void UdtSocket::connectToHost(const QHostAddress& address, quint16 port, quint16 localPort, bool datagramMode) {
    setState(SocketState::Init);
    _isDatagram = datagramMode;
    _socketRole = SocketRole::Client;
    startConnect(address, port, localPort);
}

void UdtSocket::rendezvousToHost(const QString& hostName, quint16 port, quint16 localPort, bool datagramMode) {
    setState(SocketState::Init);
    _isDatagram = datagramMode;
    _socketRole = SocketRole::Rendezvous;
    startNameConnect(hostName, port, localPort);
}

void UdtSocket::rendezvousToHost(const QHostAddress& address, quint16 port, quint16 localPort, bool datagramMode) {
    setState(SocketState::Init);
    _isDatagram = datagramMode;
    _socketRole = SocketRole::Rendezvous;
    startConnect(address, port, localPort);
}

void UdtSocket::startNameConnect(const QString& hostName, quint16 port, quint16 localPort) {

    // first check to see if the specified hostName is really an ip address
    QHostAddress ipResolver;
    if (ipResolver.setAddress(hostName)) {
        startConnect(ipResolver, port, localPort);
        return;
    }
    _remotePort = port;

    setState(SocketState::HostLookup);
    _hostLookupID = QHostInfo::lookupHost(hostName, [this, port, localPort](QHostInfo info) { onNameResolved(info, port, localPort); });
}

void UdtSocket::onNameResolved(QHostInfo info, quint16 port, quint16 localPort) {
    if (_sockState != SocketState::HostLookup || _hostLookupID != info.lookupId()) {
        qCInfo(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort() << "[" << _socketID
                            << "] received unexpected host resolution response event";
        return;
    }
    _hostLookupID = 0;

    if (info.error() != QHostInfo::NoError) {
        _errorString = info.errorString();
        setState(SocketState::LookupFailure);
        return;
    }

    QList<QHostAddress> addresses = info.addresses();
    if (addresses.isEmpty()) {
        _errorString = "Address did not resolve to an IP address";
        setState(SocketState::LookupFailure);
        return;
    }

    startConnect(addresses.first(), _remotePort, localPort);
    emit hostFound();
}

// setup an "off-axis" UDP socket that (hopefully) will inform us of Path-MTU issues for the destination
void UdtSocket::createOffAxisSocket() {
    _offAxisUdpSocket.bind(_multiplexer->serverAddress());
    _offAxisUdpSocket.connectToHost(_remoteAddr, _remotePort);

    // try to avoid fragmentation (and hopefully be notified if we exceed path MTU)
    QAbstractSocket::NetworkLayerProtocol protocol = _multiplexer->serverAddress().protocol();
    if (protocol == QAbstractSocket::IPv4Protocol) {  // DF bit is only relevant for IPv4
        auto sd = _offAxisUdpSocket.socketDescriptor();
#if defined(Q_OS_LINUX)
        int val = IP_PMTUDISC_DONT;
        setsockopt(sd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));
#elif defined(Q_OS_WIN)
        int val = 0;  // false
        if (setsockopt(sd, IPPROTO_IP, IP_DONTFRAGMENT, reinterpret_cast<const char*>(&val), sizeof(val))) {
            auto wsaErr = WSAGetLastError();
            qCWarning(networking) << "Socket::bind Cannot setsockopt IP_DONTFRAGMENT" << wsaErr;
        }
#endif
    }

    // retrieve the (initial?) Path MTU from the off-axis socket
    QVariant varMtu = _offAxisUdpSocket.socketOption(QAbstractSocket::PathMtuSocketOption);
    int mtu = varMtu.toInt(NULL);
    if (mtu == 0) {
        qCWarning(networking) << "Attempt to retrieve Path MTU setting failed";
        _mtu.store(1500);  // default value, shouldn't be seeing this though
    } else {
        _mtu.store(mtu);
    }
}

// Contains code common between server/client and rendezvous code to execute before beginning the connection attempt
bool UdtSocket::preConnect(const QHostAddress& address, quint16 port, quint16 localPort) {

    // discover and/or create the multiplexer responsible for packets on this local port & protocol
    QAbstractSocket::NetworkLayerProtocol protocol = address.protocol();
    QHostAddress localAddress;
    switch (protocol) {
        case QAbstractSocket::NetworkLayerProtocol::IPv6Protocol:
            localAddress = QHostAddress(QHostAddress::AnyIPv6);
            break;
        case QAbstractSocket::NetworkLayerProtocol::IPv4Protocol:
            localAddress = QHostAddress(QHostAddress::AnyIPv4);
            break;
        default:
            localAddress = QHostAddress(QHostAddress::Any);
            break;
    }
    _multiplexer = UdtMultiplexer::getInstance(localPort, localAddress, NULL, &_errorString);
    if (_multiplexer.isNull()) {
        // TODO: retrieve the error message associated with this? or error messages anywhere?
        return false;
    }

    _remoteAddr = address;
    _remotePort = port;

    QRandomGenerator random;
    _initialPacketSequence = random.generate();

    // register ourselves with the multiplexer
    _multiplexer->newSocket(sharedFromThis());
    _createTime.start();

    // setup an "off-axis" UDP socket that (hopefully) will inform us of Path-MTU issues for the destination
    createOffAxisSocket();

    // spin up the monitoring connection and attach ourselves to it
    _monitorThread.start();
    moveToThread(&_monitorThread);

    return true;
}

// called when an incoming handshake was received by a UdtServer
UdtSocketPointer UdtSocket::newServerSocket(UdtMultiplexerPointer multiplexer, const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort) {
    UdtSocketPointer socket = UdtSocketPointer::create();
    socket->_remoteAddr = peerAddress;
    socket->_remotePort = peerPort;
    socket->_farSocketID = hsPacket._farSocketID;
    socket->_socketRole = SocketRole::Server;
    socket->_isDatagram = (hsPacket._sockType == SocketType::DGRAM);

    // register ourselves with the multiplexer
    socket->_multiplexer->newSocket(socket);
    socket->_createTime.start();

    // setup an "off-axis" UDP socket that (hopefully) will inform us of Path-MTU issues for the destination
    socket->createOffAxisSocket();

    // spin up the monitoring connection and attach ourselves to it
    socket->_monitorThread.start();
    socket->moveToThread(&socket->_monitorThread);

    if (!socket->readHandshake(hsPacket, peerAddress, peerPort)) {
        return UdtSocketPointer();
    }

    return socket;
}

void UdtSocket::startConnect(const QHostAddress& address, quint16 port, quint16 localPort) {

    if (!preConnect(address, port, localPort)) {
        // an error occurred
        setState(SocketState::Error);
        return;
    }

    if (_socketRole == SocketRole::Rendezvous) {
        setState(SocketState::Rendezvous);

        _connTimeout.start(30 * Second);
        _connRetry.start(250 * Millisecond);
    } else {
        setState(SocketState::Connecting);

        _connTimeout.start(3 * Second);
        _connRetry.start(250 * Millisecond);
    }

    if (_socketRole == SocketRole::Rendezvous) {
        connect(_multiplexer.get(), &UdtMultiplexer::readyRendezvousHandshake, this, &UdtSocket::onRendezvousHandshake);
        sendHandshake(0, HandshakePacket::RequestType::Rendezvous);
    } else {
        sendHandshake(0, HandshakePacket::RequestType::Request);
    }
}

bool UdtSocket::waitForConnected(int msecs /* = 30000 */) {
    QDeadlineTimer deadline(msecs);
    QMutexLocker locker(&_sockStateProtect);
    while (!deadline.hasExpired()) {
        switch (_sockState) {
            case SocketState::HostLookup:
            case SocketState::Rendezvous:
            case SocketState::Connecting:
                break;  // continue to wait
            default:
                return true;  // either connected or failed to connect
        }

        _sockStateCondition.wait(&_sockStateProtect, deadline);
    }

    // connection timed out, shut it down
    setState(SocketState::Timeout);
    return false;
}

bool UdtSocket::waitForDisconnected(int msecs /* = 30000 */) {
    QDeadlineTimer deadline(msecs);
    QMutexLocker locker(&_sockStateProtect);
    while (!deadline.hasExpired()) {
        if (_sockState != SocketState::Connected) {
            return true;  // not connected, success?
        }
        _sockStateCondition.wait(&_sockStateProtect, deadline);
    }

    // connection timed out
    return false;
}

void UdtSocket::sendHandshake(quint32 synCookie, HandshakePacket::RequestType requestType) {

    HandshakePacket hsResponse;
    hsResponse._udtVer = UDT_VERSION;
    hsResponse._sockType = _isDatagram ? SocketType::DGRAM : SocketType::STREAM;
    hsResponse._initPktSeq = _initialPacketSequence;
    hsResponse._maxPktSize = _mtu;
    hsResponse._maxFlowWinSize = _maxFlowWinSize; // maximum flow window size
    hsResponse._reqType = requestType;
    hsResponse._farSocketID = _socketID;
    hsResponse._synCookie = synCookie;
    hsResponse._sockAddr = _multiplexer->serverAddress();

    Packet udtPacket = hsResponse.toPacket();
    quint32 ts = static_cast<quint32>(_createTime.nsecsElapsed() / 1000);
	_cong->onPktSent(udtPacket);
    qCDebug(networking) << _multiplexer->serverAddress() << ":" << _multiplexer->serverPort() << "[" << _socketID
        << "] sending handshake(" << static_cast<uint>(requestType) << ") to " << _remoteAddr << ":" << _remotePort << "[" << _farSocketID << "]";
    _multiplexer->sendPacket(_remoteAddr, _remotePort, _socketID, ts, udtPacket);
}

// slot called when the multiplexer has received a rendezvous packet and we're currently trying to connect
void UdtSocket::onRendezvousHandshake() {
    if (_multiplexer.isNull()) {
        // we have no multiplexer, not much point in going forward (and crashing)
        return;
    }
    if (state() != SocketState::Rendezvous) {
        // shouldn't have received this, are we connected?
        disconnect(_multiplexer.get(), &UdtMultiplexer::readyRendezvousHandshake, this, &UdtSocket::onRendezvousHandshake);
        return;
    }
    for (;;) {
        PacketEventPointer<HandshakePacket> packetEvent = _multiplexer->nextRendezvousHandshake(_remoteAddr, _remotePort);
        if (packetEvent.isNull()) { // is there anything here for us to look at?
            return;
        }
        readHandshake(packetEvent->packet, packetEvent->peerAddress, packetEvent->peerPort);
    }
}

// checkValidHandshake checks to see if we want to accept a new connection with this handshake.
bool UdtSocket::checkValidHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort) {
    if (hsPacket._udtVer != UDT_VERSION) {
        return false;
	}
	return true;
}

// readHandshake is received when a handshake packet is received without a destination, either as part
// of a listening response or as a rendezvous connection
bool UdtSocket::readHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort) {
    if (peerAddress != _remoteAddr || peerPort != _remotePort || _isDatagram != (hsPacket._sockType == SocketType::DGRAM)) {
        log.Printf("huh? initted with %s but handshake with %s", _remoteAddr.String(), peerAddress.String());
		return false;
	}

	switch(state()) {
	case SocketState::Init: // server accepting a connection from a client
		_initialPacketSequence = hsPacket._initPktSeq;
		//_udtVer = int(hsPacket.UdtVer);
		_farSocketID = hsPacket._socketID;
		_isDatagram = hsPacket._sockType == SocketType::DGRAM;

		if(_mtu.load() > hsPacket._maxPktSize) {
			_mtu.store(hsPacket._maxPktSize);
		}
		launchProcessors();
		_recv.configureHandshake(hsPacket);
		_send.configureHandshake(hsPacket, true);
        setState(SocketState::Connected);

		sendHandshake(hsPacket._synCookie, HandshakePacket::RequestType::Response);
		return true;

	case SocketState::Connecting:  // client attempting to connect to server
        switch(hsPacket._reqType) {
		case HandshakePacket::RequestType::Refused:
            setState(SocketState::Refused);
			return true;
		
		case HandshakePacket::RequestType::Request:
			if(!checkValidHandshake(hsPacket, peerAddress, peerPort) || hsPacket._initPktSeq != _initialPacketSequence) {
				// ignore, not a valid handshake request
				return false;
			}
			// handshake isn't done yet, send it back with the cookie we received
			sendHandshake(hsPacket._synCookie, HandshakePacket::RequestType::Response);
			return true;

        case HandshakePacket::RequestType::Response:
		    if(!checkValidHandshake(hsPacket, peerAddress, peerPort) || hsPacket._initPktSeq != _initialPacketSequence) {
			    // ignore, not a valid handshake request
			    return false;
		    }
		    _farSocketID = hsPacket._farSocketID;

		    if (_mtu.load() > hsPacket._maxPktSize) {
			    _mtu.store(hsPacket._maxPktSize);
		    }
		    launchProcessors();
		    _recv.configureHandshake(hsPacket);
		    _send.configureHandshake(hsPacket, true);
            setState(SocketState::Connected);
        }
		return true;

	case SocketState::Rendezvous:  // client attempting to rendezvous with another client
		if(hsPacket._reqType == HandshakePacket::RequestType::Refused) {
            setState(SocketState::Refused);
			return true;
		}
		if(hsPacket._reqType != HandshakePacket::RequestType::Rendezvous || _farSocketID == 0) {
			return false; // not a request packet, ignore
		}
		if(!checkValidHandshake(hsPacket, peerAddress, peerPort)) {
			// not a compatible handshake, ignore
			return false;
		}
		/* not quite sure how to negotiate this, assuming split-brain for now
		if(p.InitPktSeq != s.initPktSeq) {
			s.sockState = sockStateCorrupted;
			return false;
		}
		*/
		_farSocketID = hsPacket._farSocketID;

		if (_mtu.load() > hsPacket._maxPktSize) {
			_mtu.store(hsPacket._maxPktSize);
		}
		launchProcessors();
		_recv.configureHandshake(hsPacket);
		_send.configureHandshake(hsPacket, false);
        setState(SocketState::Connected);

		// send the final rendezvous packet
        sendHandshake(hsPacket._synCookie, HandshakePacket::RequestType::Response);
		return true;

	case SocketState::Connected:  // server repeating a handshake to a client
        switch(_socketRole) {
        case SocketRole::Server:
            if (hsPacket._reqType == HandshakePacket::RequestType::Request) {
                // client didn't receive our response handshake, resend it
                sendHandshake(hsPacket._synCookie, HandshakePacket::RequestType::Response);
            }
            break;
        case SocketRole::Rendezvous:
            if (hsPacket._reqType == HandshakePacket::RequestType::Response) {
                // this is a rendezvous connection (re)send our response
                sendHandshake(hsPacket._synCookie, HandshakePacket::RequestType::Response2);
            }
            break;
		}
		return true;
	}

	return false;
}

// called by the multiplexer read loop when a packet is received for this socket.
// Minimal processing is permitted but try not to stall the caller
void UdtSocket::readPacket(const Packet& udtPacket, const QHostAddress& peerAddress, uint peerPort) {
	now := time.Now();
	if(_sockState == SocketState::Closed) {
		return;
	}
	if(!from.IP.Equal(s.raddr.IP) || from.Port != s.raddr.Port) {
		log.Printf("Socket connected to %s received a packet from %s? Discarded", s.raddr.String(), from.String());
		return;
	}

	s.recvEvent <- recvPktEvent{pkt: p, now: now};

	switch (udtPacket._type) {
    case PacketType::Handshake: { // sent by both peers
        HandshakePacket hsPacket(udtPacket, peerAddress.protocol());
        readHandshake(hsPacket, peerAddress, peerPort);
        break;
    }
    case PacketType::Shutdown:    // sent by either peer
        emit _shutdownEvent(sockStateClosed, true);
        break;
    case PacketType::Ack:  // receiver -> sender
    case PacketType::Nak:
		emit _sendEvent(udtPacket, now);
        break;
    case PacketType::UserDefPkt:
		emit _cong.onCustomMsg(udtPacket);
        break;
	}
}
/*
public: // from QUdpSocket
    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
    QNetworkDatagram receiveDatagram(qint64 maxSize = -1);
    qint64 readDatagram(char* data, qint64 maxlen);
    qint64 writeDatagram(const char* data, qint64 len);

public:  // from QAbstractSocket
    virtual void disconnectFromHost();
    bool flush();
    bool isValid() const;
    qint64 readBufferSize() const;
    virtual void setReadBufferSize(qint64 size);
    virtual void setSocketOption(QAbstractSocket::SocketOption option, const QVariant& value);
    virtual QVariant socketOption(QAbstractSocket::SocketOption option);

    virtual bool atEnd() const override;
    virtual qint64 bytesAvailable() const override;
    virtual qint64 bytesToWrite() const override;
    virtual bool canReadLine() const override;
    virtual void close() override;
    virtual bool waitForBytesWritten(int msecs = 30000) override;
    virtual bool waitForReadyRead(int msecs = 30000) override;

protected: // from QAbstractSocket
    virtual qint64 readData(char* data, qint64 maxSize) override;
    virtual qint64 readLineData(char* data, qint64 maxlen) override;
    virtual qint64 writeData(const char* data, qint64 size) override;

private slots:
    void onConnectionRetry();
    void onConnectionTimeout();
*/
typedef struct ReceivePacketEvent {
    Packet packet;
    time.Time now;
};

typedef struct SendMessage {
    ByteSlice content;
    time.Time time;     // time message is submitted
    time.Duration ttl;  // message dropped if it can't be sent in this timeframe
};

typedef struct ShutdownMessage {
    SocketState sockState;
    bool permitLinger;
    QString error;
};

/*******************************************************************************
 Implementation of net.Conn interface
*******************************************************************************/

// Grab the next data packet
func (s *udtSocket) fetchReadPacket(blocking bool) ([]byte, error) {
	var result []byte
	if blocking {
		for {
			if s.readDeadlinePassed {
				return nil, syscall.ETIMEDOUT
			}
			var deadline <-chan time.Time
			if s.readDeadline != nil {
				deadline = s.readDeadline.C
			}
			select {
			case result = <-s.messageIn:
				return result, nil
			case _, ok := <-deadline:
				if !ok {
					continue
				}
				s.readDeadlinePassed = true
				return nil, syscall.ETIMEDOUT
			}
		}
	}

	select {
	case result = <-s.messageIn:
		// ok we have a message
	default:
		// ok we've read some stuff and there's nothing immediately available
		return nil, nil
	}
	return result, nil
}

func (s *udtSocket) connectionError() error {
	switch _sockState {
	case sockStateRefused:
		return errors.New("Connection refused by remote host")
	case sockStateCorrupted:
		return errors.New("Connection closed due to protocol error")
	case sockStateClosed:
		return errors.New("Connection closed")
	case sockStateTimeout:
		return errors.New("Connection timed out")
	}
	return nil
}

// TODO: int sendmsg(const char* data, int len, int msttl, bool inorder)

// Read reads data from the connection.
// Read can be made to time out and return an Error with Timeout() == true
// after a fixed time limit; see SetDeadline and SetReadDeadline.
// (required for net.Conn implementation)
func (s *udtSocket) Read(p []byte) (n int, err error) {
	connErr := s.connectionError()
	if s.isDatagram {
		// for datagram sockets, block until we have a message to return and then return it
		// if the buffer isn't big enough, return a truncated message (discarding the rest) and return an error
		msg, rerr := s.fetchReadPacket(connErr == nil)
		if rerr != nil {
			err = rerr
			return
		}
		if msg == nil && connErr != nil {
			err = connErr
			return
		}
		n = copy(p, msg)
		if n < len(msg) {
			err = errors.New("Message truncated")
		}
	} else {
		// for streaming sockets, block until we have at least something to return, then
		// fill up the passed buffer as far as we can without blocking again
		idx := 0
		l := len(p)
		n = 0
		for idx < l {
			if s.currPartialRead == nil {
				// Grab the next data packet
				currPartialRead, rerr := s.fetchReadPacket(n == 0 && connErr == nil)
				s.currPartialRead = currPartialRead
				if rerr != nil {
					err = rerr
					return
				}
				if s.currPartialRead == nil {
					if n != 0 {
						return
					}
					if connErr != nil {
						err = connErr
						return
					}
				}
			}
			thisN := copy(p[idx:], s.currPartialRead)
			n = n + thisN
			idx = idx + thisN
			if n >= len(s.currPartialRead) {
				// we've exhausted the current data packet, reset to nil
				s.currPartialRead = nil
			} else {
				s.currPartialRead = s.currPartialRead[n:]
			}
		}
	}
	return
}

// Write writes data to the connection.
// Write can be made to time out and return an Error with Timeout() == true
// after a fixed time limit; see SetDeadline and SetWriteDeadline.
// (required for net.Conn implementation)
func (s *udtSocket) Write(p []byte) (n int, err error) {
	// at the moment whatever we have right now we'll shove it into a channel and return
	// on the other side:
	//  for datagram sockets: this is a distinct message to be broken into as few packets as possible
	//  for streaming sockets: collect as much as can fit into a packet and send them out
	switch s.sockState {
	case sockStateRefused:
		err = errors.New("Connection refused by remote host")
		return
	case sockStateCorrupted:
		err = errors.New("Connection closed due to protocol error")
		return
	case sockStateClosed:
		err = errors.New("Connection closed")
		return
	}

	n = len(p)

	for {
		if s.writeDeadlinePassed {
			err = syscall.ETIMEDOUT
			return
		}
		var deadline <-chan time.Time
		if s.writeDeadline != nil {
			deadline = s.writeDeadline.C
		}
		select {
		case s.messageOut <- sendMessage{content: p, tim: time.Now()}:
			// send successful
			return
		case _, ok := <-deadline:
			if !ok {
				continue
			}
			s.writeDeadlinePassed = true
			err = syscall.ETIMEDOUT
			return
		}
	}
}

// Close closes the connection.
// Any blocked Read or Write operations will be unblocked.
// Write operations will be permitted to send (initial packets)
// Read operations will return an error
// (required for net.Conn implementation)
func (s *udtSocket) Close() error {
	if !s.isOpen() {
		return nil // already closed
	}

	close(s.messageOut)
	_, _ = <-s.shutdownEvent
	return nil
}

/*******************************************************************************
 Private functions
*******************************************************************************/

// newSocket creates a new UDT socket, which will be configured afterwards as either an incoming our outgoing socket
func newSocket(m *multiplexer, config *Config, sockID uint32, isServer bool, isDatagram bool, raddr *net.UDPAddr) (s *udtSocket) {
	now := time.Now()

	mtu := m.mtu
	if config.MaxPacketSize > 0 && config.MaxPacketSize < mtu {
		mtu = config.MaxPacketSize
	}

	maxFlowWinSize := config.MaxFlowWinSize
	if maxFlowWinSize == 0 {
		maxFlowWinSize = DefaultConfig().MaxFlowWinSize
	}
	if maxFlowWinSize < 32 {
		maxFlowWinSize = 32
	}

	s = &udtSocket{
		m:              m,
		Config:         config,
		raddr:          raddr,
		created:        now,
		sockState:      sockStateInit,
		udtVer:         4,
		isServer:       isServer,
		mtu:            atomicUint32{val: uint32(mtu)},
		maxFlowWinSize: maxFlowWinSize,
		isDatagram:     isDatagram,
		sockID:         sockID,
		initPktSeq:     packet.PacketID{Seq: randUint32()},
		messageIn:      make(chan []byte, 256),
		messageOut:     make(chan sendMessage, 256),
		recvEvent:      make(chan recvPktEvent, 256),
		sendEvent:      make(chan recvPktEvent, 256),
		sockClosed:     make(chan struct{}, 1),
		sockShutdown:   make(chan struct{}, 1),
		deliveryRate:   16,
		bandwidth:      1,
		sendPacket:     make(chan packet.Packet, 256),
		shutdownEvent:  make(chan shutdownMessage, 5),
	}
	s.cong = newUdtSocketCc(s)

	return
}

func (s *udtSocket) launchProcessors() {
	s.send = newUdtSocketSend(s)
	s.recv = newUdtSocketRecv(s)
	s.cong.init(s.initPktSeq)
}

func (s *udtSocket) goManageConnection() {
	sockClosed := s.sockClosed
	sockShutdown := s.sockShutdown
	for {
		select {
		case <-s.lingerTimer: // linger timer expired, shut everything down
			s.m.closeSocket(s.sockID)
			close(s.sockClosed)
			return
		case _, _ = <-sockShutdown:
			// catching this to force re-evaluation of this select (catching the linger timer)
		case _, _ = <-sockClosed:
			return
		case p := <-s.sendPacket:
			ts := uint32(time.Now().Sub(s.created) / time.Microsecond)
			s.cong.onPktSent(p)
			log.Printf("%s (id=%d) sending %s to %s (id=%d)", s.m.laddr.String(), s.sockID, packet.PacketTypeName(p.PacketType()),
				s.raddr.String(), s.farSockID)
			s.m.sendPacket(s.raddr, s.farSockID, ts, p)
		case sd := <-s.shutdownEvent: // connection shut down
			s.shutdown(sd.sockState, sd.permitLinger, sd.err)
		case <-s.connTimeout: // connection timed out
			s.shutdown(sockStateTimeout, true, nil)
		case <-s.connRetry: // resend connection attempt
			s.connRetry = nil
			switch s.sockState {
			case sockStateConnecting:
				s.sendHandshake(0, packet.HsRequest)
				s.connRetry = time.After(250 * time.Millisecond)
			case sockStateRendezvous:
				s.sendHandshake(0, packet.HsRendezvous)
				s.connRetry = time.After(250 * time.Millisecond)
			}
		}
	}
}

func (s *udtSocket) shutdown(sockState sockState, permitLinger bool, err error) {
	if !s.isOpen() {
		return // already closed
	}
	if err != nil {
		log.Printf("socket shutdown (type=%d), due to error: %s", int(sockState), err.Error())
	} else {
		log.Printf("socket shutdown (type=%d)", int(sockState))
	}
	if s.sockState == sockStateRendezvous {
		s.m.endRendezvous(s)
	}
	if s.connectWait != nil {
		s.connectWait.Done()
		s.connectWait = nil
	}
	s.sockState = sockState
	s.cong.close()

	if permitLinger {
		linger := s.Config.LingerTime
		if linger == 0 {
			linger = DefaultConfig().LingerTime
		}
		s.lingerTimer = time.After(linger)
	}

	s.connTimeout = nil
	s.connRetry = nil
	if permitLinger {
		close(s.sockShutdown)
	} else {
		s.m.closeSocket(s.sockID)
		close(s.sockClosed)
	}
	s.messageIn <- nil
}

func absdiff(a uint, b uint) uint {
	if a < b {
		return b - a
	}
	return a - b
}

func (s *udtSocket) applyRTT(rtt uint) {
	s.rttProt.Lock()
	s.rttVar = (s.rttVar*3 + absdiff(s.rtt, rtt)) >> 2
	s.rtt = (s.rtt*7 + rtt) >> 3
	s.rttProt.Unlock()
}

func (s *udtSocket) getRTT() (rtt, rttVar uint) {
	s.rttProt.RLock()
	rtt = s.rtt
	rttVar = s.rttVar
	s.rttProt.RUnlock()
	return
}

// Update Estimated Bandwidth and packet delivery rate
func (s *udtSocket) applyReceiveRates(deliveryRate uint, bandwidth uint) {
	s.receiveRateProt.Lock()
	if deliveryRate > 0 {
		s.deliveryRate = (s.deliveryRate*7 + deliveryRate) >> 3
	}
	if bandwidth > 0 {
		s.bandwidth = (s.bandwidth*7 + bandwidth) >> 3
	}
	s.receiveRateProt.Unlock()
}

func (s *udtSocket) getRcvSpeeds() (deliveryRate uint, bandwidth uint) {
	s.receiveRateProt.RLock()
	deliveryRate = s.deliveryRate
	bandwidth = s.bandwidth
	s.receiveRateProt.RUnlock()
	return
}

