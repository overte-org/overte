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
#include <QtCore/QMutexLocker>

namespace udt4 {

inline void UdtSocket::abort() {
    setState(UdtSocketState::Init);
}

inline bool UdtSocket::isInDatagramMode() const {
    return _isDatagram;
}

inline const QHostAddress& UdtSocket::peerAddress() const {
    return _remoteAddr;
}

inline quint16 UdtSocket::peerPort() const {
    return _remotePort;
}

inline UdtSocketState UdtSocket::state() const {
    QMutexLocker locker(&_sockStateProtect);
    return _sockState;
}

inline QString UdtSocket::errorString() const {
    return _errorString;
}

inline bool UdtSocket::isValid() const {
    return state() == UdtSocketState::Connected;
}

inline void UdtSocket::setLocalSocketID(quint32 socketID) {
    _socketID = socketID;
}

// search through the specified map for the first entry >= key but < limit
template <class T>
inline typename std::map<PacketID, T, WrappedSequenceLess<PacketID>>::iterator findFirstEntry(std::map<PacketID, T, WrappedSequenceLess<PacketID>>& map, const PacketID& key, const PacketID& limit) {
    std::map<PacketID, T, WrappedSequenceLess<PacketID>>::iterator lookup = map.lower_bound(key);
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
inline typename std::map<PacketID, T, WrappedSequenceLess<PacketID>>::const_iterator findFirstEntry(const std::map<PacketID, T, WrappedSequenceLess<PacketID>>& map, const PacketID& key, const PacketID& limit) {
    std::map<PacketID, T, WrappedSequenceLess<PacketID>>::const_iterator lookup = map.lower_bound(key);
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