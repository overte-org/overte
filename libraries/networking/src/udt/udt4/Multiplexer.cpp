//
//  Multiplexer.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "Multiplexer.h"

#include "../../ByteSlice.h"
#include "../../NetworkLogging.h"
#include <QtCore/QException>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMutexLocker>
#include <QtNetwork/QNetworkDatagram>
#include <QtCore/QRandomGenerator>
#include "UdtSocket.h"

#ifdef WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

using namespace udt4;

QMutex UdtMultiplexer::gl_multiplexerMapProtect;
UdtMultiplexer::TMultiplexerMap UdtMultiplexer::gl_multiplexerMap;

// getInstance gets or creates a multiplexer for the given local address.
UdtMultiplexerPointer UdtMultiplexer::getInstance(quint16 port,
                                                  const QHostAddress& localAddress /* = QHostAddress::Any */,
                                                  QAbstractSocket::SocketError* serverError /* = nullptr */,
                                                  QString* errorString /* = nullptr */) {
    UdtMultiplexerPointer multiplexer = lookupInstance(port, localAddress);
    if (!multiplexer.isNull() && multiplexer->isLive()) {
        return multiplexer;
    }

    // No multiplexer, need to create connection
    multiplexer = UdtMultiplexerPointer(new UdtMultiplexer());
    if (!multiplexer->create(port, localAddress)) {
        // if we hit an exception trying to construct a multiplexer, check to see if we haven't hit a race condition
        multiplexer = lookupInstance(port, localAddress);
        if (!multiplexer.isNull() && multiplexer->isLive()) {
            return multiplexer;
        }

        // we haven't, throw our exception
        if (serverError != nullptr) {
            *serverError = multiplexer->serverError();
        }
        if (errorString != nullptr) {
            *errorString = multiplexer->errorString();
        }
        return UdtMultiplexerPointer();
    }

    {
        TLocalPortPair resultLocalPort(multiplexer->serverPort(), multiplexer->serverAddress().toString());
        QMutexLocker locker(&gl_multiplexerMapProtect);
        gl_multiplexerMap.insert(resultLocalPort, multiplexer);
    }
    return multiplexer;
}

UdtMultiplexerPointer UdtMultiplexer::lookupInstance(quint16 port, const QHostAddress& localAddress) {
    // we don't even attempt to lookup port=0 (which should normally be handled as "pick an ephemeral port")
    if (port == 0) {
        return UdtMultiplexerPointer();
    }

    UdtMultiplexerPointer multiplexer;

    {
        TLocalPortPair localPort(port, localAddress.toString());
        QMutexLocker locker(&gl_multiplexerMapProtect);
        TMultiplexerMap::const_iterator lookup = gl_multiplexerMap.find(localPort);
        if (lookup != gl_multiplexerMap.end()) {
            multiplexer = lookup.value().lock();
        }
    }

    // if we found it on the first try then return what we've got
    if (multiplexer || (localAddress != QHostAddress::AnyIPv4 && localAddress != QHostAddress::AnyIPv6)) {
        return multiplexer;
    }

    // at this point we haven't found a multiplexer and we're using AnyIPv4/AnyIPv6.  Retry with Any
    {
        QHostAddress anyAddress(QHostAddress::Any);
        TLocalPortPair localPort(port, anyAddress.toString());
        QMutexLocker locker(&gl_multiplexerMapProtect);
        TMultiplexerMap::const_iterator lookup = gl_multiplexerMap.find(localPort);
        if (lookup != gl_multiplexerMap.end()) {
            multiplexer = lookup.value().lock();
        }
    }

    return multiplexer;
}

UdtMultiplexer::UdtMultiplexer() {
    QRandomGenerator random;
    _nextSid = random.generate();

    connect(&_udpSocket, &QUdpSocket::readyRead, this, &UdtMultiplexer::onPacketReadReady, Qt::DirectConnection);
    connect(this, &UdtMultiplexer::readySendPacket, this, &UdtMultiplexer::onPacketWriteReady);
}

UdtMultiplexer::~UdtMultiplexer() {
    // deregister this multiplexer
    TLocalPortPair localPort(_serverPort, _serverAddress.toString());
    {
        QMutexLocker locker(&gl_multiplexerMapProtect);
        TMultiplexerMap::iterator lookup = gl_multiplexerMap.find(localPort);
        if (lookup != gl_multiplexerMap.end()) {
            gl_multiplexerMap.erase(lookup);
        }
    }

    // tear everything down
    _udpSocket.close();

    _readThread.quit();
    _writeThread.quit();
}

bool UdtMultiplexer::create(quint16 port, const QHostAddress& localAddress) {
    _readThread.start();
    _writeThread.start();
    moveToReadThread(&_udpSocket);
    moveToWriteThread(this);

    if (!_udpSocket.bind(localAddress, port)) {
        return false;
    }
    _serverAddress = _udpSocket.localAddress();
    _serverPort = _udpSocket.localPort();

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
        auto sd = _udpSocket.socketDescriptor();
        int val = 0;  // false
        if (setsockopt(sd, IPPROTO_IP, IP_DONTFRAGMENT, reinterpret_cast<const char*>(&val), sizeof(val))) {
            auto wsaErr = WSAGetLastError();
            qCWarning(networking) << "Socket::bind Cannot setsockopt IP_DONTFRAGMENT" << wsaErr;
        }
    }
