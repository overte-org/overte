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

enum
{
    IPV4_HEADER_SIZE = 20,
    IPV6_HEADER_SIZE = 40,
    UDP_HEADER_SIZE = 8,
};

Packet::Packet(ByteSlice networkPacket) {
    if (networkPacket.length() >= 16) {
        quint32 sequence = qFromBigEndian<quint32>(&networkPacket[0]);
        _sequence = PacketID(sequence);
        quint32 additionalInfo = qFromBigEndian<quint32>(&networkPacket[4]);
        _additionalInfo = SequenceNumber(additionalInfo);
        _timestamp = qFromBigEndian<quint32>(&networkPacket[8]);
        _socketID = qFromBigEndian<quint32>(&networkPacket[12]);
        _contents = networkPacket.substring(16);
        if ((sequence & 0x8000) == 0) {
            // this is a data packet
            _type = PacketType::Data;
        } else {
            _type = static_cast<PacketType>((sequence & 0x7F00) >> 16);
        }
    }
}

uint Packet::ipHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol) {
    switch (protocol) {
        case QAbstractSocket::IPv4Protocol:
            return IPV4_HEADER_SIZE + UDP_HEADER_SIZE;
        case QAbstractSocket::IPv6Protocol:
            return IPV6_HEADER_SIZE + UDP_HEADER_SIZE;
        default:
            return UDP_HEADER_SIZE;
    }
}

uint Packet::packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol) {
    return ipHeaderSize(protocol) + 16;
}

ByteSlice Packet::toNetworkPacket() const {
    quint32 sequence;
    if (_type < PacketType::Data) {
        sequence = 0x80000000 | (static_cast<quint32>(_type) << 16);
    } else {
        sequence = static_cast<quint32>(_sequence);
    }

    ByteSlice packetData;
    quint8* buffer = reinterpret_cast<quint8*>(packetData.create(_contents.length() + 16));
    *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(sequence);
    *reinterpret_cast<quint32*>(&buffer[4]) = qToBigEndian<quint32>(static_cast<quint32>(_additionalInfo));
    *reinterpret_cast<quint32*>(&buffer[8]) = qToBigEndian<quint32>(_timestamp);
    *reinterpret_cast<quint32*>(&buffer[12]) = qToBigEndian<quint32>(_socketID);
    if (_contents.length() != 0) {
        memcpy(&buffer[16], &_contents[0], _contents.length());
    }

    return packetData;
}

HandshakePacket::HandshakePacket(const Packet& src, QAbstractSocket::NetworkLayerProtocol protocol) :
    _timestamp(src._timestamp), _socketID(src._socketID) {
    assert(src._type == PacketType::Handshake && src._contents.length() >= 48);
    if (src._contents.length() >= 48) {
        _udtVer = qFromBigEndian<quint32>(&src._contents[0]);
        _sockType = static_cast <SocketType>(qFromBigEndian<quint32>(&src._contents[4]));
        _initPktSeq = PacketID(qFromBigEndian<quint32>(&src._contents[8]));
        _maxPktSize = qFromBigEndian<quint32>(&src._contents[12]);
        _maxFlowWinSize = qFromBigEndian<quint32>(&src._contents[16]);
        _reqType = static_cast<RequestType>(qFromBigEndian<quint32>(&src._contents[20]));
        _farSocketID = qFromBigEndian<quint32>(&src._contents[24]);
        _synCookie = qFromBigEndian<quint32>(&src._contents[28]);
        _extra = src._contents.substring(48);

        switch (protocol) {
            case QAbstractSocket::IPv4Protocol:
                _sockAddr.setAddress(*reinterpret_cast<const quint32*>(&src._contents[32]));
                break;
            case QAbstractSocket::IPv6Protocol:
                _sockAddr.setAddress(&src._contents[32]);
                break;
        }
    }
}

uint HandshakePacket::packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol) {
    return Packet::packetHeaderSize(protocol) + 48;
}

Packet HandshakePacket::toPacket() const {
    ByteSlice packetData;
    quint8* buffer = reinterpret_cast<quint8*>(packetData.create(_extra.length() + 48));

    *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(_udtVer);
    *reinterpret_cast<quint32*>(&buffer[4]) = qToBigEndian<quint32>(static_cast<quint32>(_sockType));
    *reinterpret_cast<quint32*>(&buffer[8]) = qToBigEndian<quint32>(static_cast<quint32>(_initPktSeq));
    *reinterpret_cast<quint32*>(&buffer[12]) = qToBigEndian<quint32>(_maxPktSize);
    *reinterpret_cast<quint32*>(&buffer[16]) = qToBigEndian<quint32>(_maxFlowWinSize);
    *reinterpret_cast<quint32*>(&buffer[20]) = qToBigEndian<quint32>(static_cast<quint32>(_reqType));
    *reinterpret_cast<quint32*>(&buffer[24]) = qToBigEndian<quint32>(_farSocketID);
    *reinterpret_cast<quint32*>(&buffer[28]) = qToBigEndian<quint32>(_synCookie);

    switch (_sockAddr.protocol()) {
        case QAbstractSocket::IPv4Protocol: {
            bool ok; // ignored anyhow, hopefully matching the protocol is enough
            *reinterpret_cast<quint32*>(&buffer[32]) = _sockAddr.toIPv4Address(&ok);
            break;
        }
        case QAbstractSocket::IPv6Protocol: {
            Q_IPV6ADDR ip6Addr = _sockAddr.toIPv6Address();
            memcpy(&buffer[32], ip6Addr.c, sizeof(ip6Addr.c));
            break;
        }
    }

    if (_extra.length() != 0) {
        memcpy(&buffer[48], &_extra[0], _extra.length());
    }

    Packet packet;
    packet._type = PacketType::Handshake;
    packet._timestamp = _timestamp;
    packet._socketID = _socketID;
    packet._contents = packetData;
    return packet;
}

MessageDropRequestPacket::MessageDropRequestPacket(const Packet& src) :
    _timestamp(src._timestamp), _socketID(src._socketID), _messageID(src._additionalInfo) {

    assert(src._type == PacketType::MsgDropReq && src._contents.length() >= 8);
    if (src._contents.length() >= 8) {
        quint32 firstPacketID = qFromBigEndian<quint32>(&src._contents[0]);
        _firstPacketID = PacketID(firstPacketID);
        quint32 lastPacketID = qFromBigEndian<quint32>(&src._contents[4]);
        _lastPacketID = PacketID(lastPacketID);
        _extra = src._contents.substring(8);
    }
}

uint MessageDropRequestPacket::packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol) {
    return Packet::packetHeaderSize(protocol) + 8;
}

Packet MessageDropRequestPacket::toPacket() const {
    ByteSlice packetData;
    quint8* buffer = reinterpret_cast<quint8*>(packetData.create(_extra.length() + 8));

    *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(static_cast<quint32>(_firstPacketID));
    *reinterpret_cast<quint32*>(&buffer[4]) = qToBigEndian<quint32>(static_cast<quint32>(_lastPacketID));

    if (_extra.length() != 0) {
        memcpy(&buffer[48], &_extra[0], _extra.length());
    }

    Packet packet;
    packet._type = PacketType::MsgDropReq;
    packet._additionalInfo = _messageID;
    packet._timestamp = _timestamp;
    packet._socketID = _socketID;
    packet._contents = packetData;
    return packet;
}
