//
//  ConnectionStats.cpp
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-08-02.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "ConnectionStats.h"

using namespace udt4;

ConnectionStats::ConnectionStats(const ConnectionStatsAtomic& src, QElapsedTimer end) :
    startTime(src.startTime), endTime(end), sentPackets(src.sentPackets), sentWireBytes(src.sentWireBytes),
    receivedPackets(src.receivedPackets), receivedWireBytes(src.receivedWireBytes), sentACKpackets(src.sentACKpackets),
    receivedACKpackets(src.receivedACKpackets), sentNAKpackets(src.sentNAKpackets), receivedNAKpackets(src.receivedNAKpackets),
    sentDataPackets(src.sentDataPackets), receivedDataPackets(src.receivedDataPackets), lostSentPackets(src.lostSentPackets),
    lostReceivedPackets(src.lostReceivedPackets), duplicateReceivedPackets(src.duplicateReceivedPackets),
    retransmittedPackets(src.retransmittedPackets), sentDataBytes(src.sentDataBytes), receivedDataBytes(src.receivedDataBytes),
    duplicateDataBytes(src.duplicateDataBytes), retransmittedDataBytes(src.retransmittedDataBytes),
    sendingCpuTime(src.sendingCpuTime), receivingCpuTime(src.receivingCpuTime) {
    if (!endTime.isValid()) {
        endTime.start();
    }
}

ConnectionStats ConnectionStats::operator-(const ConnectionStats& rhs) const {
    ConnectionStats result(*this);
    result.startTime = rhs.endTime;
    result.sentPackets -= rhs.sentPackets;
    result.sentWireBytes -= rhs.sentWireBytes;
    result.receivedPackets -= rhs.receivedPackets;
    result.receivedWireBytes -= rhs.receivedWireBytes;
    result.sentACKpackets -= rhs.sentACKpackets;
    result.receivedACKpackets -= rhs.receivedACKpackets;
    result.sentNAKpackets -= rhs.sentNAKpackets;
    result.receivedNAKpackets -= rhs.receivedNAKpackets;
    result.sentDataPackets -= rhs.sentDataPackets;
    result.receivedDataPackets -= rhs.receivedDataPackets;
    result.lostSentPackets -= rhs.lostSentPackets;
    result.lostReceivedPackets -= rhs.lostReceivedPackets;
    result.duplicateReceivedPackets -= rhs.duplicateReceivedPackets;
    result.retransmittedPackets -= rhs.retransmittedPackets;
    result.sentDataBytes -= rhs.sentDataBytes;
    result.receivedDataBytes -= rhs.receivedDataBytes;
    result.duplicateDataBytes -= rhs.duplicateDataBytes;
    result.retransmittedDataBytes -= rhs.retransmittedDataBytes;
    result.sendingCpuTime -= rhs.sendingCpuTime;
    result.receivingCpuTime -= rhs.receivingCpuTime;
    return result;
}