#endif

    for (int i = 0; i < 2; i++) {
        QAbstractSocket::SocketOption bufferOpt;
        QString bufferTypeString;

        int numBytes = 0;

        if (i == 0) {
            bufferOpt = QAbstractSocket::SendBufferSizeSocketOption;
            numBytes = UDP_SEND_BUFFER_SIZE_BYTES;
            bufferTypeString = "send";

        } else {
            bufferOpt = QAbstractSocket::ReceiveBufferSizeSocketOption;
            numBytes = UDP_RECEIVE_BUFFER_SIZE_BYTES;
            bufferTypeString = "receive";
        }

        int oldBufferSize = _udpSocket.socketOption(bufferOpt).toInt();

        if (oldBufferSize < numBytes) {
            _udpSocket.setSocketOption(bufferOpt, QVariant(numBytes));
            int newBufferSize = _udpSocket.socketOption(bufferOpt).toInt();

            qCDebug(networking) << "Changed socket" << bufferTypeString << "buffer size from" << oldBufferSize << "to"
                                << newBufferSize << "bytes";
        } else {
            // don't make the buffer smaller
            qCDebug(networking) << "Did not change socket" << bufferTypeString << "buffer size from" << oldBufferSize
                                << "since it is larger than desired size of" << numBytes;
        }
    }

    return true;
}

/*
// Adapted from https://github.com/hlandau/degoutils/blob/master/net/mtu.go
const absMaxDatagramSize = 2147483646 // 2**31-2
func discoverMTU(ourIP net.IP) (uint, error) {

	ifaces, err := net.Interfaces()
	if err != nil {
		return 65535, err
	}

	var filtered []net.Interface
	for _, iface := range ifaces {
		addrs, err := iface.Addrs()
		if err != nil {
			log.Printf("cannot retrieve iface addresses for %s: %s", iface.Name, err.Error())
			continue
		}
		for _, a := range addrs {
			var ipnet *net.IPNet
			switch v := a.(type) {
			case *net.IPAddr:
				ipnet = &net.IPNet{IP: v.IP, Mask: v.IP.DefaultMask()}
			case *net.IPNet:
				ipnet = v
			}
			if ipnet == nil {
				log.Printf("cannot retrieve IPNet from address %s on interface %s", a.String(), iface.Name)
				continue
			}
			if ipnet.Contains(ourIP) {
				filtered = append(filtered, iface)
			}
		}
	}
	if len(filtered) == 0 {
		log.Printf("cannot identify interface(s) associated with %s, doing blind search", ourIP.String())
		filtered = ifaces
	}

	var mtu int = 65535
	for _, iface := range filtered {
		if iface.Flags&(net.FlagUp|net.FlagLoopback) == net.FlagUp && iface.MTU > mtu {
			mtu = iface.MTU
		}
	}
	if mtu > absMaxDatagramSize {
		mtu = absMaxDatagramSize
	}
	return uint(mtu), nil
}
*/
void UdtMultiplexer::newSocket(UdtSocketPointer socket) {
    quint32 socketID = _nextSid.fetchAndSubRelaxed(1);
    socket->setLocalSocketID(socketID);

    QMutexLocker locker(&_connectedSocketsProtect);
    _connectedSockets.insert(socketID, socket.get());
}

bool UdtMultiplexer::closeSocket(quint32 sockID) {
    QMutexLocker locker(&_connectedSocketsProtect);
    TSocketMap::iterator lookup = _connectedSockets.find(sockID);
    if (lookup != _connectedSockets.end()) {
        _connectedSockets.erase(lookup);
    }
    return lookup != _connectedSockets.end();
}

