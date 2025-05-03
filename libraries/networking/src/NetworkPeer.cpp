//
//  NetworkPeer.cpp
//  libraries/networking/src
//
//  Created by Stephen Birarda on 2014-10-02.
//  Copyright 2014 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "NetworkPeer.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDataStream>

#include <SharedUtil.h>
#include <UUID.h>

#include "NetworkLogging.h"
#include <Trace.h>
#include "NodeType.h"

const NetworkPeer::LocalID NetworkPeer::NULL_LOCAL_ID;

NetworkPeer::NetworkPeer(QObject* parent) :
    QObject(parent),
    _uuid(),
    _publicSocketIPv4(),
    _publicSocketIPv6(),
    _localSocketIPv4(),
    _localSocketIPv6(),
    _symmetricSocketIPv4(),
    _symmetricSocketIPv6(),
    _activeSocket(NULL),
    _wakeTimestamp(QDateTime::currentMSecsSinceEpoch()),
    _connectionAttempts(0)
{
    _lastHeardMicrostamp = usecTimestampNow();
}

NetworkPeer::NetworkPeer(const QUuid& uuid, const SockAddr& publicSocketIPv4, const SockAddr& publicSocketIPv6,
    const SockAddr& localSocketIPv4, const SockAddr& localSocketIPv6, QObject* parent) :
    QObject(parent),
    _uuid(uuid),
    _publicSocketIPv4(publicSocketIPv4),
    _publicSocketIPv6(publicSocketIPv6),
    _localSocketIPv4(localSocketIPv4),
    _localSocketIPv6(localSocketIPv6),
    _symmetricSocketIPv4(),
    _symmetricSocketIPv6(),
    _activeSocket(NULL),
    _wakeTimestamp(QDateTime::currentMSecsSinceEpoch()),
    _connectionAttempts(0)
{
    _lastHeardMicrostamp = usecTimestampNow();
}

void NetworkPeer::setPublicSocketIPv4(const SockAddr& publicSocket) {
    if (publicSocket != _publicSocketIPv4) {
        if (_activeSocket == &_publicSocketIPv4) {
            // if the active socket was the public socket then reset it to NULL
            _activeSocket = NULL;
        }

        bool wasOldSocketIPv4Null = _publicSocketIPv4.isNull();

        auto previousSocketIPv4 = _publicSocketIPv4;
        _publicSocketIPv4 = publicSocket;
        _publicSocketIPv4.setObjectName(previousSocketIPv4.objectName());

        if (!wasOldSocketIPv4Null) {
            qCDebug(networking) << "Public IPv4 socket change for node" << *this << "; previously" << previousSocketIPv4;
            emit socketUpdated(previousSocketIPv4, _publicSocketIPv4);
        }
    }
}

void NetworkPeer::setPublicSocketIPv6(const SockAddr& publicSocket) {
    if (publicSocket.toIPv6Only() != _publicSocketIPv6) {
        if (_activeSocket == &_publicSocketIPv6) {
            // if the active socket was the public socket then reset it to NULL
            _activeSocket = NULL;
        }

        bool wasOldSocketIPv6Null = _publicSocketIPv6.isNull();

        auto previousSocketIPv6 = _publicSocketIPv6;
        _publicSocketIPv6 = publicSocket;
        _publicSocketIPv6.setObjectName(previousSocketIPv6.objectName());

        if (!wasOldSocketIPv6Null) {
            qCDebug(networking) << "Public IPv6 socket change for node" << *this << "; previously" << previousSocketIPv6;
            emit socketUpdated(previousSocketIPv6, _publicSocketIPv6);
        }
    }
}

void NetworkPeer::setLocalSocketIPv4(const SockAddr& localSocket) {
    if (localSocket.toIPv4Only() != _localSocketIPv4) {
        if (_activeSocket == &_localSocketIPv4) {
            // if the active socket was the local socket then reset it to NULL
            _activeSocket = NULL;
        }
        
        bool wasOldSocketNullIPv4 = _localSocketIPv4.isNull();
        
        auto previousSocket = _localSocketIPv4;
        _localSocketIPv4 = localSocket;
        _localSocketIPv4.setObjectName(previousSocket.objectName());

        if (!wasOldSocketNullIPv4) {
            qCDebug(networking) << "Local IPv4 socket change for node" << *this << "; previously" << previousSocket;
            emit socketUpdated(previousSocket, _localSocketIPv4);
        }
    }
}

