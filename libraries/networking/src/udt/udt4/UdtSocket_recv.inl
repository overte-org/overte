//
//  UdtSocket_recv.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-07-19.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtSocket_recv_inl
#define hifi_udt4_UdtSocket_recv_inl
#include "UdtSocket_recv.h"

namespace udt4 {

// search through the specified map for the first entry >= key but < limit
template <class T>
typename std::map<PacketID, T>::iterator findFirst(std::map<PacketID, T>& map, const PacketID& key, const PacketID& limit) {
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
typename std::map<PacketID, T>::const_iterator findFirst(const std::map<PacketID, T>& map, const PacketID& key, const PacketID& limit) {
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

UdtSocket_receive::ReceivedPacket::ReceivedPacket(const Packet& p, const QElapsedTimer& t) : udtPacket(p), timeReceived(t) {
}

}  // namespace udt4

#endif /* hifi_udt4_UdtSocket_recv_h */