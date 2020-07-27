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
#include <QtCore/QMutexLocker>
#include <QtCore/QReadLocker>
#include <QtCore/QWriteLocker>
#include <QtCore/QRandomGenerator>

#ifdef WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

using namespace udt4;

UdtSocket::UdtSocket(QObject* parent) : QIODevice(parent), _send(*this), _recv(*this), _congestion(*this) {
    _connTimeout.setSingleShot(true);
    _connTimeout.setTimerType(Qt::PreciseTimer);
    connect(&_connTimeout, &QTimer::timeout, this, &UdtSocket::onConnectionTimeout);

    _connRetry.setSingleShot(true);
    _connRetry.setTimerType(Qt::PreciseTimer);
    connect(&_connRetry, &QTimer::timeout, this, &UdtSocket::onConnectionRetry);

    _lingerTimer.setSingleShot(true);
    _lingerTimer.setTimerType(Qt::PreciseTimer);
    connect(&_lingerTimer, &QTimer::timeout, this, &UdtSocket::onLingerTimeout);

    connect(this, &UdtSocket::handshakeReceived, this, &UdtSocket::handshakeReceived);
    connect(this, &UdtSocket::shutdownRequested, this, &UdtSocket::onShutdownRequested);
}

UdtSocket::~UdtSocket() {
    setState(UdtSocketState::Init);
}

bool UdtSocket::setCongestionControl(CongestionControlPointer congestionControl) {
    switch (state()) {
    case UdtSocketState::HostLookup:
    case UdtSocketState::Rendezvous:
    case UdtSocketState::Connecting:
    case UdtSocketState::HalfConnected:
    case UdtSocketState::Connected:
        return false;
    };
    _congestion.setCongestionControl(congestionControl);
    return true;
}

QString UdtSocket::addressDebugString(const QHostAddress& address, quint16 port, quint32 socketID) {
    return QString("%1:%2[%3]").arg(address.toString()).arg(port).arg(socketID);
}

QString UdtSocket::localAddressDebugString() const {
    return addressDebugString(_multiplexer->serverAddress(), _multiplexer->serverPort(), _socketID);
}

QString UdtSocket::remoteAddressDebugString() const {
    return addressDebugString(_remoteAddr, _remotePort, _farSocketID);
}

