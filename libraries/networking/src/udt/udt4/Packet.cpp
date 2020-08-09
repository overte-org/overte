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
        _additionalInfo = qFromBigEndian<quint32>(&networkPacket[4]);
        _timestamp = std::chrono::microseconds(qFromBigEndian<quint32>(&networkPacket[8]));
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
    *reinterpret_cast<quint32*>(&buffer[8]) = qToBigEndian<quint32>(_timestamp.count());
    *reinterpret_cast<quint32*>(&buffer[12]) = qToBigEndian<quint32>(_socketID);
    if (_contents.length() != 0) {
        memcpy(&buffer[16], &_contents[0], _contents.length());
    }

    return packetData;
}

DataPacket::DataPacket(const Packet& src) :
    _timestamp(src._timestamp), _socketID(src._socketID), _packetID(src._sequence), _contents(src._contents) {
    assert(src._type == PacketType::Data);
    _messagePosition = static_cast<MessagePosition>((src._additionalInfo & 0xC0000000) >> 30);
    _isOrdered = (src._additionalInfo & 0x20000000) != 0;
    _messageNumber = MessageNumber(src._additionalInfo);
}

uint DataPacket::packetHeaderSize(QAbstractSocket::NetworkLayerProtocol protocol) {
    return Packet::packetHeaderSize(protocol);
}

Packet DataPacket::toPacket() const {
    quint32 additionalInfo = static_cast<quint32>(_messageNumber);
    assert((additionalInfo & 0xE0000000) == 0);
    if (_isOrdered) {
        additionalInfo |= 0x20000000;
    }
    additionalInfo |= (static_cast<quint32>(_messagePosition) & 0x3) << 30;

    Packet packet;
    packet._timestamp = _timestamp;
    packet._socketID = _socketID;
    packet._sequence = _packetID;
    packet._contents = _contents;
    packet._additionalInfo = static_cast<quint32>(additionalInfo);
    return packet;
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

ACKPacket::ACKPacket(const Packet& src) :
    _timestamp(src._timestamp), _socketID(src._socketID), _ackSequence(src._additionalInfo) {
    assert(src._type == PacketType::Ack && src._contents.length() >= 4);
    if (src._contents.length() >= 4) {
        _lastPacketReceived = PacketID(qFromBigEndian<quint32>(&src._contents[0]));
        if (src._contents.length() < 24) {
            _ackType = AckType::Light;
        } else {
            _rtt = static_cast<std::chrono::microseconds>(qFromBigEndian<quint32>(&src._contents[4]));
            _rttVariance = static_cast<std::chrono::microseconds>(qFromBigEndian<quint32>(&src._contents[8]));
            _availBufferSize = qFromBigEndian<quint32>(&src._contents[12]);
            if (src._contents.length() < 16) {
                _ackType = AckType::Normal;
            } else {
                _ackType = AckType::Full;
                _packetReceiveRate = qFromBigEndian<quint32>(&src._contents[16]);
                _estimatedLinkCapacity = qFromBigEndian<quint32>(&src._contents[20]);
            }
        }
    }
}

uint ACKPacket::packetSize(QAbstractSocket::NetworkLayerProtocol protocol) const {
    switch (_ackType) {
    case AckType::Light:
        return Packet::packetHeaderSize(protocol) + 4;
        break;
    case AckType::Normal:
        return Packet::packetHeaderSize(protocol) + 16;
        break;
    case AckType::Full:
        return Packet::packetHeaderSize(protocol) + 24;
        break;
    }
    assert(false);
    return 0;
}

Packet ACKPacket::toPacket() const {
    ByteSlice packetData;
    quint8* buffer;
    
    switch (_ackType) {
    case AckType::Light:
        buffer = reinterpret_cast<quint8*>(packetData.create(4));
        *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(static_cast<quint32>(_lastPacketReceived));
        break;

    case AckType::Normal:
        buffer = reinterpret_cast<quint8*>(packetData.create(16));
        *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(static_cast<quint32>(_lastPacketReceived));
        *reinterpret_cast<quint32*>(&buffer[4]) = qToBigEndian<quint32>(_rtt.count());
        *reinterpret_cast<quint32*>(&buffer[8]) = qToBigEndian<quint32>(_rttVariance.count());
        *reinterpret_cast<quint32*>(&buffer[12]) = qToBigEndian<quint32>(_availBufferSize);
        break;

    case AckType::Full:
        buffer = reinterpret_cast<quint8*>(packetData.create(24));
        *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(static_cast<quint32>(_lastPacketReceived));
        *reinterpret_cast<quint32*>(&buffer[4]) = qToBigEndian<quint32>(_rtt.count());
        *reinterpret_cast<quint32*>(&buffer[8]) = qToBigEndian<quint32>(_rttVariance.count());
        *reinterpret_cast<quint32*>(&buffer[12]) = qToBigEndian<quint32>(_availBufferSize);
        *reinterpret_cast<quint32*>(&buffer[16]) = qToBigEndian<quint32>(_packetReceiveRate);
        *reinterpret_cast<quint32*>(&buffer[20]) = qToBigEndian<quint32>(_estimatedLinkCapacity);
        break;
    }

    Packet packet;
    packet._type = PacketType::Ack;
    packet._timestamp = _timestamp;
    packet._additionalInfo = static_cast<quint32>(_ackSequence);
    packet._socketID = _socketID;
    packet._contents = packetData;
    return packet;
}

NAKPacket::NAKPacket(const Packet& src) : _timestamp(src._timestamp), _socketID(src._socketID) {
    assert(src._type == PacketType::Nak && src._contents.length() >= 4);
    if (src._contents.length() >= 4) {
        unsigned count = static_cast<unsigned>(src._contents.length() / sizeof(quint32));
        for (unsigned idx = 0; idx < count; idx++) {
            _lossData.append(qFromBigEndian<quint32>(&src._contents[idx * sizeof(quint32)]));
        }
    }
}

uint NAKPacket::packetSize(QAbstractSocket::NetworkLayerProtocol protocol) const {
    return Packet::packetHeaderSize(protocol) + _lossData.length() * sizeof(quint32);
}

Packet NAKPacket::toPacket() const {
    ByteSlice packetData;
    quint8* buffer = reinterpret_cast<quint8*>(packetData.create(_lossData.length() * sizeof(quint32)));

    unsigned idx;
    IntegerList::const_iterator trans;
    for (idx = 0, trans = _lossData.begin(); trans != _lossData.end(); trans++, idx++) {
        *reinterpret_cast<quint32*>(&buffer[idx * sizeof(quint32)]) = qToBigEndian<quint32>(static_cast<quint32>(*trans));
    }

    Packet packet;
    packet._type = PacketType::Nak;
    packet._timestamp = _timestamp;
    packet._socketID = _socketID;
    packet._contents = packetData;
    return packet;
}

MessageDropRequestPacket::MessageDropRequestPacket(const Packet& src) :
    _timestamp(src._timestamp), _socketID(src._socketID), _messageID(src._additionalInfo) {
    assert(src._type == PacketType::MsgDropReq && src._contents.length() >= 8);
    if (src._contents.length() >= 8) {
        _firstPacketID = PacketID(qFromBigEndian<quint32>(&src._contents[0]));
        _lastPacketID = PacketID(qFromBigEndian<quint32>(&src._contents[4]));
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
    packet._additionalInfo = static_cast<quint32>(_messageID);
    packet._timestamp = _timestamp;
    packet._socketID = _socketID;
    packet._contents = packetData;
    return packet;
}
