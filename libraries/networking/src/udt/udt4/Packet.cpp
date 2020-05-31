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

ByteSlice Packet::toNetworkPacket() const {
    quint16 sequence;
    if (_type < PacketType::Data) {
        sequence = 0x8000 | (static_cast<quint16>(_type) << 16);
    } else {
        sequence = _sequence;
    }

    ByteSlice packetData;
    quint8* buffer = reinterpret_cast<quint8*>(packetData.create(_contents.length() + 16));
    *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(sequence);
    *reinterpret_cast<quint32*>(&buffer[4]) = qToBigEndian<quint32>(_additionalInfo);
    *reinterpret_cast<quint32*>(&buffer[8]) = qToBigEndian<quint32>(_timestamp);
    *reinterpret_cast<quint32*>(&buffer[12]) = qToBigEndian<quint32>(_socketID);
    if (_contents.length() != 0) {
        memcpy(&buffer[16], &_contents[0], _contents.length());
    }

    return packetData;
}

HandshakePacket::HandshakePacket(const Packet& src, QAbstractSocket::NetworkLayerProtocol protocol) :
    _timestamp(src._timestamp), _socketID(src._socketID) {
    if (src._contents.length() >= 48) {
        _udtVer = qFromBigEndian<quint32>(&src._contents[0]);
        _sockType = static_cast <SocketType>(qFromBigEndian<quint32>(&src._contents[4]));
        _initPktSeq = qFromBigEndian<quint32>(&src._contents[8]);
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

Packet HandshakePacket::toPacket() const {
    ByteSlice packetData;
    quint8* buffer = reinterpret_cast<quint8*>(packetData.create(_extra.length() + 48));

    *reinterpret_cast<quint32*>(&buffer[0]) = qToBigEndian<quint32>(_udtVer);
    *reinterpret_cast<quint32*>(&buffer[4]) = qToBigEndian<quint32>(static_cast<quint32>(_sockType));
    *reinterpret_cast<quint32*>(&buffer[8]) = qToBigEndian<quint32>(_initPktSeq);
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