void UdtSocket::setState(UdtSocketState newState) {
    QMutexLocker locker(&_sockStateProtect);
    if (_sockState == newState) {
        return;
    }

    // stop listening for rendezvous packets
    if (_sockState == UdtSocketState::Rendezvous && !_multiplexer.isNull()) {
        disconnect(_multiplexer.get(), &UdtMultiplexer::readyRendezvousHandshake, this, &UdtSocket::onRendezvousHandshake);
    }

    bool isClosed = true;
    bool isFullClosed = true;
    bool isLookup = false;
    bool isConnecting = false;
    bool isError = false;

    switch (newState) {
    case UdtSocketState::HostLookup:
        isLookup = true;
        break;
    case UdtSocketState::Rendezvous:
    case UdtSocketState::Connecting:
    case UdtSocketState::HalfConnected:
        isClosed = false;
        isFullClosed = false;
        isConnecting = true;
        break;
    case UdtSocketState::Connected:
        isClosed = false;
        isFullClosed = false;
        break;
    case UdtSocketState::HalfClosed:
        isFullClosed = false;
        _lingerTimer.start(LINGER_TIMEOUT);
        break;
    case UdtSocketState::LookupFailure:
    case UdtSocketState::Error:
    case UdtSocketState::Refused:
    case UdtSocketState::Corrupted:
    case UdtSocketState::Timeout:
        isError = true;
        break;
    case UdtSocketState::Init:
    case UdtSocketState::Closed:
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
    if (newState != UdtSocketState::HalfClosed && _lingerTimer.isActive()) {
        _lingerTimer.stop();
    }
    if (isFullClosed) {
        if (_monitorThread.isRunning()) {
            _monitorThread.quit();
            moveToThread(QThread::currentThread());
            _send.moveToThread(QThread::currentThread());
        }
        if (_multiplexer != nullptr && _socketID != 0) {
            _multiplexer->closeSocket(_socketID);
        }
        _multiplexer.reset();
        _socketID = 0;
        _farSocketID = 0;
        _synCookie = 0;
        _receivedMessages.clear();
        _currPartialRead.clear();
        _connectionRetriesBeforeMTU = 0;
        if (_offAxisUdpSocket.isOpen()) {
            _offAxisUdpSocket.close();
        }
    }

    UdtSocketState oldState = _sockState;
    _sockState = newState;
    _sockStateCondition.wakeAll();
    locker.unlock();
    {
        // notify anything waiting for messages that the state may have changed from underneath them
        QMutexLocker guard(&_receivedMessageProtect);
        _receivedMessageCondition.wakeAll();
    }

    _send.setState(newState);

    if (isClosed) {
        switch(oldState) {
            case UdtSocketState::Rendezvous:
            case UdtSocketState::Connecting:
            case UdtSocketState::HalfConnected:
            case UdtSocketState::Connected:
                _congestion.close();
                break;
        }
    }

    emit stateChanged(newState);
    if (oldState == UdtSocketState::Connected) {
        emit disconnected();
    } else if (newState == UdtSocketState::Connected) {
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
    setState(UdtSocketState::Init);
    _isDatagram = datagramMode;
    _socketRole = SocketRole::Client;
    startNameConnect(hostName, port, localPort);
}

void UdtSocket::connectToHost(const QHostAddress& address, quint16 port, quint16 localPort, bool datagramMode) {
    setState(UdtSocketState::Init);
    _isDatagram = datagramMode;
    _socketRole = SocketRole::Client;
    startConnect(address, port, localPort);
}

void UdtSocket::rendezvousToHost(const QString& hostName, quint16 port, quint16 localPort, bool datagramMode) {
    setState(UdtSocketState::Init);
    _isDatagram = datagramMode;
    _socketRole = SocketRole::Rendezvous;
    startNameConnect(hostName, port, localPort);
}

void UdtSocket::rendezvousToHost(const QHostAddress& address, quint16 port, quint16 localPort, bool datagramMode) {
    setState(UdtSocketState::Init);
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

    setState(UdtSocketState::HostLookup);
    _hostLookupID = QHostInfo::lookupHost(hostName, [this, port, localPort](QHostInfo info) { onNameResolved(info, port, localPort); });
}

void UdtSocket::onNameResolved(QHostInfo info, quint16 port, quint16 localPort) {
    if (_sockState != UdtSocketState::HostLookup || _hostLookupID != info.lookupId()) {
        qCInfo(networking) << localAddressDebugString() << " received unexpected host resolution response event";
        return;
    }
    _hostLookupID = 0;

    if (info.error() != QHostInfo::NoError) {
        _errorString = info.errorString();
        setState(UdtSocketState::LookupFailure);
        return;
    }

    QList<QHostAddress> addresses = info.addresses();
    if (addresses.isEmpty()) {
        _errorString = "Address did not resolve to an IP address";
        setState(UdtSocketState::LookupFailure);
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
    unsigned mtu = getCurrentPathMtu();
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
    _initialPacketSequence = PacketID(random.generate());

    // register ourselves with the multiplexer
    _multiplexer->newSocket(sharedFromThis());
    _createTime.start();

    // setup an "off-axis" UDP socket that (hopefully) will inform us of Path-MTU issues for the destination
    createOffAxisSocket();

    // spin up the monitoring connection and attach ourselves to it
    _monitorThread.start();
    moveToThread(&_monitorThread);
    _send.moveToThread(&_monitorThread);

    return true;
}

// called when an incoming handshake was received by a UdtServer
UdtSocketPointer UdtSocket::newServerSocket(UdtMultiplexerPointer multiplexer, const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort) {
    UdtSocketPointer socket = UdtSocketPointer::create();
    
    if (!socket->initServerSocket(multiplexer, hsPacket, peerAddress, peerPort)) {
        return UdtSocketPointer();
    }

    return socket;
}

bool UdtSocket::initServerSocket(UdtMultiplexerPointer multiplexer, const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort) {
    _remoteAddr = peerAddress;
    _remotePort = peerPort;
    _farSocketID = hsPacket._farSocketID;
    _socketRole = SocketRole::Server;
    _isDatagram = (hsPacket._sockType == SocketType::DGRAM);

    // register ourselves with the multiplexer
    _multiplexer = multiplexer;
    _multiplexer->newSocket(sharedFromThis());
    _createTime.start();

    // setup an "off-axis" UDP socket that (hopefully) will inform us of Path-MTU issues for the destination
    createOffAxisSocket();

    // spin up the monitoring connection and attach ourselves to it
    _monitorThread.start();
    moveToThread(&_monitorThread);
    _send.moveToThread(&_monitorThread);

    return checkValidHandshake(hsPacket, peerAddress, peerPort) && processHandshake(hsPacket);
}

void UdtSocket::startConnect(const QHostAddress& address, quint16 port, quint16 localPort) {

    if (!preConnect(address, port, localPort)) {
        // an error occurred
        setState(UdtSocketState::Error);
        return;
    }

    if (_socketRole == SocketRole::Rendezvous) {
        setState(UdtSocketState::Rendezvous);

        _connTimeout.start(RENDEZVOUS_CONNECT_TIMEOUT);
    } else {
        setState(UdtSocketState::Connecting);

        _connTimeout.start(CONNECT_TIMEOUT);
    }
    _connectionRetriesBeforeMTU = 0;
    _connRetry.start(CONNECT_RETRY);

    if (_socketRole == SocketRole::Rendezvous) {
        connect(_multiplexer.get(), &UdtMultiplexer::readyRendezvousHandshake, this, &UdtSocket::onRendezvousHandshake);
        sendHandshake(HandshakePacket::RequestType::Rendezvous, false);
    } else {
        sendHandshake(HandshakePacket::RequestType::Request, false);
    }
}

// called while connecting if we haven't received a response from the peer within 250ms.  Resend initial handshake
// This is a slot called on the "object-local" thread
void UdtSocket::onConnectionRetry() {
    switch(state()) {
        case UdtSocketState::Rendezvous:
            _connRetry.start(CONNECT_RETRY);
            sendHandshake(HandshakePacket::RequestType::Rendezvous, false);
            break;
        case UdtSocketState::Connecting:
            _connRetry.start(CONNECT_RETRY);
            sendHandshake(HandshakePacket::RequestType::Request, false);
            break;
        case UdtSocketState::HalfConnected:
            unsigned mtu = getCurrentPathMtu();
            if (mtu != 0 && mtu < _mtu.load()) {
                _connectionRetriesBeforeMTU = 0;
                _mtu.store(mtu);
            } else if (++_connectionRetriesBeforeMTU > MTU_DROP_INTERVAL) {
                _connectionRetriesBeforeMTU = 0;
                unsigned newMtu = _mtu.load() - MTU_DROP_INCREMENT;
                if (newMtu < MTU_MINIMUM) {
                    newMtu = MTU_MINIMUM;
                }
                _mtu.store(newMtu);
            }
            _connRetry.start(CONNECT_RETRY);
            sendHandshake(HandshakePacket::RequestType::Response, true);
            break;
    }
}

// Called when we have failed to connect to the peer within an acceptible timeframe
// This is a slot called on the "object-local" thread
void UdtSocket::onConnectionTimeout() {
    switch(state()) {
        case UdtSocketState::Rendezvous:
        case UdtSocketState::Connecting:
        case UdtSocketState::HalfConnected:
            break;
        default:
            return;
    }

    setState(UdtSocketState::Timeout);
}

bool UdtSocket::waitForConnected(int msecs /* = 30000 */) {
    QDeadlineTimer deadline(msecs);
    QMutexLocker locker(&_sockStateProtect);
    while (!deadline.hasExpired()) {
        switch (_sockState) {
            case UdtSocketState::HostLookup:
            case UdtSocketState::Rendezvous:
            case UdtSocketState::Connecting:
            case UdtSocketState::HalfConnected:
                break;  // continue to wait
            default:
                return true;  // either connected or failed to connect
        }

        _sockStateCondition.wait(&_sockStateProtect, deadline);
    }

    // connection timed out, shut it down
    setState(UdtSocketState::Timeout);
    return false;
}

bool UdtSocket::waitForDisconnected(int msecs /* = 30000 */) {
    QDeadlineTimer deadline(msecs);
    QMutexLocker locker(&_sockStateProtect);
    while (!deadline.hasExpired()) {
        if (_sockState != UdtSocketState::Connected) {
            return true;  // not connected, success?
        }
        _sockStateCondition.wait(&_sockStateProtect, deadline);
    }

    // connection timed out
    return false;
}

unsigned UdtSocket::getCurrentPathMtu() const {
    QVariant varMtu = const_cast<QUdpSocket&>(_offAxisUdpSocket).socketOption(QAbstractSocket::PathMtuSocketOption);
    return varMtu.toUInt(NULL);
}

void UdtSocket::sendHandshake(HandshakePacket::RequestType requestType, bool mtuDiscovery) {

    unsigned sockMtu = _mtu.load();
    HandshakePacket hsResponse;
    hsResponse._udtVer = UDT_VERSION;
    hsResponse._sockType = _isDatagram ? SocketType::DGRAM : SocketType::STREAM;
    hsResponse._initPktSeq = _initialPacketSequence;
    hsResponse._maxPktSize = sockMtu;
    hsResponse._maxFlowWinSize = _maxFlowWinSize; // maximum flow window size
    hsResponse._reqType = requestType;
    hsResponse._farSocketID = _socketID;
    hsResponse._synCookie = _synCookie;
    hsResponse._sockAddr = _multiplexer->serverAddress();

    if (mtuDiscovery) {
        // during MTU discovery we send the biggest packet we believe we can send and see if it makes it to the other side alive
        // in theory an ICMP "Packet Too Big" or "Fragmentation required" packet will be sent if we hit a bump.  The operating
        // system normally wouldn't allow us to see ICMP packets but should process them and set PathMtuSocketOption of a "connected"
        // udp socket.  The multiplexer cannot "connect" to the destination, we're hoping that the off-axis udp socket will be seen
        // as the "sender" of the packets well enough to have it be a stand-in (the ICMP packet has enough information in it to
        // prove that the off-axis socket was not the source of the message though).  If we don't get ICMP packets (either because
        // of the operating system or due to firewalls blocking ICMP) then we need to gradually dial back the maximum MTU on
        // packet-resends until something gets through.

        uint headerSize = hsResponse.packetHeaderSize(_remoteAddr.protocol());
        assert(sockMtu > headerSize);
        hsResponse._extra.create(sockMtu - headerSize);
    }

    Packet udtPacket = hsResponse.toPacket();
    std::chrono::microseconds ts(_createTime.nsecsElapsed() / 1000);
    _congestion.onPacketSent(udtPacket);
    qCDebug(networking) << localAddressDebugString() << " sending handshake(" << static_cast<uint>(requestType) << ") to " << remoteAddressDebugString();
    _multiplexer->sendPacket(_remoteAddr, _remotePort, _farSocketID, ts, udtPacket);
}

// slot called when the multiplexer has received a rendezvous packet and we're currently trying to connect
// This is a slot called on the "object-local" thread
void UdtSocket::onRendezvousHandshake() {
    if (_multiplexer.isNull()) {
        // we have no multiplexer, not much point in going forward (and crashing)
        return;
    }
    if (state() != UdtSocketState::Rendezvous) {
        // shouldn't have received this, are we connected?
        disconnect(_multiplexer.get(), &UdtMultiplexer::readyRendezvousHandshake, this, &UdtSocket::onRendezvousHandshake);
        return;
    }
    for (;;) {
        PacketEventPointer<HandshakePacket> packetEvent = _multiplexer->nextRendezvousHandshake(_remoteAddr, _remotePort);
        if (packetEvent.isNull()) { // is there anything here for us to look at?
            return;
        }
        if (checkValidHandshake(packetEvent->packet, packetEvent->peerAddress, packetEvent->peerPort)) {
            processHandshake(packetEvent->packet);
        }
    }
}

// slot called when the multiplexer has received a packet addressed to us that seems to be a handshake packet
// This is a slot called on the "object-local" thread
void UdtSocket::onHandshakeReceived(HandshakePacket hsPacket, QElapsedTimer) {
    processHandshake(hsPacket);
}

// checkValidHandshake checks to see if we want to accept a new connection with this handshake.
bool UdtSocket::checkValidHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort) {
    if (peerAddress != _remoteAddr || peerPort != _remotePort) {
        qCInfo(networking) << localAddressDebugString() << ": huh? connected to " << remoteAddressDebugString()
            << " but was sent a handshake packet from " << addressDebugString(peerAddress, peerPort, hsPacket._socketID);
        return false;
    }

    if (_isDatagram != (hsPacket._sockType == SocketType::DGRAM)) {
        qCInfo(networking) << localAddressDebugString() << ": huh? connected with isDatagram=" << _isDatagram
            << " but was sent a handshake packet with isDatagram=" << (hsPacket._sockType == SocketType::DGRAM);
        return false;
    }

    if (hsPacket._udtVer != UDT_VERSION) {
        return false;
    }
    return true;
}

// readHandshake is received when a handshake packet is received from a UdtServer to an already-established connection
bool UdtSocket::readHandshake(const HandshakePacket& hsPacket, const QHostAddress& peerAddress, uint peerPort) {
    if (!checkValidHandshake(hsPacket, peerAddress, peerPort)) {
        return false;
    }

    return processHandshake(hsPacket);
}
/*  Connection protocol

    Client -> Server
        Client sends Request, sets state=Connecting ->
            -> Server[UdtServer] sees Request, creates a SYN cookie and returns Request
        Client[Connecting] sees Request with cookie, sends Response(MTU), sets state=HalfConnected ->
            -> Server[UdtServer] sees Response with valid cookie, creates UdtSocket and sends Response, sets state=Connected
        Client[HalfConnected] sees Response, sets state=Connected

    Rendezvous <-> Rendezvous
        Peer sends Rendezvous, sets state=Rendezvous ->
        Peer[Rendezvous] sees Rendezvous and sends Response(MTU), sets state=HalfConnected ->
        Peer[HalfConnected] sees Response and sends Response2, sets state=Connected ->

    Currently all "Response" packets are sent padded up to the maximum MTU, this currently seems like a good place
    to probe for maximum packet size without modifying the protocol too badly, where dropped packets should elicit
    a retry request
*/

bool UdtSocket::processHandshake(const HandshakePacket& hsPacket) {
    switch(state()) {
    case UdtSocketState::Init:  // server accepting a connection from a client
        _initialPacketSequence = hsPacket._initPktSeq;
        _farSocketID = hsPacket._socketID;
        _isDatagram = hsPacket._sockType == SocketType::DGRAM;

        if(_mtu.load() > hsPacket._maxPktSize) {
            _mtu.store(hsPacket._maxPktSize);
        }
        _congestion.init(_initialPacketSequence, _mtu.load());
        _recv.configureHandshake(hsPacket);
        _send.configureHandshake(hsPacket, _initialPacketSequence, true, _mtu.load());
        _synCookie = hsPacket._synCookie;
        setState(UdtSocketState::Connected);
        sendHandshake(HandshakePacket::RequestType::Response, false);
        return true;

    case UdtSocketState::Connecting:  // client attempting to connect to server
        switch(hsPacket._reqType) {
        case HandshakePacket::RequestType::Refused:
            setState(UdtSocketState::Refused);
            return true;
        
        case HandshakePacket::RequestType::Request:
            if(hsPacket._initPktSeq != _initialPacketSequence) {
                // ignore, not a valid handshake request
                return false;
            }
            // handshake isn't done yet, send it back with the cookie we received
            {
                unsigned mtu = getCurrentPathMtu();
                if (mtu != 0 && mtu < _mtu.load()) {
                    _mtu.store(mtu);
                }
            }
            _synCookie = hsPacket._synCookie;
            sendHandshake(HandshakePacket::RequestType::Response, true);
            _connectionRetriesBeforeMTU = 0;
            _connRetry.start(CONNECT_RETRY);
            setState(UdtSocketState::HalfConnected);
            return true;
        }

    case UdtSocketState::Rendezvous:  // client attempting to rendezvous with another client
        if(hsPacket._reqType == HandshakePacket::RequestType::Refused) {
            setState(UdtSocketState::Refused);
            return true;
        }
        if(hsPacket._reqType != HandshakePacket::RequestType::Rendezvous) {
            return false; // not a request packet, ignore
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
        {
            unsigned mtu = getCurrentPathMtu();
            if (mtu != 0 && mtu < _mtu.load()) {
                _mtu.store(mtu);
            }
        }
        _synCookie = hsPacket._synCookie;
        sendHandshake(HandshakePacket::RequestType::Response, true);
        _connectionRetriesBeforeMTU = 0;
        _connRetry.start(CONNECT_RETRY);
        setState(UdtSocketState::HalfConnected);
        return true;

    case UdtSocketState::HalfConnected:
        switch (hsPacket._reqType) {
            case HandshakePacket::RequestType::Refused:
                setState(UdtSocketState::Refused);
                return true;
            case HandshakePacket::RequestType::Rendezvous:
                if (_socketRole == SocketRole::Rendezvous) {  // replay of the rendezvous handshake?
                    assert(_farSocketID == hsPacket._farSocketID);
                    _farSocketID = hsPacket._farSocketID;

                    if (_mtu.load() > hsPacket._maxPktSize) {
                        _mtu.store(hsPacket._maxPktSize);
                    }
                    unsigned mtu = getCurrentPathMtu();
                    if (mtu != 0 && mtu < _mtu.load()) {
                        _connectionRetriesBeforeMTU = 0;
                        _mtu.store(mtu);
                    }
                    _connRetry.start(CONNECT_RETRY);
                    sendHandshake(HandshakePacket::RequestType::Response, true);
                    return true;
                }
                return false;
            case HandshakePacket::RequestType::Response:
                break;  // handled below
            default:
                return false;  // not a packet we're handling here
        }
        if (_socketRole == SocketRole::Client && hsPacket._initPktSeq != _initialPacketSequence) {
            // ignore, not a valid handshake request
            return false;
        }
        assert(_socketRole == SocketRole::Client || _farSocketID == hsPacket._farSocketID);
        _farSocketID = hsPacket._farSocketID;

        if (_mtu.load() > hsPacket._maxPktSize) {
            _mtu.store(hsPacket._maxPktSize);
        }
        _congestion.init(_initialPacketSequence, _mtu.load());
        _recv.configureHandshake(hsPacket);
        _send.configureHandshake(hsPacket, _initialPacketSequence, _socketRole == SocketRole::Client, _mtu.load());
        setState(UdtSocketState::Connected);

        if (_socketRole == SocketRole::Rendezvous) {
            // send the final rendezvous packet
            sendHandshake(HandshakePacket::RequestType::Response2, false);
        }
        return true;

    case UdtSocketState::Connected:  // server repeating a handshake to a client
        switch(_socketRole) {
        case SocketRole::Server:
            if (hsPacket._reqType == HandshakePacket::RequestType::Request) {
                // client didn't receive our response handshake, resend it
                sendHandshake(HandshakePacket::RequestType::Response, true);
            }
            break;
        case SocketRole::Rendezvous:
            if (hsPacket._reqType == HandshakePacket::RequestType::Response) {
                // peer didn't receive our response2 handshake, resend it
                sendHandshake(HandshakePacket::RequestType::Response2, false);
                break;
            }
            break;
        }
        return true;
    }

	return false;
}

// Called by the multiplexer read loop on any packets received for this socket
// This is running on the multiplexer's primary thread so keep processing to a minimum
void UdtSocket::readPacket(const Packet& udtPacket, const QHostAddress& peerAddress, uint peerPort) {
	QElapsedTimer now;

    if (peerAddress != _remoteAddr || peerPort != _remotePort) {
        qCInfo(networking) << localAddressDebugString() << ": Socket connected to " << remoteAddressDebugString()
            << " but received a packet from " << addressDebugString(peerAddress, peerPort, udtPacket._socketID) << ", discarded.";
        return;
    }

    _send.resetReceiveTimer();

    switch (udtPacket._type) {
    case PacketType::Handshake: { // sent by both peers
        HandshakePacket hsPacket(udtPacket, peerAddress.protocol());
        if (checkValidHandshake(hsPacket, peerAddress, peerPort)) {
            emit handshakeReceived(hsPacket, now, QPrivateSignal());
        }
        break;
    }
    case PacketType::Ack:  // receiver -> sender
    case PacketType::Nak:
    case PacketType::Congestion:
        emit _send.packetReceived(udtPacket, now);
        break;
    case PacketType::Data:
        _congestion.onPacketReceived(udtPacket, now);
        emit _recv.packetReceived(udtPacket, now);
        break;
    case PacketType::Shutdown:  // sent by either peer
    case PacketType::Ack2:  // sender -> receiver
    case PacketType::MsgDropReq:
    case PacketType::SpecialErr:
        emit _recv.packetReceived(udtPacket, now);
        break;
    case PacketType::UserDefPkt:
        emit customMessageReceived(udtPacket, now);
        _congestion.onCustomMessageReceived(udtPacket, now);
        break;
    }
}

void UdtSocket::onShutdownRequested(UdtSocketState toState, QString error) {
    if (toState == UdtSocketState::HalfClosed && state() != UdtSocketState::Connected) {
        toState = UdtSocketState::Closed;
    }
    _errorString = error;
    setState(toState);
}

void UdtSocket::close() {
    disconnectFromHost();
}

void UdtSocket::disconnectFromHost() {
    switch (state()) {
    case UdtSocketState::Connected:
        _send.queueDisconnect();
        requestShutdown(UdtSocketState::HalfClosed, "Shutdown requested by us");
        break;
    case UdtSocketState::Rendezvous:
    case UdtSocketState::Connecting:
    case UdtSocketState::HalfConnected:
        setState(UdtSocketState::Closed);
        break;
    }
}

void UdtSocket::onLingerTimeout() {
    if (state() == UdtSocketState::HalfClosed) {
        setState(UdtSocketState::Closed);
    }
}

qint64 UdtSocket::writeDatagram(const char* data, quint64 len, const QDeadlineTimer& timeout) {
    if (len == 0 || !_isDatagram || state() != UdtSocketState::Connected) {
        return -1;
    }
    ByteSlice slice;
    void* newData = slice.create(len);
    memcpy(newData, data, len);
    _send.sendMessage(slice, timeout);
    return len;
}

qint64 UdtSocket::writeDatagram(const QByteArray& datagram, const QDeadlineTimer& timeout) {
    if (datagram.isEmpty() || !_isDatagram || state() != UdtSocketState::Connected) {
        return -1;
    }
    ByteSlice slice;
    size_t len = datagram.size();
    void* newData = slice.create(len);
    memcpy(newData, datagram.constData(), len);
    _send.sendMessage(slice, timeout);
    return len;
}

qint64 UdtSocket::writeDatagram(const ByteSlice& datagram, const QDeadlineTimer& timeout) {
    if (datagram.empty() || !_isDatagram || state() != UdtSocketState::Connected) {
        return -1;
    }
    _send.sendMessage(datagram, timeout);
    return datagram.length();
}

qint64 UdtSocket::writeData(const char* data, qint64 size) {
    if (size <= 0 || _isDatagram || state() != UdtSocketState::Connected) {
        return -1;
    }
    ByteSlice slice;
    void* newData = slice.create(size);
    memcpy(newData, data, size);
    _send.sendMessage(slice, QDeadlineTimer(QDeadlineTimer::Forever));
    return size;
}

void UdtSocket::receivedMessage(const ByteSlice& message) {
    QMutexLocker guard(&_receivedMessageProtect);
    _receivedMessages.append(message);
    _receivedMessageCondition.wakeAll();
}

bool UdtSocket::hasPendingDatagrams() const {
    if (!_isDatagram) {
        return false;
    }
    QMutexLocker guard(&_receivedMessageProtect);
    return _receivedMessages.isEmpty();
}

qint64 UdtSocket::pendingDatagramSize() const {
    if (!_isDatagram) {
        return -1;
    }
    QMutexLocker guard(&_receivedMessageProtect);
    if (_receivedMessages.empty()) {
        return -1;
    }
    return _receivedMessages.first().length();
}

qint64 UdtSocket::bytesAvailable() const {
    if (_isDatagram) {
        return 0;
    }
    QMutexLocker guard(&_receivedMessageProtect);
    quint64 len = 0;
    for (MessageQueue::const_iterator iter = _receivedMessages.begin(); iter != _receivedMessages.end(); iter++) {
        len += iter->length();
    }
    return len;
}

qint64 UdtSocket::readDatagram(char* data, qint64 maxlen) {
    if (!_isDatagram) {
        return -1;
    }
    ByteSlice datagram = receiveDatagram();
    if (datagram.empty()) {
        return -1;
    }
    if (maxlen != 0) {
        size_t len = datagram.length();
        if (maxlen > 0 && static_cast<size_t>(maxlen) < len) {
            len = maxlen;
        }
        memcpy(data, datagram.constData(), len);
    }
    return datagram.length();
}

ByteSlice UdtSocket::receiveMessage() {
    QMutexLocker guard(&_receivedMessageProtect);
    while (_receivedMessages.empty() && state() == UdtSocketState::Connected) {
        _receivedMessageCondition.wait(&_receivedMessageProtect);
    }
    if (_receivedMessages.empty()) {
        return ByteSlice();
    }
    return _receivedMessages.dequeue();
}

ByteSlice UdtSocket::receiveDatagram() {
    if (!_isDatagram) {
        return ByteSlice();
    }
    return receiveMessage();
}

qint64 UdtSocket::readData(char* data, qint64 maxSize) {
    // the read starts out as blocking but then grabs whatever is available at the time
    if (_isDatagram) {
        return -1;
    }
    if (_currPartialRead.empty()) {
        _currPartialRead = receiveMessage();
        if (_currPartialRead.empty()) {
            return -1;
        }
    }
    qint64 totalLen = 0;
    for (;;) {
        qint64 len = _currPartialRead.length();
        if (len >= maxSize) {
            if (maxSize > 0) {
                memcpy(data, _currPartialRead.constData(), maxSize);
                totalLen += maxSize;
                if (len == maxSize) {
                    _currPartialRead.clear();
                } else {
                    _currPartialRead = _currPartialRead.substring(len);
                }
            }
            return totalLen;
        }

        memcpy(data, _currPartialRead.constData(), len);
        totalLen += len;
        data += len;
        maxSize -= len;

        // check to see if there's another message we can tack on to this
        QMutexLocker guard(&_receivedMessageProtect);
        if (_receivedMessages.empty()) {
            return totalLen;
        }
        _currPartialRead = _receivedMessages.dequeue();
    }
}

qint64 UdtSocket::readLineData(char* data, qint64 maxSize) {
    // the read starts out as blocking but then grabs whatever is available at the time
    if (_isDatagram) {
        return -1;
    }
    if (_currPartialRead.empty()) {
        _currPartialRead = receiveMessage();
        if (_currPartialRead.empty()) {
            return -1;
        }
    }
    qint64 totalLen = 0;
    for (;;) {
        qint64 len = _currPartialRead.length();
        const quint8* eolnPos = static_cast<const quint8*>(memchr(_currPartialRead.constData(), '\n', len));
        if (eolnPos != nullptr && eolnPos - _currPartialRead.constData() < maxSize) {
            qint64 lineLen = eolnPos - _currPartialRead.constData() + 1;
            memcpy(data, _currPartialRead.constData(), lineLen);
            if (lineLen < maxSize) {
                data[lineLen + 1] = '\0';
            }
            totalLen += lineLen;
            _currPartialRead = _currPartialRead.substring(lineLen);
            return totalLen;
        }
        if (len >= maxSize) {
            if (maxSize > 0) {
                memcpy(data, _currPartialRead.constData(), maxSize);
                totalLen += maxSize;
                _currPartialRead = _currPartialRead.substring(maxSize);
            }
            return totalLen;
        }

        memcpy(data, _currPartialRead.constData(), len);
        totalLen += len;
        data += len;
        maxSize -= len;

        // check to see if there's another message we can tack on to this
        QMutexLocker guard(&_receivedMessageProtect);
        if (_receivedMessages.empty()) {
            return totalLen;
        }
        _currPartialRead = _receivedMessages.dequeue();
    }
}

bool UdtSocket::waitForReadyRead(int msecs) {
    if (_isDatagram) {
        return false;
    }
    QDeadlineTimer timeout(msecs);
    QMutexLocker guard(&_receivedMessageProtect);
    while (_receivedMessages.empty() && state() == UdtSocketState::Connected) {
        if (!_receivedMessageCondition.wait(&_receivedMessageProtect, timeout)) {
            break;
        }
    }
    return (!_receivedMessages.empty());
}

qint64 UdtSocket::bytesToWrite() const {
    if (_isDatagram) {
        return 0;
    }
    return _send.bytesToWrite() + QIODevice::bytesToWrite();
}

bool UdtSocket::waitForBytesWritten(int msecs) {
    if (_isDatagram) {
        return false;
    }
    QDeadlineTimer timeout(msecs);
    return _send.waitForPacketSent(timeout);
}

bool UdtSocket::flush() {
    return _send.flush();
}


/*******************************************************************************
 Private functions
*******************************************************************************/

void UdtSocket::sendPacket(const Packet& udtPacket) {
    std::chrono::microseconds ts(_createTime.nsecsElapsed() / 1000);
	_congestion.onPacketSent(udtPacket);
    qCDebug(networking) << localAddressDebugString() << ": sending " << udtPacket._type << " to " << remoteAddressDebugString();
    _multiplexer->sendPacket(_remoteAddr, _remotePort, _farSocketID, ts, udtPacket);
}

template<class T>
T absdiff(T a, T b) {
	if (a < b) {
        return b - a;
	}
	return a - b;
}

void UdtSocket::applyRTT(std::chrono::microseconds rtt) {
    QWriteLocker guard(&_rttProtect);
    _rttVariance = std::chrono::microseconds((_rttVariance * 3 + absdiff(_rtt, rtt)).count() >> 2);
    _rtt = std::chrono::microseconds((_rtt * 7 + rtt).count() >> 3);
}

void UdtSocket::getRTT(std::chrono::microseconds& rtt, std::chrono::microseconds& rttVariance) const {
    QReadLocker guard(&_rttProtect);
	rtt = _rtt;
	rttVariance = _rttVariance;
}

// Update Estimated Bandwidth and packet delivery rate
void UdtSocket::applyReceiveRates(unsigned deliveryRate, unsigned bandwidth) {
    QWriteLocker guard(&_receiveRateProtect);
    if (deliveryRate > 0) {
		_deliveryRate = (_deliveryRate*7 + deliveryRate) >> 3;
	}
	if(bandwidth > 0) {
		_bandwidth = (_bandwidth*7 + bandwidth) >> 3;
	}
}

void UdtSocket::getReceiveRates(unsigned& recvSpeed, unsigned& bandwidth) const {
    QReadLocker guard(&_receiveRateProtect);
    recvSpeed = _deliveryRate;
	bandwidth = _bandwidth;
    return;
}

unsigned UdtSocket::getMaxBandwidth() const {
    // TODO: eventually we may support bandwidth limits.  Right now we do not
    return 0;
}

unsigned UdtSocket::getMaxFlowWinSize() const {
    return _maxFlowWinSize;
}

void UdtSocket::ingestErrorPacket(const Packet& udtPacket) {
    // TODO: this was a packet introduced by the reference implementation but not defined in the spec
    // We don't send it and I'm not really sure what we would do if we received it
    // Right now we do nothing.
}

void UdtSocket::setCongestionWindow(unsigned pkt) {
    _send.setCongestionWindow(pkt);
}

void UdtSocket::setPacketSendPeriod(std::chrono::microseconds snd) {
    _send.setPacketSendPeriod(snd);
}

void UdtSocket::setACKperiod(std::chrono::microseconds ack) {
    _recv.setACKperiod(ack);
}

void UdtSocket::setACKinterval(unsigned ack) {
    _recv.setACKinterval(ack);
}

void UdtSocket::setRTOperiod(std::chrono::microseconds rto) {
    _send.setRTOperiod(rto);
}

void UdtSocket::requestShutdown(UdtSocketState toState, QString error) {
    emit shutdownRequested(toState, error, QPrivateSignal());
}

UdtSocket_CongestionControl& UdtSocket::getCongestionControl() {
    return _congestion;
}

namespace udt4 {

// search through the specified set for the first entry >= key but < limit
std::set<PacketID, WrappedSequenceLess<PacketID>>::iterator findFirstEntry(std::set<PacketID, WrappedSequenceLess<PacketID>>& set, const PacketID& key, const PacketID& limit) {
    std::set<PacketID, WrappedSequenceLess<PacketID>>::iterator lookup = set.lower_bound(key);
    if (key < limit) {
        if (lookup == set.end() || *lookup >= limit) {
            return set.end();
        } else {
            return lookup;
        }
    }
    if (lookup != set.end()) {
        return lookup;
    }
    lookup = set.lower_bound(PacketID(0UL));
    if (lookup == set.end() || *lookup >= limit) {
        return set.end();
    } else {
        return lookup;
    }
}

// search through the specified set for the first entry >= key but < limit
std::set<PacketID, WrappedSequenceLess<PacketID>>::const_iterator findFirstEntry(const std::set<PacketID, WrappedSequenceLess<PacketID>>& set, const PacketID& key, const PacketID& limit) {
    std::set<PacketID, WrappedSequenceLess<PacketID>>::const_iterator lookup = set.lower_bound(key);
    if (key < limit) {
        if (lookup == set.end() || *lookup >= limit) {
            return set.end();
        } else {
            return lookup;
        }
    }
    if (lookup != set.end()) {
        return lookup;
    }
    lookup = set.lower_bound(PacketID(0UL));
    if (lookup == set.end() || *lookup >= limit) {
        return set.end();
    } else {
        return lookup;
    }
}

}  // namespace udt4
