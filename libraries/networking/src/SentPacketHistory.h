//
//  SentPacketHistory.h
//  libraries/networking/src
//
//  Created by Yixin Wang on 6/5/2014
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_SentPacketHistory_h
#define hifi_SentPacketHistory_h

#include <stdint.h>

#include <QtCore/QByteArray>
#include <QtCore/QReadWriteLock>

#include "NLPacket.h"
#include "NLPacketList.h"
#include "RingBufferHistory.h"
#include "SequenceNumberStats.h"

class NLPacket;

class SentPacketHistory {

public:
    SentPacketHistory(int size = MAX_REASONABLE_SEQUENCE_GAP);

    void untrackedPacketSent(uint16_t sequenceNumber);

    void packetSent(uint16_t sequenceNumber, const NLPacket& packet);
    void packetListSent(uint16_t sequenceNumber, const NLPacketList& packetList);
    const NLPacket* getPacket(uint16_t sequenceNumber) const;

private:
    mutable QReadWriteLock _packetsLock { QReadWriteLock::Recursive };
    RingBufferHistory<std::unique_ptr<NLPacket>> _sentPackets;    // circular buffer

    uint16_t _newestSequenceNumber;
};

#endif
