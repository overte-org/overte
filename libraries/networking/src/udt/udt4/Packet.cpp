//
//  Packet.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Packet.h"

#include <QtCore/QtEndian>

using namespace udt4;

Packet::Packet(ByteSlice networkPacket) {
    if (networkPacket.length() >= 16) {
        _sequence = qFromBigEndian<quint32>(&networkPacket[0]);
        _additionalInfo = qFromBigEndian<quint32>(&networkPacket[4]);
        _timestamp = qFromBigEndian<quint32>(&networkPacket[8]);
        _socketID = qFromBigEndian<quint32>(&networkPacket[12]);
        _contents = networkPacket.substring(16);
        if ((_sequence & 0x8000) == 0) {
            // this is a data packet
            _type = PacketType::Data;
        } else {
            _type = static_cast<PacketType>((_sequence & 0x7F00) >> 16);
        }
    }
}
