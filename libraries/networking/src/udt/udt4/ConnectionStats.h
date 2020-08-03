//
//  ConnectionStats.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-08-02.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_ConnectionStats_h
#define hifi_udt4_ConnectionStats_h

#include <atomic>
#include <chrono>
#include <QtCore/QElapsedTimer>
#include <QtCore/QSharedPointer>

namespace udt4 {

struct ConnectionStatsAtomic {
    QElapsedTimer startTime;

    // performance metrics
    std::atomic<quint32> sentPackets{ 0 };                  // number of sent packets, including retransmissions (from base class)
    std::atomic<quint64> sentWireBytes{ 0 };                // number of bytes sent, including all packet headers (from base class)
    std::atomic<quint32> receivedPackets{ 0 };              // number of received packets (from base class)
    std::atomic<quint64> receivedWireBytes{ 0 };            // number of bytes received, including all packet headers (from base class)

    std::atomic<quint32> sentACKpackets{ 0 };               // number of sent ACK packets (from receiver side)
    std::atomic<quint32> receivedACKpackets{ 0 };           // number of received ACK packets (from sender side)
    std::atomic<quint32> sentNAKpackets{ 0 };               // number of sent NAK packets (from receiver side)
    std::atomic<quint32> receivedNAKpackets{ 0 };           // number of received NAK packets (from sender side)

    std::atomic<quint32> sentDataPackets{ 0 };              // number of data packets sent (from sender side)
    std::atomic<quint32> receivedDataPackets{ 0 };          // number of data packets received (from receiver side)
    std::atomic<quint32> lostSentPackets{ 0 };              // number of lost data packets (from sender side)
    std::atomic<quint32> lostReceivedPackets{ 0 };          // number of lost data packets (from receiver side)
    std::atomic<quint32> duplicateReceivedPackets{ 0 };     // number of duplicate data packets (from receiver side)
    std::atomic<quint32> retransmittedPackets{ 0 };         // number of retransmitted data packets (from sender side)

    std::atomic<quint64> sentDataBytes{ 0 };                // number of data bytes sent (from sender side)
    std::atomic<quint64> receivedDataBytes{ 0 };            // number of data bytes received (from receiver side)
    std::atomic<quint64> duplicateDataBytes{ 0 };           // number of duplicate data bytes (from receiver side)
    std::atomic<quint64> retransmittedDataBytes{ 0 };       // number of retransmitted data bytes (from sender side)
    
    std::atomic<std::chrono::nanoseconds> sendingCpuTime;    // busy sending cpu time (i.e., idle time exclusive)
    std::atomic<std::chrono::nanoseconds> receivingCpuTime;  // busy receiving cpu time (i.e., idle time exclusive)

    // instant measurements
//    std::atomic<std::chrono::microseconds> packetSendingPeriod;  // packet sending period (sending side)
//    std::atomic<unsigned> packetFlowWindow{ 0 };        // flow window size, in number of packets (sending side, from remote)
//    std::atomic<unsigned> packetCongestionWindow{ 0 };  // congestion window size, in number of packets (congestion control)
//    std::atomic<unsigned> packetFlightSize{ 0 };        // number of packets on flight (sending side)
//    std::atomic<std::chrono::microseconds> rtt;         // RTT
//    std::atomic<unsigned> MbpsBandwidth{ 0 };           // estimated bandwidth, in Mb/s
};
typedef QSharedPointer<ConnectionStatsAtomic> ConnectionStatsAtomicPointer;

struct ConnectionStats {
    QElapsedTimer startTime;
    QElapsedTimer endTime;

    // performance metrics
    quint32 sentPackets{ 0 };                  // number of sent packets, including retransmissions
    quint64 sentWireBytes{ 0 };                // number of bytes sent, including all packet headers
    quint32 receivedPackets{ 0 };              // number of received packets
    quint64 receivedWireBytes{ 0 };            // number of bytes received, including all packet headers

    quint32 sentACKpackets{ 0 };               // number of sent ACK packets
    quint32 receivedACKpackets{ 0 };           // number of received ACK packets
    quint32 sentNAKpackets{ 0 };               // number of sent NAK packets
    quint32 receivedNAKpackets{ 0 };           // number of received NAK packets

    quint32 sentDataPackets{ 0 };              // number of data packets sent
    quint32 receivedDataPackets{ 0 };          // number of data packets received
    quint32 lostSentPackets{ 0 };              // number of lost data packets (sender side)
    quint32 lostReceivedPackets{ 0 };          // number of lost data packets (receiver side)
    quint32 duplicateReceivedPackets{ 0 };     // number of duplicate data packets (receiver side)
    quint32 retransmittedPackets{ 0 };         // number of retransmitted data packets

    quint64 sentDataBytes{ 0 };                // number of data bytes sent
    quint64 receivedDataBytes{ 0 };            // number of data bytes received
    quint64 duplicateDataBytes{ 0 };           // number of duplicate data bytes (receiver side)
    quint64 retransmittedDataBytes{ 0 };       // number of retransmitted data bytes

    std::chrono::nanoseconds sendingCpuTime;    // busy sending cpu time (i.e., idle time exclusive)
    std::chrono::nanoseconds receivingCpuTime;  // busy receiving cpu time (i.e., idle time exclusive)

    // instant measurements
//    std::chrono::microseconds packetSendingPeriod;  // packet sending period (sending side)
//    unsigned packetFlowWindow{ 0 };        // flow window size, in number of packets (sending side, from remote)
//    unsigned packetCongestionWindow{ 0 };  // congestion window size, in number of packets (congestion control)
//    unsigned packetFlightSize{ 0 };        // number of packets on flight (sending side)
    std::chrono::microseconds rtt;
    std::chrono::microseconds rttVariance;
    unsigned receiveRate;
    unsigned estimatedBandwith;
    /*
        // the following stats are trailing averages in the result, not totals
        int sendRate { 0 };
        int congestionWindowSize { 0 };
        int packetSendPeriod { 0 };

    virtual unsigned getMaxFlowWinSize() const;
    virtual void setCongestionWindow(unsigned pkt);
    virtual void setPacketSendPeriod(std::chrono::microseconds snd);
    virtual void setACKperiod(std::chrono::microseconds ack);
    virtual void setACKinterval(unsigned ack);
    virtual void setRTOperiod(std::chrono::microseconds rto);
*/
    explicit ConnectionStats(const ConnectionStatsAtomic& src, QElapsedTimer endTime = QElapsedTimer());
    ConnectionStats operator-(const ConnectionStats& rhs) const;
};

}  // namespace udt4

#endif /* hifi_udt4_ConnectionStats_h */