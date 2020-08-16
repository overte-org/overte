//
//  NativeCC.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-05-25.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#ifndef hifi_udt4_NativeCC_h
#define hifi_udt4_NativeCC_h

#include <chrono>
#include "CongestionControl.h"
#include "PacketID.h"
#include <QtCore/QElapsedTimer>
#include <QtCore/QRandomGenerator>

namespace udt4 {

// NativeCongestionControl implements the default congestion control logic for UDT
class NativeCongestionControl : public CongestionControl {
public:
    NativeCongestionControl();

public:                                                                                   // implementation of CongestionControl
    virtual void init(CongestionControlParms& parms);                                     // connection is being setup.
    virtual void close(CongestionControlParms& parms);                                    // connection is closed.
    virtual void onACK(CongestionControlParms& parms, PacketID packetID);                 // ACK packet is received
    virtual void onNAK(CongestionControlParms& parms, const QList<PacketID>& packetIDs);  // loss report is received
    virtual void onTimeout(CongestionControlParms& parms);                                // a timeout event occurs
    virtual void onPacketSent(CongestionControlParms& parms, const Packet& packet);       // data is sent
    virtual void onPacketReceived(CongestionControlParms& parms,
                                  const Packet& packet,
                                  const QElapsedTimer& timeReceived);                     // data is received
    virtual void onCustomMessageReceived(CongestionControlParms& parms,
                                         const Packet& packet,
                                         const QElapsedTimer& timeReceived);              // user-defined packet is received

private:
    static constexpr std::chrono::seconds ONE_SECOND{ 1 };

    QRandomGenerator _random;
    std::chrono::microseconds _rcInterval;     // UDT Rate control interval
    QElapsedTimer _lastRCTime;                 // last rate increase time
    bool _slowStart{ true };                   // if in slow start phase
    PacketID _lastAck;                         // last ACKed seq no
    bool _loss{ false };                       // if loss happened since last rate increase
    PacketID _lastDecSeq;                      // biggest sequence number when last time the packet sending rate is decreased
    std::chrono::microseconds _lastDecPeriod;  // value of PacketSendPeriod when last decrease happened
    unsigned _nakCount{ 0 };                   // current number of NAKs in the current period
    unsigned _decRandom{ 1 };                  // random threshold on decrease by number of loss events
    unsigned _avgNAKNum{ 0 };                  // average number of NAKs in a congestion period
    unsigned _decCount{ 0 };                   // number of decreases in a congestion epoch
};

}  // namespace udt4

#endif /* hifi_udt4_NativeCC_h */