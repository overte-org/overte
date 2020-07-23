//
//  UdtSocket.inl
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-28.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtSocket_inl
#define hifi_udt4_UdtSocket_inl
#include "UdtSocket.h"

namespace udt4 {

void UdtSocket::abort() {
    setState(UdtSocketState::Init);
}

bool UdtSocket::isInDatagramMode() const {
    return _isDatagram;
}

const QHostAddress& UdtSocket::peerAddress() const {
    return _remoteAddr;
}

quint16 UdtSocket::peerPort() const {
    return _remotePort;
}

UdtSocketState UdtSocket::state() const {
    QMutexLocker locker(&_sockStateProtect);
    return _sockState;
}

QString UdtSocket::errorString() const {
    return _errorString;
}

qint64 UdtSocket::writeDatagram(const QByteArray& datagram) {
    return writeDatagram(datagram.constData(), datagram.size());
}

qint64 UdtSocket::writeDatagram(const ByteSlice& datagram) {
    return writeDatagram(reinterpret_cast<const char*>(datagram.constData()), datagram.length());
}

bool UdtSocket::isValid() const {
    return state() == UdtSocketState::Connected;
}

void UdtSocket::setLocalSocketID(quint32 socketID) {
    _socketID = socketID;
}

// search through the specified map for the first entry >= key but < limit
template <class T>
typename std::map<PacketID, T>::iterator findFirstEntry(std::map<PacketID, T>& map, const PacketID& key, const PacketID& limit) {
    std::map<PacketID, T>::iterator lookup = map.lower_bound(key);
    if (key < limit) {
        if (lookup == map.end() || lookup->first >= limit) {
            return map.end();
        } else {
            return lookup;
        }
    }
    if (lookup != map.end()) {
        return lookup;
    }
    lookup = map.lower_bound(PacketID(0UL));
    if (lookup == map.end() || lookup->first >= limit) {
        return map.end();
    } else {
        return lookup;
    }
}

// search through 
template <class T>
typename std::map<PacketID, T>::const_iterator findFirstEntry(const std::map<PacketID, T>& map, const PacketID& key, const PacketID& limit) {
    std::map<PacketID, T>::const_iterator lookup = map.lower_bound(key);
    if (key < limit) {
        if (lookup == map.end() || lookup->first >= limit) {
            return map.end();
        } else {
            return lookup;
        }
    }
    if (lookup != map.end()) {
        return lookup;
    }
    lookup = map.lower_bound(PacketID(0UL));
    if (lookup == map.end() || lookup->first >= limit) {
        return map.end();
    } else {
        return lookup;
    }
}

}  // namespace udt4
#endif /* hifi_udt4_UdtSocket_h */