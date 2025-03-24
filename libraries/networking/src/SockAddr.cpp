//
//  SockAddr.cpp
//  libraries/networking/src
//
//  Created by Stephen Birarda on 11/26/2013.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "SockAddr.h"

#include <qdatastream.h>
#include <qhostinfo.h>
#include <qnetworkinterface.h>

#include "NetworkLogging.h"

#ifdef WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

int sockAddrMetaTypeId = qRegisterMetaType<SockAddr>();

SockAddr::SockAddr() :
    _socketType(SocketType::Unknown), _addressIPv4(), _addressIPv6(),
    _port(0)
{
}

SockAddr::SockAddr(SocketType socketType, const QHostAddress& addressV4, const QHostAddress& addressV6, quint16 port) :
    _socketType(socketType), _addressIPv4(addressV4), _addressIPv6(addressV6),
    _port(port)
{
    Q_ASSERT(_addressIPv4.protocol() == QAbstractSocket::IPv4Protocol || _addressIPv4.isNull());
    Q_ASSERT(_addressIPv6.protocol() == QAbstractSocket::IPv6Protocol || _addressIPv6.isNull());
}

SockAddr::SockAddr(const SockAddr& otherSockAddr) :
    QObject(),
    _socketType(otherSockAddr._socketType), _addressIPv4(otherSockAddr._addressIPv4),
    _addressIPv6(otherSockAddr._addressIPv6),
    _port(otherSockAddr._port)
{
    setObjectName(otherSockAddr.objectName());
}

SockAddr& SockAddr::operator=(const SockAddr& rhsSockAddr) {
    setObjectName(rhsSockAddr.objectName());
    _socketType = rhsSockAddr._socketType;
    _addressIPv4 = rhsSockAddr._addressIPv4;
    _addressIPv6 = rhsSockAddr._addressIPv6;
    _port = rhsSockAddr._port;
    return *this;
}

SockAddr::SockAddr(SocketType socketType, const QString& hostname, quint16 hostOrderPort, bool shouldBlockForLookup, QAbstractSocket::NetworkLayerProtocol protocolToUse) :
    _socketType(socketType), _addressIPv4(), _addressIPv6(),
    _port(hostOrderPort)
{
    // if we parsed an IPv4 address out of the hostname, don't look it up
    QHostAddress address(hostname);
    if (address.protocol() != QAbstractSocket::IPv4Protocol && address.protocol() != QAbstractSocket::IPv6Protocol) {
        // lookup the IP by the hostname
        if (shouldBlockForLookup) {
            qCDebug(networking) << "Synchronously looking up IP address for hostname" << hostname << "for" << socketType << "socket on port" << hostOrderPort;
            QHostInfo result = QHostInfo::fromName(hostname);
            handleLookupResult(result, protocolToUse);
        } else {
            qCDebug(networking) << "Asynchronously looking up IP address for hostname" << hostname << "for" << socketType << "socket on port" << hostOrderPort;
            if (protocolToUse == QAbstractSocket::IPv4Protocol) {
                int lookupID = QHostInfo::lookupHost(hostname, this, &SockAddr::handleLookupResultIPv4Only);
                qCDebug(networking) << "Lookup ID for " << hostname << "is" << lookupID;
            } else if (protocolToUse == QAbstractSocket::IPv6Protocol) {
                int lookupID = QHostInfo::lookupHost(hostname, this, &SockAddr::handleLookupResultIPv6Only);
                qCDebug(networking) << "Lookup ID for " << hostname << "is" << lookupID;
            } else {
                int lookupID = QHostInfo::lookupHost(hostname, this, &SockAddr::handleLookupResultAnyIP);
                qCDebug(networking) << "Lookup ID for " << hostname << "is" << lookupID;
            }
        }
    }
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        _addressIPv4 = address;
    }
    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        _addressIPv6 = address;
    }
}

void SockAddr::swap(SockAddr& otherSockAddr) {
    using std::swap;

    swap(_socketType, otherSockAddr._socketType);
    swap(_addressIPv4, otherSockAddr._addressIPv4);
    swap(_addressIPv6, otherSockAddr._addressIPv6);
    swap(_port, otherSockAddr._port);

    // Swap objects name
    auto temp = otherSockAddr.objectName();
    otherSockAddr.setObjectName(objectName());
    setObjectName(temp);
}

bool SockAddr::operator==(const SockAddr& rhsSockAddr) const {
    return _socketType == rhsSockAddr._socketType && _addressIPv4 == rhsSockAddr._addressIPv4 &&
           _addressIPv6 == rhsSockAddr._addressIPv6 && _port == rhsSockAddr._port;
}

void SockAddr::setAddress(const QHostAddress& address) {
    if (address.protocol() == QAbstractSocket::IPv4Protocol) {
        _addressIPv4 = address;
    } else if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        _addressIPv6 = address;
    } else {
        Q_ASSERT(false);
    }
}

void SockAddr::handleLookupResultIPv4Only(const QHostInfo& hostInfo) {
    handleLookupResult(hostInfo, QAbstractSocket::IPv4Protocol);
}

void SockAddr::handleLookupResultIPv6Only(const QHostInfo& hostInfo) {
    handleLookupResult(hostInfo, QAbstractSocket::IPv6Protocol);
}

void SockAddr::handleLookupResultAnyIP(const QHostInfo& hostInfo) {
    handleLookupResult(hostInfo, QAbstractSocket::AnyIPProtocol);
}