void UdtMultiplexer::onPacketReadReady() {  // executes from goRead thread
    while (_udpSocket.hasPendingDatagrams()) {
        ByteSlice packetData;
        qint64 packetSize = _udpSocket.pendingDatagramSize();
        void* packetBuffer = packetData.create(packetSize);
        QHostAddress peerAddress;
        quint16 peerPort;
        if (-1 == _udpSocket.readDatagram(reinterpret_cast<char*>(packetBuffer), packetSize, &peerAddress, &peerPort)) {
            qCWarning(networking) << "Received an invalid UDP packet (error likely logged nearby)";
            continue;
        }

        Packet udtPacket(packetData);

        // attempt to route the packet
        if (udtPacket._socketID == 0) {
            if (udtPacket._type != PacketType::Handshake) {
                qCWarning(networking) << "Received non-handshake packet with destination socket = 0";
                continue;
            }
            HandshakePacket hsPacket(udtPacket, peerAddress.protocol());

            switch (hsPacket._reqType) {
                case HandshakePacket::RequestType::Rendezvous:
                    {
                        QMutexLocker locker(&_rendezvousHandshakesProtect);
                        _rendezvousHandshakes.append(PacketEventPointer<HandshakePacket>::create(hsPacket, peerAddress, peerPort));
                    }
                    emit readyRendezvousHandshake();
                    break;
                case HandshakePacket::RequestType::Request: {
                    bool wasEmpty;
                    {
                        QMutexLocker locker(&_serverHandshakesProtect);
                        wasEmpty = _serverHandshakes.isEmpty();
                        _serverHandshakes.enqueue(PacketEventPointer<HandshakePacket>::create(hsPacket, peerAddress, peerPort));
                    }
                    if (wasEmpty) {
                        emit readyServerHandshake();
                    }
                    break;
                }
                default:
                    qCInfo(networking) << "Received handshake packet directed at sockID=0 with unexpected request type="
                                       << static_cast<uint>(hsPacket._reqType);
                    break;
            }
            return;
        }

        UdtSocket* destSocket = nullptr;
        {
            QMutexLocker locker(&_connectedSocketsProtect);
            TSocketMap::const_iterator lookup = _connectedSockets.find(udtPacket._socketID);
            if (lookup != _connectedSockets.end()) {
                destSocket = lookup.value();
            }
        }
        if (destSocket) {
            destSocket->readPacket(udtPacket, peerAddress, peerPort);
        }
    }
}

/*
write runs in a goroutine and writes packets to conn using a buffer from the
writeBufferPool, or a new buffer.
*/
void UdtMultiplexer::onPacketWriteReady() {
    for (;;) {
        PacketEventPointer<Packet> thisPacket;
        {
            QMutexLocker locker(&_sendPacketProtect);
            if (_sendPacket.isEmpty()) {
                return;
            }
            thisPacket = _sendPacket.dequeue();
            if (thisPacket.isNull()) {
                return;
            }
        }

        ByteSlice networkPacket = thisPacket->packet.toNetworkPacket();
        if (0 > _udpSocket.writeDatagram(reinterpret_cast<const char*>(networkPacket.constData()), networkPacket.length(),
                                         thisPacket->peerAddress, thisPacket->peerPort)) {
            qCWarning(networking) << "Error sending packet to " << thisPacket->peerAddress.toString() << ":"
                                  << thisPacket->peerPort << "[" << thisPacket->packet._socketID << "] - ("
                                  << _udpSocket.error() << ")" << _udpSocket.errorString();
        }
    }
}

void UdtMultiplexer::sendPacket(const QHostAddress& destAddr,
                                quint32 destPort,
                                quint32 destSockID,
                                std::chrono::microseconds timestamp,
                                Packet packet) {
    packet._socketID = destSockID;
    packet._timestamp = timestamp;
    if (destSockID == 0 && packet._type != PacketType::Handshake) {
        qCCritical(networking) << "Attempt to send non-handshake packet with destination socket = 0";
        return;
    }

    QMutexLocker locker(&_sendPacketProtect);
    _sendPacket.enqueue(PacketEventPointer<Packet>::create(packet, destAddr, destPort));
    emit readySendPacket(QPrivateSignal());
}

PacketEventPointer<HandshakePacket> UdtMultiplexer::nextServerHandshake() {
    QMutexLocker locker(&_serverHandshakesProtect);
    pruneServerHandshakes();
    if (_serverHandshakes.isEmpty()) {
        return PacketEventPointer<HandshakePacket>();
    }
    return _serverHandshakes.dequeue();
}

void UdtMultiplexer::pruneServerHandshakes() {  // ASSUMES WE ARE HOLDING _serverHandshakesProtect
    while (!_serverHandshakes.isEmpty()) {
        const PacketEventPointer<HandshakePacket>& firstPacket = _serverHandshakes.head();
        if (firstPacket->age.nsecsElapsed() < MAX_SERVER_HANDSHAKE_AGE) {
            return;
        }
        _serverHandshakes.dequeue();
    }
}

PacketEventPointer<HandshakePacket> UdtMultiplexer::nextRendezvousHandshake(const QHostAddress& peerAddress, quint32 peerPort) {
    QMutexLocker locker(&_rendezvousHandshakesProtect);
    pruneRendezvousHandshakes();
    for (THandshakeList::iterator trans = _rendezvousHandshakes.begin(); trans != _rendezvousHandshakes.end(); ++trans) {
        PacketEventPointer<HandshakePacket> thisPacket = *trans;
        if (thisPacket->peerAddress == peerAddress && thisPacket->peerPort == peerPort) {
            _rendezvousHandshakes.erase(trans);
            return thisPacket;
        }
    }
    return PacketEventPointer<HandshakePacket>();
}

void UdtMultiplexer::pruneRendezvousHandshakes() {  // ASSUMES WE ARE HOLDING _rendezvousHandshakesProtect
    while (!_rendezvousHandshakes.isEmpty()) {
        const PacketEventPointer<HandshakePacket>& firstPacket = _rendezvousHandshakes.front();
        if (firstPacket->age.nsecsElapsed() < MAX_SERVER_HANDSHAKE_AGE) {
            return;
        }
        _serverHandshakes.pop_front();
    }
}
