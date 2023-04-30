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
#include <chrono>
#include <array>

#include <QtCore/QByteArray>
#include <QtNetwork/QHostAddress>

#include "../../ByteSlice.h"
#include "WrappedSequence.h"

namespace udt4 {
Q_NAMESPACE

#ifndef UDT_OBFUSCATION_DISABLED
  using PacketID = WrappedSequence<29>;
#else
  using PacketID = WrappedSequence<31>;
#endif
static_assert(sizeof(PacketID) == sizeof(quint32), "PacketID invalid size");

using SequenceNumber = WrappedSequence<29>;  // either an ACK number or a Message number
static_assert(sizeof(SequenceNumber) == sizeof(quint32), "SequenceNumber invalid size");

using MessageNumber = SequenceNumber;
using ACKSequence = SequenceNumber;


// PacketType describes the type of UDP packet we're dealing with
enum class PacketType : quint16
{
    // Control packet types
    Handshake = 0x0,   // see HandshakePacket
    Keepalive = 0x1,   // no additional data
    Ack = 0x2,         // see ACKPacket
    Nak = 0x3,         // see NAKPacket
    Congestion = 0x4,  // removed in ver4 (no additional data)
    Shutdown = 0x5,    // no additional data
    Ack2 = 0x6,        // additional info: ACK sequence number
    MsgDropReq = 0x7,  // see MessageDropRequestPacket
    SpecialErr = 0x8,  // undocumented but reference implementation seems to use it (additional info: error code)
    UserDefPkt = 0x7FFF,
    Data = 0x8000,     // not found in any control packet, but used to identify data packets
    Stun = 0x8001,     // this is a RFC5389 STUN packet
    Invalid = 0x8001,
};
Q_ENUM_NS(PacketType)

// SocketType describes the kind of socket this is (i.e. streaming vs message)
// the reference implementation differs from the RFC here, I'm using the definition from the reference
enum class SocketType : quint16
{
    Unknown = 0,
    STREAM = 1,  // reliable streaming protocol (e.g. TCP)
    DGRAM = 2,   // partially-reliable messaging protocol
};

#ifndef UDT_OBFUSCATION_DISABLED
// This is hidden behind a #define in case it becomes "controversial", this appears to attempt to bypass
// packet-loss caused by pattern-matching firewalls by obfuscating the contents if we have a packet that
// just isn't making its way through

static const std::array<uint64_t, 4> OBFUSCATION_KEYS{ {
    0x0,
    0x6362726973736574,
    0x7362697261726461,
    0x72687566666d616e,
} };

enum class PacketObfuscationKey
{
    None = 0x0,  // 00
    K1 = 0x1,    // 01
    K2 = 0x2,    // 10
    K3 = 0x3,    // 11
};
enum
{
    OBFUSCATION_THRESHOLD = 2,
};
#endif

class Packet {
public:
    inline Packet() {}
    Packet(ByteSlice networkPacket);
    ByteSlice toNetworkPacket() const;

    static uint ipHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);
    static uint packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);
    inline quint8 userDefinedPacketType() const { return _sequence & 0xFF; }
    void setUserDefinedPacketType(quint8 type);
    inline QByteArray toByteArray() const {
        return QByteArray(reinterpret_cast<const char*>(_contents.constData()), static_cast<int>(_contents.length()));
    }

public:
    PacketType _type{ PacketType::Invalid };
    quint32 _sequence{ 0 };
    quint32 _additionalInfo{ 0 };
    std::chrono::microseconds _timestamp{ 0 };
    quint32 _socketID{ 0 };
    ByteSlice _contents;
};

class DataPacket {
public:
    enum class MessagePosition
    {
        Only = 3,    // this is the only packet in the message
        First = 2,   // this is the first packet in a multi-packet message
        Last = 1,    // this is the last packet in a multi-packet message
        Middle = 0,  // this is a middle packet in a multi-packet message
    };

public:
    inline DataPacket() {}
    DataPacket(const Packet& src);
    Packet toPacket() const;

