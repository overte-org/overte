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

#ifdef WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

using namespace udt4;

UdtSocket::UdtSocket(QObject* parent) : QIODevice(parent) {
    _connTimeout.setSingleShot(true);
    _connTimeout.setTimer(Qt::PreciseTimer);
    _connRetry.setSingleShot(true);
    _connRetry.setTimer(Qt::PreciseTimer);
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
    abort();
    _isDatagram = datagramMode;
    _isServer = false;
}

void UdtSocket::connectToHost(const QHostAddress& address, quint16 port, quint16 localPort, bool datagramMode) {
    abort();
    _isDatagram = datagramMode;
    _isServer = false;
    startConnect(address, port, localPort);
}

void UdtSocket::rendezvousToHost(const QString& hostName, quint16 port, quint16 localPort, bool datagramMode) {
    abort();
    _isDatagram = datagramMode;
    _isServer = false;
}

void UdtSocket::rendezvousToHost(const QHostAddress& address, quint16 port, quint16 localPort, bool datagramMode) {
    abort();
    _isDatagram = datagramMode;
    _isServer = false;
    startRendezvous(address, port, localPort);
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
    _multiplexer = UdtMultiplexer::getInstance(localPort, localAddress, NULL, NULL);
    if (_multiplexer.isNull()) {
        // TODO: retrieve the error message associated with this? or error messages anywhere?
        return false;
    }

    _remoteAddr = address;
    _remotePort = port;

    // setup an "off-axis" UDP socket that (hopefully) will inform us of Path-MTU issues for the destination
    _offAxisUdpSocket.bind(_multiplexer->serverAddress());
    _offAxisUdpSocket.connectToHost(address, port);

    // try to avoid fragmentation (and hopefully be notified if we exceed path MTU)
#if defined(Q_OS_LINUX)
    auto sd = _udpSocket.socketDescriptor();
    if (localAddress.protocol() == QAbstractSocket::IPv4Protocol) {  // DF bit is only relevant for IPv4
        int val = IP_PMTUDISC_DONT;
        setsockopt(sd, IPPROTO_IP, IP_MTU_DISCOVER, &val, sizeof(val));
    }
    //int on = 1;
    //setsockopt(sd, SOL_IP, IP_RECVERR, &on, sizeof(on)); // Let us know of any network errors
#elif defined(Q_OS_WIN)
    if (localAddress.protocol() == QAbstractSocket::IPv4Protocol) {  // DF bit is only relevant for IPv4
        auto sd = _offAxisUdpSocket.socketDescriptor();
        int val = 0;  // false
        if (setsockopt(sd, IPPROTO_IP, IP_DONTFRAGMENT, reinterpret_cast<const char*>(&val), sizeof(val))) {
            auto wsaErr = WSAGetLastError();
            qCWarning(networking) << "Socket::bind Cannot setsockopt IP_DONTFRAGMENT" << wsaErr;
        }
    }
#endif

    // retrieve the (initial?) Path MTU from the off-axis socket
    QVariant varMtu = _offAxisUdpSocket.socketOption(QAbstractSocket::PathMtuSocketOption);
    int mtu = varMtu.toInt(NULL);
    if (mtu == 0) {
        qCWarning(networking) << "Attempt to retrieve Path MTU setting failed";
        _mtu.store(1500);  // default value, shouldn't be seeing this though
    } else {
        _mtu.store(mtu);
    }

    return true;
}

void UdtSocket::startConnect(const QHostAddress& address, quint16 port, quint16 localPort) {

    if (!preConnect(address, port, localPort)) {
        // an error occurred, do something?
        return;
    }

	connectWait := &sync.WaitGroup{};
	_connectWait = connectWait;
	connectWait.Add(1);

	_sockState = SocketState::Connecting;

	_connTimeout.start(3 * Second);
	_connRetry.start(250 * Millisecond);
	go s.goManageConnection();

	sendHandshake(0, HandshakePacket::RequestType::Request);

	connectWait.Wait();
	return _connectionError();
}

