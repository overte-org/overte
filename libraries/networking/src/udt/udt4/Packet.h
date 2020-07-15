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
#include "PacketID.h"
#include <QtNetwork/QHostAddress>

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

// SocketType describes the kind of socket this is (i.e. streaming vs message)
enum class SocketType : quint16
{
    Unknown = 0,
    STREAM = 1, // reliable streaming protocol (e.g. TCP)
    DGRAM = 2,  // partially-reliable messaging protocol
};

class Packet {
public:
    inline Packet() {}
    Packet(ByteSlice networkPacket);
    ByteSlice toNetworkPacket() const;

    static uint ipHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);
    static uint packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);

public:
    PacketType _type{ PacketType::Invalid };
    PacketID _sequence;
    quint32 _additionalInfo{ 0 };
    quint32 _timestamp{ 0 };
    quint32 _socketID{ 0 };
    ByteSlice _contents;
};

class HandshakePacket {
public:
    // HandshakeReqType describes the type of handshake packet
    enum class RequestType : qint32
    {
        Request = 1,     // an attempt to establish a new connection
        Rendezvous = 0,  // an attempt to establish a new connection using mutual rendezvous packets
        Response = -1,   // a response to a handshake request
        Response2 = -2,  // an acknowledgement that a HsResponse was received
        Refused = 1002,  // notifies the peer of a connection refusal
    };

public:
    inline HandshakePacket() {}
    HandshakePacket(const Packet& src, QAbstractSocket::NetworkLayerProtocol protocol);
    Packet toPacket() const;

    static uint packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);

public:
    quint32 _timestamp{ 0 };
    quint32 _socketID{ 0 };
    quint32 _udtVer{ 0 };           // UDT version
    SocketType _sockType{ SocketType::Unknown };  // Socket Type (1 = STREAM or 2 = DGRAM)
    PacketID _initPktSeq;           // initial packet sequence number
    quint32 _maxPktSize{ 0 };       // maximum packet size (including UDP/IP headers)
    quint32 _maxFlowWinSize{ 0 };   // maximum flow window size
    RequestType _reqType{ RequestType::Refused };  // connection type (regular(1), rendezvous(0), -1/-2 response)
    quint32 _farSocketID{ 0 };      // socket ID
    quint32 _synCookie{ 0 };        // SYN cookie
	QHostAddress _sockAddr;         // the IP address of the UDP socket to which this packet is being sent
    ByteSlice _extra;
};

}  // namespace udt4

#endif /* hifi_udt4_Packet_h */