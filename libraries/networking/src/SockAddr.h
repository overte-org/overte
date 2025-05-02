//
//  SockAddr.h
//  libraries/networking/src
//
//  Created by Stephen Birarda on 11/26/2013.
//  Copyright 2013 High Fidelity, Inc.
//  Copyright 2021 Vircadia contributors.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SockAddr_h
#define hifi_SockAddr_h

#include <cstdint>
#include <string>

struct sockaddr;

#include <QtNetwork/QHostInfo>

#include "SocketType.h"


class SockAddr : public QObject {
    Q_OBJECT
public:
    SockAddr();
    SockAddr(SocketType socketType, const QHostAddress& addressV4, const QHostAddress& addressV6, quint16 port);
    SockAddr(const SockAddr& otherSockAddr);
    // ipV4Only is used for STUN.
    SockAddr(SocketType socketType, const QString& hostname, quint16 hostOrderPort, bool shouldBlockForLookup = false,
             QAbstractSocket::NetworkLayerProtocol protocolToUse = QAbstractSocket::AnyIPProtocol);

    // TODO(IPv6): original code had && too but is this correct? Shouldn't it be invalid if either port or address is invalid?
    // fo0r now I changed it to ||
    //bool isNull() const { return _addressIPv4.isNull() && _addressIPv6.isNull() && _port == 0; }
    bool isNull() const { return (_addressIPv4.isNull() && _addressIPv6.isNull()) || _port == 0; }
    void clear() {
        _addressIPv4.clear();
        _addressIPv6.clear(); _port = 0;}

    SockAddr& operator=(const SockAddr& rhsSockAddr);
    void swap(SockAddr& otherSockAddr);

    bool operator==(const SockAddr& rhsSockAddr) const;
    bool operator!=(const SockAddr& rhsSockAddr) const { return !(*this == rhsSockAddr); }

    SocketType getType() const { return _socketType; }
    SocketType* getSocketTypePointer() { return &_socketType; }
    void setType(const SocketType socketType) { _socketType = socketType; }

    const QHostAddress& getAddressIPv6() const { return _addressIPv6; }
    const QHostAddress& getAddressIPv4() const { return _addressIPv4; }
    QHostAddress* getAddressPointerIPv6() { return &_addressIPv6; }
    QHostAddress* getAddressPointerIPv4() { return &_addressIPv4; }
    void setAddress(const QHostAddress& address);

    SockAddr toIPv6Only() const { return {_socketType, QHostAddress(), _addressIPv6, _port}; }; // TODO(IPv6): set qobject name too
    SockAddr toIPv4Only() const { return {_socketType, _addressIPv4, QHostAddress(), _port}; };

    quint16 getPort() const { return _port; }
    quint16* getPortPointer() { return &_port; }
    void setPort(quint16 port) { _port = port; }

    static int packSockAddr(unsigned char* packetData, const SockAddr& packSockAddr);
    static int unpackSockAddr(const unsigned char* packetData, SockAddr& unpackDestSockAddr);

    QString toString() const;
    QString toShortString() const;

    bool hasPrivateAddress() const; // checks if the address behind this sock addr is private per RFC 1918

    friend QDebug operator<<(QDebug debug, const SockAddr& sockAddr);
    friend QDataStream& operator<<(QDataStream& dataStream, const SockAddr& sockAddr);
    friend QDataStream& operator>>(QDataStream& dataStream, SockAddr& sockAddr);

private slots:
    // In some cases, for example for STUN IPv4-ony lookup is needed.
    void handleLookupResult(const QHostInfo& hostInfo, QAbstractSocket::NetworkLayerProtocol protocolToUse);
    void handleLookupResultIPv4Only(const QHostInfo& hostInfo);
    void handleLookupResultIPv6Only(const QHostInfo& hostInfo);
    void handleLookupResultAnyIP(const QHostInfo& hostInfo);
signals:
    void lookupCompleted();
    void lookupFailed();
private:
    SocketType _socketType { SocketType::Unknown };
    QHostAddress _addressIPv4;
    QHostAddress _addressIPv6;
    quint16 _port;
};

uint qHash(const SockAddr& key, uint seed);

namespace std {
    template <>
    struct hash<SockAddr> {
        // NOTE: this hashing specifically ignores IPv6 addresses - if we begin to support those we will need
        // to conditionally hash the bytes that represent an IPv6 address
        size_t operator()(const SockAddr& sockAddr) const {
            // use XOR of implemented std::hash templates for new hash
            // depending on the type of address we're looking at
            uint hashResult = 0;

            if (!sockAddr.getAddressIPv4().isNull()) {
                Q_ASSERT(sockAddr.getAddressIPv4().protocol() == QAbstractSocket::IPv4Protocol);
                hashResult ^= hash<uint32_t>()((uint32_t)sockAddr.getAddressIPv4().toIPv4Address());
            }
            if (!sockAddr.getAddressIPv6().isNull()) {
                bool ipv4Test;
                Q_ASSERT(sockAddr.getAddressIPv6().protocol() == QAbstractSocket::IPv6Protocol);
                sockAddr.getAddressIPv6().toIPv4Address(&ipv4Test);
                Q_ASSERT(!ipv4Test);
                union {
                    Q_IPV6ADDR ip;
                    // IPv6 is 128 bit long
                    uint64_t value[2];
                } ipConversion;
                ipConversion.ip = sockAddr.getAddressIPv6().toIPv6Address();
                hashResult ^= hash<uint64_t>()(ipConversion.value[0]) ^ hash<uint64_t>()(ipConversion.value[1]);
            }
            return hashResult ^ hash<uint16_t>()((uint16_t) sockAddr.getPort());
        }
    };
}

Q_DECLARE_METATYPE(SockAddr);

#endif // hifi_SockAddr_h
