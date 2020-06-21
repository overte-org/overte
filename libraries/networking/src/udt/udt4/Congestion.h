//
//  Congestion.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-06-20.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#pragma once

#ifndef hifi_udt4_Congestion_h
#define hifi_udt4_Congestion_h

#include <QtCore/QList>
#include "Packet.h"

namespace udt4 {

// Interface provided to UDT congestion control systems
class CongestionControlParms {
public:
	virtual quint32 getSendCurrentSequenceNumber() const = 0; // returns the most recently sent packet ID
	virtual void setCongestionWindowSize(uint) = 0;               // sets the size of the congestion window (in packets)
	virtual uint getCongestionWindowSize() const = 0;         // gets the size of the congestion window (in packets)
	virtual uint getPacketSendPeriod() const = 0;             // gets the current delay between sending packets (in ms)
	virtual void setPacketSendPeriod(uint) = 0;               // sets the current delay between sending packets (in ms)
	virtual uint getMaxFlowWindow() const = 0;                // returns the largest number of unacknowledged packets we can receive (in packets)
	virtual void getReceiveRates(uint& recvSpeed, uint& bandwidth) const = 0; // returns the current calculated receive rate and bandwidth (in packets/sec)
	virtual uint getRTT() const = 0;                          // returns the current calculated roundtrip time between peers (in ms)
	virtual uint getMSS() const = 0;                          // returns the largest packet size we can currently send (in bytes)
	virtual void setACKPeriod(uint) = 0;                      // sets the time between ACKs sent to the peer (in ms)
	virtual void setACKInterval(uint) = 0;                    // sets the number of packets sent to the peer before sending an ACK (in packets)
	virtual void setRTOPeriod(uint) = 0;                      // overrides the default EXP timeout calculations waiting for data from the peer (in ms)
};

// Interface to be implemented by UDT congestion control systems
class CongestionControl {
public:
    virtual void init(CongestionControlParms& parms) = 0;                                    // connection is being setup.
    virtual void close(CongestionControlParms& parms) = 0;                                   // connection is closed.
    virtual void onACK(CongestionControlParms& parms, quint32 packetID) = 0;                 // ACK packet is received
    virtual void onNAK(CongestionControlParms& parms, const QList<quint32>& packetIDs) = 0;  // loss report is received
    virtual void onTimeout(CongestionControlParms& parms) = 0;                               // a timeout event occurs
    virtual void onPktSent(CongestionControlParms& parms, const Packet& packet) = 0;         // data is sent
    virtual void onPktRecv(CongestionControlParms& parms, const Packet& packet) = 0;         // data is received
	virtual void onCustomMsg(CongestionControlParms& parms, const Packet& packet) = 0;       // user-defined packet is received
};

}  // namespace udt4

#endif /* hifi_udt4_Congestion_h */