void UdtSocket::startRendezvous(const QHostAddress& address, quint16 port, quint16 localPort) {
    if (!preConnect(address, port, localPort)) {
        // an error occurred, do something?
        return;
    }

	connectWait := &sync.WaitGroup{};
	_connectWait = connectWait;
	_connectWait.Add(1);

	_sockState = SocketState::Rendezvous;

	_connTimeout.start(30 * Second);
	_connRetry.start(250 * Millisecond);
	go s.goManageConnection();

	_multiplexer->startRendezvous(s);
	sendHandshake(0, HandshakePacket::RequestType::Rendezvous);

	connectWait.Wait();
	return _connectionError();
}

void UdtSocket::sendHandshake(quint32 synCookie, HandshakePacket::RequestType requestType) {

    HandshakePacket hsResponse;
    hsResponse._udtVer = 3;
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

// called by the multiplexer read loop when a packet is received for this socket.
// Minimal processing is permitted but try not to stall the caller
void UdtSocket::readPacket(const Packet& udtPacket, const QHostAddress& peerAddress, uint peerPort) {
	now := time.Now()
	if(_sockState == SocketState::Closed) {
		return
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
public:
    explicit UdtSocket(QObject* parent = nullptr);
    virtual ~UdtSocket();

public: // from QUdpSocket
    bool hasPendingDatagrams() const;
    qint64 pendingDatagramSize() const;
    QNetworkDatagram receiveDatagram(qint64 maxSize = -1);
    qint64 readDatagram(char* data, qint64 maxlen);
    qint64 writeDatagram(const char* data, qint64 len);

public:  // from QAbstractSocket
    void abort();
    virtual void connectToHost(const QString& hostName, quint16 port, QIODevice::OpenMode openMode = ReadWrite, bool datagramMode = true);
    virtual void connectToHost(const QHostAddress& address, quint16 port, QIODevice::OpenMode openMode = ReadWrite, bool datagramMode = true);
    virtual void rendezvousToHost(const QString& hostName, quint16 port, QIODevice::OpenMode openMode = ReadWrite, bool datagramMode = true);
    virtual void rendezvousToHost(const QHostAddress& address, quint16 port, QIODevice::OpenMode openMode = ReadWrite, bool datagramMode = true);
    virtual void disconnectFromHost();
    bool flush();
    bool isValid() const;
    qint64 readBufferSize() const;
    virtual void setReadBufferSize(qint64 size);
    virtual void setSocketOption(QAbstractSocket::SocketOption option, const QVariant& value);
    virtual QVariant socketOption(QAbstractSocket::SocketOption option);
    virtual bool waitForConnected(int msecs = 30000);
    virtual bool waitForDisconnected(int msecs = 30000);

    virtual bool atEnd() const override;
    virtual qint64 bytesAvailable() const override;
    virtual qint64 bytesToWrite() const override;
    virtual bool canReadLine() const override;
    virtual void close() override;
    virtual bool isSequential() const override;
    virtual bool waitForBytesWritten(int msecs = 30000) override;
    virtual bool waitForReadyRead(int msecs = 30000) override;

signals: // from QAbstractSocket
    void connected();
    void disconnected();
    void hostFound();
    void stateChanged(SocketState socketState);

protected: // from QAbstractSocket
    virtual qint64 readData(char* data, qint64 maxSize) override;
    virtual qint64 readLineData(char* data, qint64 maxlen) override;
    virtual qint64 writeData(const char* data, qint64 size) override;

public: // internal implementation
    bool readHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort);
    bool checkValidHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort);
    static UdtSocketPointer newSocket(UdtMultiplexerPointer multiplexer, quint32 socketID, bool isServer, bool isDatagram,
        const QHostAddress& peerAddress, uint peerPort);
    void readPacket(const Packet& udtPacket, const QHostAddress& peerAddress, uint peerPort);
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
	switch s.sockState {
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

func (s *udtSocket) isOpen() bool {
	switch s.sockState {
	case sockStateClosed, sockStateRefused, sockStateCorrupted, sockStateTimeout:
		return false
	default:
		return true
	}
}

// SetDeadline sets the read and write deadlines associated
// with the connection. It is equivalent to calling both
// SetReadDeadline and SetWriteDeadline.
//
// A deadline is an absolute time after which I/O operations
// fail with a timeout (see type Error) instead of
// blocking. The deadline applies to all future and pending
// I/O, not just the immediately following call to Read or
// Write. After a deadline has been exceeded, the connection
// can be refreshed by setting a deadline in the future.
//
// An idle timeout can be implemented by repeatedly extending
// the deadline after successful Read or Write calls.
//
// A zero value for t means I/O operations will not time out.
//
// Note that if a TCP connection has keep-alive turned on,
// which is the default unless overridden by Dialer.KeepAlive
// or ListenConfig.KeepAlive, then a keep-alive failure may
// also return a timeout error. On Unix systems a keep-alive
// failure on I/O can be detected using
// errors.Is(err, syscall.ETIMEDOUT).
// (required for net.Conn implementation)
func (s *udtSocket) SetDeadline(t time.Time) error {
	s.setDeadline(t, &s.readDeadline, &s.readDeadlinePassed)
	s.setDeadline(t, &s.writeDeadline, &s.writeDeadlinePassed)
	return nil
}

func (s *udtSocket) setDeadline(dl time.Time, timer **time.Timer, timerPassed *bool) {
	if *timer == nil {
		if !dl.IsZero() {
			*timer = time.NewTimer(dl.Sub(time.Now()))
		}
	} else {
		now := time.Now()
		if !dl.IsZero() && dl.Before(now) {
			*timerPassed = true
		}
		oldTime := *timer
		if dl.IsZero() {
			*timer = nil
		}
		oldTime.Stop()
		_, _ = <-oldTime.C
		if !dl.IsZero() && dl.After(now) {
			*timerPassed = false
			oldTime.Reset(dl.Sub(time.Now()))
		}
	}
}

// SetReadDeadline sets the deadline for future Read calls
// and any currently-blocked Read call.
// A zero value for t means Read will not time out.
// (required for net.Conn implementation)
func (s *udtSocket) SetReadDeadline(t time.Time) error {
	s.setDeadline(t, &s.readDeadline, &s.readDeadlinePassed)
	return nil
}

// SetWriteDeadline sets the deadline for future Write calls
// and any currently-blocked Write call.
// Even if write times out, it may return n > 0, indicating that
// some of the data was successfully written.
// A zero value for t means Write will not time out.
// (required for net.Conn implementation)
func (s *udtSocket) SetWriteDeadline(t time.Time) error {
	s.setDeadline(t, &s.writeDeadline, &s.writeDeadlinePassed)
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

// checkValidHandshake checks to see if we want to accept a new connection with this handshake.
func (s *udtSocket) checkValidHandshake(m *multiplexer, p *packet.HandshakePacket, from *net.UDPAddr) bool {
	if s.udtVer != 4 {
		return false
	}
	return true
}

// readHandshake is received when a handshake packet is received without a destination, either as part
// of a listening response or as a rendezvous connection
func (s *udtSocket) readHandshake(m *multiplexer, p *packet.HandshakePacket, from *net.UDPAddr) bool {
	if !from.IP.Equal(s.raddr.IP) || from.Port != s.raddr.Port {
		log.Printf("huh? initted with %s but handshake with %s", s.raddr.String(), from.String())
		return false
	}

	switch s.sockState {
	case sockStateInit: // server accepting a connection from a client
		s.initPktSeq = p.InitPktSeq
		s.udtVer = int(p.UdtVer)
		s.farSockID = p.SockID
		s.isDatagram = p.SockType == packet.TypeDGRAM

		if s.mtu.get() > p.MaxPktSize {
			s.mtu.set(p.MaxPktSize)
		}
		s.launchProcessors()
		s.recv.configureHandshake(p)
		s.send.configureHandshake(p, true)
		s.sockState = sockStateConnected
		s.connTimeout = nil
		s.connRetry = nil
		go s.goManageConnection()

		s.sendHandshake(p.SynCookie, packet.HsResponse)
		return true

	case sockStateConnecting: // client attempting to connect to server
		if p.ReqType == packet.HsRefused {
			s.sockState = sockStateRefused
			return true
		}
		if p.ReqType == packet.HsRequest {
			if !s.checkValidHandshake(m, p, from) || p.InitPktSeq != s.initPktSeq || !from.IP.Equal(s.raddr.IP) || from.Port != s.raddr.Port || s.isDatagram != (p.SockType == packet.TypeDGRAM) {
				// ignore, not a valid handshake request
				return true
			}
			// handshake isn't done yet, send it back with the cookie we received
			s.sendHandshake(p.SynCookie, packet.HsResponse)
			return true
		}
		if p.ReqType != packet.HsResponse {
			// unexpected packet type, ignore
			return true
		}
		if !s.checkValidHandshake(m, p, from) || p.InitPktSeq != s.initPktSeq || !from.IP.Equal(s.raddr.IP) || from.Port != s.raddr.Port || s.isDatagram != (p.SockType == packet.TypeDGRAM) {
			// ignore, not a valid handshake request
			return true
		}
		s.farSockID = p.SockID

		if s.mtu.get() > p.MaxPktSize {
			s.mtu.set(p.MaxPktSize)
		}
		s.launchProcessors()
		s.recv.configureHandshake(p)
		s.send.configureHandshake(p, true)
		s.connRetry = nil
		s.sockState = sockStateConnected
		s.connTimeout = nil
		if s.connectWait != nil {
			s.connectWait.Done()
			s.connectWait = nil
		}
		return true

	case sockStateRendezvous: // client attempting to rendezvous with another client
		if p.ReqType == packet.HsRefused {
			s.sockState = sockStateRefused
			return true
		}
		if p.ReqType != packet.HsRendezvous || s.farSockID == 0 {
			return true // not a request packet, ignore
		}
		if !s.checkValidHandshake(m, p, from) || !from.IP.Equal(s.raddr.IP) || from.Port != s.raddr.Port || s.isDatagram != (p.SockType == packet.TypeDGRAM) {
			// not a compatible handshake, ignore
			return true
		}
		/* not quite sure how to negotiate this, assuming split-brain for now
		if p.InitPktSeq != s.initPktSeq {
			s.sockState = sockStateCorrupted
			return true
		}
		*/
		s.farSockID = p.SockID
		s.m.endRendezvous(s)

		if s.mtu.get() > p.MaxPktSize {
			s.mtu.set(p.MaxPktSize)
		}
		s.launchProcessors()
		s.recv.configureHandshake(p)
		s.send.configureHandshake(p, false)
		s.connRetry = nil
		s.sockState = sockStateConnected
		s.connTimeout = nil
		if s.connectWait != nil {
			s.connectWait.Done()
			s.connectWait = nil
		}

		// send the final rendezvous packet
		s.sendHandshake(p.SynCookie, packet.HsResponse)
		return true

	case sockStateConnected: // server repeating a handshake to a client
		if s.isServer && p.ReqType == packet.HsRequest {
			// client didn't receive our response handshake, resend it
			s.sendHandshake(p.SynCookie, packet.HsResponse)
		} else if !s.isServer && p.ReqType == packet.HsResponse {
			// this is a rendezvous connection (re)send our response
			s.sendHandshake(p.SynCookie, packet.HsResponse2)
		}
		return true
	}

	return false
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

