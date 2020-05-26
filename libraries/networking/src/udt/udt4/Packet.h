//
//  Packet.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#ifndef hifi_udt4_Packet_h
#define hifi_udt4_Packet_h

// Structure of packets and functions for writing/reading them
#include "ByteSlice.h"

namespace udt4 {

// PacketType describes the type of UDP packet we're dealing with
enum class PacketType : quint16
{
    // Control packet types
    Handshake = 0x0,
    Keepalive = 0x1,
    Ack = 0x2,
    Nak = 0x3,
    Congestion = 0x4,  // unused in ver4
    Shutdown = 0x5,
    Ack2 = 0x6,
    MsgDropReq = 0x7,
    SpecialErr = 0x8,  // undocumented but reference implementation seems to use it
    UserDefPkt = 0x7FFF,
    Data = 0x8000,  // not found in any control packet, but used to identify data packets
    Invalid = 0x8001,
};

class Packet {
public:
    Packet(ByteSlice networkPacket);

public:
    PacketType _type{ PacketType::Invalid };
    quint32 _sequence{ 0 };
    quint32 _additionalInfo{ 0 };
    quint32 _timestamp{ 0 };
    quint32 _socketID{ 0 };
    ByteSlice _contents;
};

}  // namespace udt4

#endif /* hifi_udt4_Packet_h */