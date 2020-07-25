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

inline UdtSocket_receive::ReceivedPacket::ReceivedPacket(const Packet& p, const QElapsedTimer& t) : udtPacket(p), timeReceived(t) {
}

}  // namespace udt4

#endif /* hifi_udt4_UdtSocket_recv_h */