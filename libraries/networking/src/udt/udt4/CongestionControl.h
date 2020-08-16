//
//  CongestionControl.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-06-20.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#ifndef hifi_udt4_CongestionControl_h
#define hifi_udt4_CongestionControl_h

#include <chrono>
#include "Packet.h"
#include "WrappedSequence.h"
#include <QtCore/QElapsedTimer>
#include <QtCore/QList>
#include <QtCore/QSharedPointer>

namespace udt4 {

// Interface provided to UDT congestion control systems
class CongestionControlParms {
public:
    virtual PacketID getSendCurrentPacketID() const = 0;                // returns the most recently sent packet ID
    virtual void setCongestionWindowSize(unsigned) = 0;                 // sets the size of the congestion window (in packets)
    virtual unsigned getCongestionWindowSize() const = 0;               // gets the size of the congestion window (in packets)
    virtual std::chrono::microseconds getPacketSendPeriod() const = 0;  // gets the current delay between sending packets
    virtual void setPacketSendPeriod(std::chrono::microseconds) = 0;    // sets the current delay between sending packets
    virtual unsigned getMaxFlowWindow() const = 0;                      // returns the largest number of unacknowledged packets we can receive (in packets)
    virtual void getReceiveRates(unsigned& recvSpeed, unsigned& bandwidth) const = 0;  // returns the current calculated receive rate and bandwidth (in packets/sec)
    virtual std::chrono::microseconds getRTT() const = 0;               // returns the current calculated roundtrip time between peers
    virtual unsigned getMSS() const = 0;                                // returns the largest packet size we can currently send (in bytes)
    virtual void setACKPeriod(std::chrono::microseconds) = 0;           // sets the time between ACKs sent to the peer
    virtual void setACKInterval(unsigned) = 0;                          // sets the number of packets sent to the peer before sending an ACK (in packets)
    virtual void setRTOPeriod(std::chrono::microseconds) = 0;           // overrides the default EXP timeout calculations waiting for data from the peer
};

// Interface to be implemented by UDT congestion control systems
class CongestionControl {
public:
    virtual void init(CongestionControlParms& parms) = 0;                                     // connection is being setup.
    virtual void close(CongestionControlParms& parms) = 0;                                    // connection is closed.
    virtual void onACK(CongestionControlParms& parms, PacketID packetID) = 0;                 // ACK packet is received
    virtual void onNAK(CongestionControlParms& parms, const QList<PacketID>& packetIDs) = 0;  // loss report is received
    virtual void onTimeout(CongestionControlParms& parms) = 0;                                // a timeout event occurs
    virtual void onPacketSent(CongestionControlParms& parms, const Packet& packet) = 0;       // data is sent
    virtual void onPacketReceived(CongestionControlParms& parms,
                                  const Packet& packet,
                                  const QElapsedTimer& timeReceived) = 0;                     // data is received
    virtual void onCustomMessageReceived(CongestionControlParms& parms,
                                         const Packet& packet,
                                         const QElapsedTimer& timeReceived) = 0;              // user-defined packet is received
};
typedef QSharedPointer<CongestionControl> CongestionControlPointer;

}  // namespace udt4

#endif /* hifi_udt4_CongestionControl_h */