void SockAddr::handleLookupResult(const QHostInfo& hostInfo, QAbstractSocket::NetworkLayerProtocol protocolToUse) {
    qCDebug(networking) << "handleLookupResult for" << hostInfo.lookupId();

    if (hostInfo.error() != QHostInfo::NoError) {
        qCDebug(networking) << "Lookup failed for" << hostInfo.lookupId() << ":" << hostInfo.errorString();
        emit lookupFailed();
    } else {
        bool addressFound = false;
        foreach(const QHostAddress& address, hostInfo.addresses()) {
            // just take the first IPv4 address
            // TODO(IPv6): what to do about IPv4 and IPv6?
            qDebug() << "SockAddr::handleLookupResult hostInfo: " << hostInfo.addresses();
            if (address.protocol() == QAbstractSocket::IPv4Protocol
                && (protocolToUse == QAbstractSocket::IPv4Protocol || protocolToUse == QAbstractSocket::AnyIPProtocol)) {
                _addressIPv4 = address;
                qCDebug(networking) << "QHostInfo lookup result for"
                    << hostInfo.hostName() << "with lookup ID" << hostInfo.lookupId() << "is" << address.toString();
                addressFound = true;
            }
            if (address.protocol() == QAbstractSocket::IPv6Protocol
                && (protocolToUse == QAbstractSocket::IPv6Protocol || protocolToUse == QAbstractSocket::AnyIPProtocol)) {
                _addressIPv6 = address;
                qCDebug(networking) << "QHostInfo lookup result for"
                                    << hostInfo.hostName() << "with lookup ID" << hostInfo.lookupId() << "is" << address.toString();
                addressFound = true;
            }
        }
        if (addressFound) {
            emit lookupCompleted();
        } else {
            emit lookupFailed();
        }
    }
}

QString SockAddr::toString() const {
    if (!_addressIPv4.isNull() && _addressIPv6.isNull()) {
        return SocketTypeToString::socketTypeToString(_socketType) + " " + _addressIPv4.toString() + ":" + QString::number(_port);
    } else if (!_addressIPv4.isNull() && _addressIPv6.isNull()) {
        // TODO(IPv6): How to format IPv6?
        Q_ASSERT(false);
    } else if (!_addressIPv4.isNull() && !_addressIPv6.isNull()) {
        // TODO(IPv6): What to do if both are valid?
        Q_ASSERT(false);
        return SocketTypeToString::socketTypeToString(_socketType) + " " + _addressIPv4.toString() + ":" + QString::number(_port);
    } else {
        // TODO(IPv6): What to do if none are valid?
        Q_ASSERT(false);
        return "";
    }
}

QString SockAddr::toShortString() const {
    if (!_addressIPv4.isNull() && _addressIPv6.isNull()) {
        return _addressIPv4.toString() + ":" + QString::number(_port);
    } else if (!_addressIPv4.isNull() && _addressIPv6.isNull()) {
        // TODO(IPv6): How to format IPv6?
        Q_ASSERT(false);
    } else if (!_addressIPv4.isNull() && !_addressIPv6.isNull()) {
        // TODO(IPv6): What to do if both are valid?
        Q_ASSERT(false);
        return _addressIPv4.toString() + ":" + QString::number(_port);
    } else {
        // TODO(IPv6): What to do if none are valid?
        Q_ASSERT(false);
        return "";
    }
}

bool SockAddr::hasPrivateAddress() const {
    // an address is private if it is loopback or falls in any of the RFC1918 address spaces
    const QPair<QHostAddress, int> TWENTY_FOUR_BIT_BLOCK = { QHostAddress("10.0.0.0"), 8 };
    const QPair<QHostAddress, int> TWENTY_BIT_BLOCK = { QHostAddress("172.16.0.0") , 12 };
    const QPair<QHostAddress, int> SIXTEEN_BIT_BLOCK = { QHostAddress("192.168.0.0"), 16 };

    // TODO(IPv6): How to do this for IPv6?
    return _addressIPv4.isLoopback()
        || _addressIPv4.isInSubnet(TWENTY_FOUR_BIT_BLOCK)
        ||
           _addressIPv4.isInSubnet(TWENTY_BIT_BLOCK)
        || _addressIPv4.isInSubnet(SIXTEEN_BIT_BLOCK);
}

QDebug operator<<(QDebug debug, const SockAddr& sockAddr) {
    debug.nospace()
        << (sockAddr._socketType != SocketType::Unknown
            ? (SocketTypeToString::socketTypeToString(sockAddr._socketType) + " ") : QString(""))
        << sockAddr._addressIPv4.toString() << ":" << sockAddr._port << " "
        << sockAddr._addressIPv6.toString() << ":" << sockAddr._port;
    return debug.space();
}

QDataStream& operator<<(QDataStream& dataStream, const SockAddr& sockAddr) {
    // Don't include socket type because ICE packets must not have it.
    // TODO(IPv6): How to add IPv6 here without braking the protocol
    dataStream << sockAddr._addressIPv4 << sockAddr._addressIPv6 << sockAddr._port;
    return dataStream;
}

QDataStream& operator>>(QDataStream& dataStream, SockAddr& sockAddr) {
    // Don't include socket type because ICE packets must not have it.
    // TODO(IPv6): How to add IPv6 here without braking the protocol
    dataStream >> sockAddr._addressIPv4 >> sockAddr._addressIPv6 >> sockAddr._port;
    return dataStream;
}

uint qHash(const SockAddr& key, uint seed) {
    // use the existing QHostAddress and quint16 hash functions to get our hash
    return qHash(key.getAddressIPv4(), seed) ^ qHash(key.getAddressIPv6(), seed) ^ qHash(key.getPort(), seed);
}