void NetworkPeer::setLocalSocketIPv6(const SockAddr& localSocket) {
    if (localSocket.toIPv6Only() != _localSocketIPv6) {
        if (_activeSocket == &_localSocketIPv6) {
            // if the active socket was the local socket then reset it to NULL
            _activeSocket = NULL;
        }

        bool wasOldSocketNullIPv6 = _localSocketIPv6.isNull();

        auto previousSocketIPv6 = _localSocketIPv6;
        _localSocketIPv6 = localSocket;
        _localSocketIPv6.setObjectName(previousSocketIPv6.objectName());

        if (!wasOldSocketNullIPv6) {
            qCDebug(networking) << "Local IPv6 socket change for node" << *this << "; previously" << previousSocketIPv6;
            emit socketUpdated(previousSocketIPv6, _localSocketIPv6);
        }
    }
}

void NetworkPeer::setSymmetricSocket(const SockAddr& symmetricSocket) {
    if (symmetricSocket.toIPv4Only() != _symmetricSocketIPv4 || symmetricSocket.toIPv6Only() != _symmetricSocketIPv6) {
        if (_activeSocket == &_symmetricSocketIPv4 || _activeSocket == &_symmetricSocketIPv6) {
            // if the active socket was the symmetric socket then reset it to NULL
            _activeSocket = NULL;
        }
        
        bool wasOldSocketNullIPv4 = _symmetricSocketIPv4.isNull();
        
        auto previousSocketIPv4 = _symmetricSocketIPv4;
        _symmetricSocketIPv4 = symmetricSocket;
        _symmetricSocketIPv4.setObjectName(previousSocketIPv4.objectName());
        
        if (!wasOldSocketNullIPv4) {
            qCDebug(networking) << "Symmetric socket change for node" << *this << "; previously" << previousSocketIPv4;
            emit socketUpdated(previousSocketIPv4, _symmetricSocketIPv4);
        }

        bool wasOldSocketNull = _symmetricSocketIPv6.isNull();

        auto previousSocketIPv6 = _symmetricSocketIPv6;
        _symmetricSocketIPv6 = symmetricSocket;
        _symmetricSocketIPv6.setObjectName(previousSocketIPv6.objectName());

        if (!wasOldSocketNull) {
            qCDebug(networking) << "Symmetric socket change for node" << *this << "; previously" << previousSocketIPv6;
            emit socketUpdated(previousSocketIPv6, _symmetricSocketIPv6);
        }
    }
}

void NetworkPeer::setActiveSocket(SockAddr* discoveredSocket) {
    _activeSocket = discoveredSocket;

    // we have an active socket, stop our ping timer
    stopPingTimer();

    // we're now considered connected to this peer - reset the number of connection attempts
    resetConnectionAttempts();
    
    if (_activeSocket) {
        emit socketActivated(*_activeSocket);
    }
}

void NetworkPeer::activateLocalSocket(QAbstractSocket::NetworkLayerProtocol protocol) {
    if (protocol == QAbstractSocket::IPv4Protocol) {
        if (_activeSocket != &_localSocketIPv4) {
            qCDebug(networking) << "Activating local socket for network peer with ID" << uuidStringWithoutCurlyBraces(_uuid);
            setActiveSocket(&_localSocketIPv4);
        }
    } else {
        if (_activeSocket != &_localSocketIPv6) {
            qCDebug(networking) << "Activating local socket for network peer with ID" << uuidStringWithoutCurlyBraces(_uuid);
            setActiveSocket(&_localSocketIPv6);
        }
    }
}

void NetworkPeer::activatePublicSocket(QAbstractSocket::NetworkLayerProtocol protocol) {
    if (protocol == QAbstractSocket::IPv4Protocol) {
        if (_activeSocket != &_publicSocketIPv4) {
            qCDebug(networking) << "Activating public socket for network peer with ID" << uuidStringWithoutCurlyBraces(_uuid);
            setActiveSocket(&_publicSocketIPv4);
        }
    } else {
        if (_activeSocket != &_publicSocketIPv6) {
            qCDebug(networking) << "Activating public socket for network peer with ID" << uuidStringWithoutCurlyBraces(_uuid);
            setActiveSocket(&_publicSocketIPv6);
        }
    }
}

