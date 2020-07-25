//
//  UdtSocket_cc.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-07-23.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtSocket_cc_h
#define hifi_udt4_UdtSocket_cc_h

#include "CongestionControl.h"
#include "Packet.h"
#include "PacketID.h"
#include <QtCore/QAtomicInteger>
#include <QtCore/QElapsedTimer>

namespace udt4 {

class UdtSocket_private;
class UdtSocket_receive;
class UdtSocket_send;

class UdtSocket_CongestionControl : private CongestionControlParms {
public:
    UdtSocket_CongestionControl(UdtSocket_private& socket);
    void setCongestionControl(CongestionControlPointer congestionControl);

public: // functions accessible to UdtSocket objects
    void init();
    void close();
    void onACK(const PacketID& lastPacketReceived);
    void onNAK(const QList<PacketID>& packetIDs);
    void onTimeout();
    void onDataPktSent(const PacketID& packetID);
    void onPacketSent(const Packet& udtPacket);
    void onPacketReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void onCustomMessageReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);

private: // CongestionControlParms interface
	virtual PacketID getSendCurrentPacketID() const;                              // returns the most recently sent packet ID
	virtual void setCongestionWindowSize(unsigned);                               // sets the size of the congestion window (in packets)
	virtual unsigned getCongestionWindowSize() const;                             // gets the size of the congestion window (in packets)
	virtual std::chrono::milliseconds getPacketSendPeriod() const;                // gets the current delay between sending packets
	virtual void setPacketSendPeriod(std::chrono::milliseconds);                  // sets the current delay between sending packets
	virtual unsigned getMaxFlowWindow() const;                                    // returns the largest number of unacknowledged packets we can receive (in packets)
	virtual void getReceiveRates(unsigned& recvSpeed, unsigned& bandwidth) const; // returns the current calculated receive rate and bandwidth (in packets/sec)
	virtual std::chrono::microseconds getRTT() const;                             // returns the current calculated roundtrip time between peers
	virtual unsigned getMSS() const;                                              // returns the largest packet size we can currently send (in bytes)
	virtual void setACKPeriod(std::chrono::milliseconds);                         // sets the time between ACKs sent to the peer
	virtual void setACKInterval(unsigned);                                        // sets the number of packets sent to the peer before sending an ACK (in packets)
	virtual void setRTOPeriod(std::chrono::milliseconds);                         // overrides the default EXP timeout calculations waiting for data from the peer

private: // internal variables
	UdtSocket_private& _socket;                     // reference to top-level UDT socket private interface
    CongestionControlPointer _congestion;           // congestion control object for this socket
    QAtomicInteger<quint32> _lastSentPacketID{ 0 }; // packetID of most recently sent packet
    unsigned _congestionWindow{ 16 };               // size of congestion window (in packets)
    std::chrono::milliseconds _sndPeriod{ 0 };      // delay between sending packets
};


}  // namespace udt4

#endif /* hifi_udt4_UdtSocket_cc_h */