    static uint packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);

public:
    PacketID _packetID;
    MessagePosition _messagePosition{ MessagePosition::Only };
    bool _isOrdered{ false };
    MessageNumber _messageNumber;
    std::chrono::microseconds _timestamp{ 0 };
    quint32 _socketID{ 0 };
    ByteSlice _contents;
#ifndef UDT_OBFUSCATION_DISABLED
    PacketObfuscationKey _obfuscationKey{ PacketObfuscationKey::None };
#endif
};

class HandshakePacket {
public:
    // HandshakeReqType describes the type of handshake packet
    enum class RequestType : qint32
    {
        Request = 1,     // an attempt to establish a new connection
        Rendezvous = 0,  // an attempt to establish a new connection using mutual rendezvous packets
        Response = -1,   // a response to a handshake request
        Response2 = -2,  // an acknowledgement that a HsResponse was received (only used for Rendezvous connections)
        Refused = 1002,  // notifies the peer of a connection refusal
    };

public:
    inline HandshakePacket() {}
    HandshakePacket(const Packet& src, QAbstractSocket::NetworkLayerProtocol protocol);
    Packet toPacket() const;

    static uint packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);

public:
    std::chrono::microseconds _timestamp{ 0 };
    quint32 _socketID{ 0 };
    quint32 _udtVer{ 0 };                          // UDT version
    SocketType _sockType{ SocketType::Unknown };   // Socket Type (1 = STREAM or 2 = DGRAM)
    PacketID _initPktSeq;                          // initial packet sequence number
    quint32 _maxPktSize{ 0 };                      // maximum packet size (including UDP/IP headers)
    quint32 _maxFlowWinSize{ 0 };                  // maximum flow window size
    RequestType _reqType{ RequestType::Refused };  // connection type (regular(1), rendezvous(0), -1/-2 response)
    quint32 _farSocketID{ 0 };                     // socket ID
    quint32 _synCookie{ 0 };                       // SYN cookie
    QHostAddress _sockAddr;                        // the IP address of the UDP socket to which this packet is being sent
    ByteSlice _extra;
};

class ACKPacket {
public:
    enum class AckType
    {
        Light,
        Normal,
        Full
    };

public:
    inline ACKPacket() {}
    ACKPacket(const Packet& src);
    Packet toPacket() const;

    uint packetSize(QAbstractSocket::NetworkLayerProtocol protocol) const;

public:
    std::chrono::microseconds _timestamp{ 0 };
    quint32 _socketID{ 0 };
    ACKSequence _ackSequence;
    PacketID _lastPacketReceived;
    AckType _ackType{ AckType::Normal };
    std::chrono::microseconds _rtt{ 0 };
    std::chrono::microseconds _rttVariance{ 0 };
    quint32 _availBufferSize{ 0 };        // (in bytes)
    quint32 _packetReceiveRate{ 0 };      // (in packets/sec)
    quint32 _estimatedLinkCapacity{ 0 };  // (in packets/sec)
};

class NAKPacket {
public:
    inline NAKPacket() {}
    NAKPacket(const Packet& src);
    Packet toPacket() const;

    uint packetSize(QAbstractSocket::NetworkLayerProtocol protocol) const;

public:
    typedef QList<quint32> IntegerList;
    std::chrono::microseconds _timestamp{ 0 };
    quint32 _socketID{ 0 };
    IntegerList _lossData;
};

class MessageDropRequestPacket {
public:
    inline MessageDropRequestPacket() {}
    MessageDropRequestPacket(const Packet& src);
    Packet toPacket() const;

    static uint packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol);

public:
    MessageNumber _messageID;
    std::chrono::microseconds _timestamp{ 0 };
    quint32 _socketID{ 0 };
    PacketID _firstPacketID;
    PacketID _lastPacketID;
    ByteSlice _extra;
};

}  // namespace udt4

#endif /* hifi_udt4_Packet_h */