void NetworkPeer::activateSymmetricSocket(QAbstractSocket::NetworkLayerProtocol protocol) {
    if (protocol == QAbstractSocket::IPv4Protocol) {
        if (_activeSocket != &_symmetricSocketIPv4) {
            qCDebug(networking) << "Activating symmetric socket (" << _symmetricSocketIPv4 << ") for network peer with ID" << uuidStringWithoutCurlyBraces(_uuid);
            setActiveSocket(&_symmetricSocketIPv4);
        }
    } else {
        if (_activeSocket != &_symmetricSocketIPv6) {
            qCDebug(networking) << "Activating symmetric socket (" << _symmetricSocketIPv6 << ") for network peer with ID" << uuidStringWithoutCurlyBraces(_uuid);
            setActiveSocket(&_symmetricSocketIPv6);
        }
    }
}

void NetworkPeer::activateMatchingOrNewSymmetricSocket(const SockAddr& matchableSockAddr) {
    Q_ASSERT(!(matchableSockAddr.getAddressIPv6().protocol() == QAbstractSocket::IPv6Protocol
        && matchableSockAddr.getAddressIPv4().protocol() == QAbstractSocket::IPv4Protocol));
    if (matchableSockAddr == _publicSocketIPv4) {
        activatePublicSocket(QAbstractSocket::IPv4Protocol);
    } else     if (matchableSockAddr == _publicSocketIPv6) {
        activatePublicSocket(QAbstractSocket::IPv6Protocol);
    } else if (matchableSockAddr == _localSocketIPv4) {
        activateLocalSocket(QAbstractSocket::IPv4Protocol);
    } else if (matchableSockAddr == _localSocketIPv6) {
        activateLocalSocket(QAbstractSocket::IPv6Protocol);
    } else {
        // set the Node's symmetric socket to the passed socket
        setSymmetricSocket(matchableSockAddr);
        // TODO(IPv6): what to do here?
        if (matchableSockAddr.getAddressIPv6().protocol() == QAbstractSocket::IPv6Protocol) {
            activateSymmetricSocket(QAbstractSocket::IPv6Protocol);
        } else {
            activateSymmetricSocket(QAbstractSocket::IPv4Protocol);
        }
    }
}

void NetworkPeer::softReset() {
    qCDebug(networking) << "Soft reset ";
    // a soft reset should clear the sockets and reset the number of connection attempts
    _localSocketIPv4.clear();
    _localSocketIPv6.clear();
    _publicSocketIPv4.clear();
    _publicSocketIPv6.clear();
    _symmetricSocketIPv4.clear();
    _symmetricSocketIPv6.clear();
    _activeSocket = NULL;

    // stop our ping timer since we don't have sockets to ping anymore anyways
    stopPingTimer();

    _connectionAttempts = 0;
}

void NetworkPeer::reset() {
    softReset();

    _uuid = QUuid();
    _wakeTimestamp = QDateTime::currentMSecsSinceEpoch();
    _lastHeardMicrostamp = usecTimestampNow();
}

QByteArray NetworkPeer::toByteArray() const {
    QByteArray peerByteArray;

    QDataStream peerStream(&peerByteArray, QIODevice::Append);
    peerStream << *this;

    return peerByteArray;
}

void NetworkPeer::startPingTimer() {
    if (!_pingTimer) {
        _pingTimer = new QTimer(this);

        connect(_pingTimer, &QTimer::timeout, this, &NetworkPeer::pingTimerTimeout);

        _pingTimer->start(UDP_PUNCH_PING_INTERVAL_MS);
    }
}

void NetworkPeer::stopPingTimer() {
    if (_pingTimer) {
        _pingTimer->stop();
        _pingTimer->deleteLater();
        _pingTimer = NULL;
    }
}

QDataStream& operator<<(QDataStream& out, const NetworkPeer& peer) {
    out << peer._uuid;
    out << peer._publicSocketIPv4;
    out << peer._publicSocketIPv6;
    out << peer._localSocketIPv4;
    out << peer._localSocketIPv6;

    return out;
}

QDataStream& operator>>(QDataStream& in, NetworkPeer& peer) {
    in >> peer._uuid;
    in >> peer._publicSocketIPv4;
    in >> peer._publicSocketIPv6;
    in >> peer._localSocketIPv4;
    in >> peer._localSocketIPv6;

    return in;
}

QDebug operator<<(QDebug debug, const NetworkPeer &peer) {
    debug << uuidStringWithoutCurlyBraces(peer.getUUID())
        << "- public:" << peer.getPublicSocketIPv4() << " " << peer.getPublicSocketIPv6()
        << "- local:" << peer.getLocalSocketIPv4() << " " << peer.getLocalSocketIPv6();
    return debug;
}
