//
//  NLPacketList.h
//  libraries/networking/src
//
//  Created by Stephen Birarda on 07/06/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_NLPacketList_h
#define hifi_NLPacketList_h

#include <QtCore/QSharedPointer>
#include "udt/PacketList.h"

#include "NLPacket.h"

class NLPacketList : public udt::PacketList {
public:
    static std::unique_ptr<NLPacketList> create(PacketType packetType, QByteArray extendedHeader = QByteArray(),
                                                bool isReliable = false, bool isOrdered = false);
    
    PacketVersion getVersion() const { return _packetVersion; }
    NLPacket::LocalID getSourceID() const { return _sourceID; }

    qint64 getMaxSegmentSize() const override { return NLPacket::maxPayloadSize(_packetType, _isOrdered); }
    
private:
    Q_DISABLE_COPY(NLPacketList)
    NLPacketList(PacketType packetType, QByteArray extendedHeader = QByteArray(), bool isReliable = false,
                 bool isOrdered = false);
    NLPacketList(udt::PacketList&& packetList);

    virtual std::unique_ptr<udt::Packet> createPacket() override;

    PacketVersion _packetVersion;
    NLPacket::LocalID _sourceID;
};

Q_DECLARE_METATYPE(QSharedPointer<NLPacketList>)

#endif // hifi_PacketList_h
