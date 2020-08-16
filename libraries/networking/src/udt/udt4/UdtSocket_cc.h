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
#include <list>
#include "Packet.h"
#include "PacketID.h"
#include <QtCore/QAtomicInteger>
#include <QtCore/QElapsedTimer>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>

namespace udt4 {

class UdtSocket_private;
class UdtSocket_receive;
class UdtSocket_send;

class UdtSocket_CongestionControl : public QThread, private CongestionControlParms {
    Q_OBJECT
public:
    UdtSocket_CongestionControl(UdtSocket_private& socket);
    ~UdtSocket_CongestionControl();
    void setCongestionControl(CongestionControlPointer congestionControl);

public:  // functions accessible to UdtSocket objects
    void init(const PacketID& packetID, unsigned mtu);
    void close();
    void onACK(const PacketID& lastPacketReceived);
    void onNAK(QList<PacketID>&& packetIDs);
    void onTimeout();
    void onDataPacketSent(const PacketID& packetID);
    void onPacketSent(const Packet& udtPacket);
    void onPacketReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void onCustomMessageReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);

private:  // CongestionControlParms interface
    virtual PacketID getSendCurrentPacketID() const;                               // returns the most recently sent packet ID
    virtual void setCongestionWindowSize(unsigned);                                // sets the size of the congestion window (in packets)
    virtual unsigned getCongestionWindowSize() const;                              // gets the size of the congestion window (in packets)
    virtual std::chrono::microseconds getPacketSendPeriod() const;                 // gets the current delay between sending packets
    virtual void setPacketSendPeriod(std::chrono::microseconds);                   // sets the current delay between sending packets
    virtual unsigned getMaxFlowWindow() const;                                     // returns the largest number of unacknowledged packets we can receive (in packets)
    virtual void getReceiveRates(unsigned& recvSpeed, unsigned& bandwidth) const;  // returns the current calculated receive rate and bandwidth (in packets/sec)
    virtual std::chrono::microseconds getRTT() const;                              // returns the current calculated roundtrip time between peers
    virtual unsigned getMSS() const;                                               // returns the largest packet size we can currently send (in bytes)
    virtual void setACKPeriod(std::chrono::microseconds);                          // sets the time between ACKs sent to the peer
    virtual void setACKInterval(unsigned);                                         // sets the number of packets sent to the peer before sending an ACK (in packets)
    virtual void setRTOPeriod(std::chrono::microseconds);                          // overrides the default EXP timeout calculations waiting for data from the peer

private:  // internal structures
    enum class EventType
    {
        unknown,
        init,
        close,
        onACK,
        onNAK,
        onTimeout,
        onPacketSent,
        onPacketReceived,
        onCustomMessageReceived,
    };

    struct Event {
        EventType type{ EventType::unknown };
        Packet udtPacket;
        QList<PacketID> packetIDs;
        QElapsedTimer timeReceived;
    };
    typedef std::list<Event> EventList;

private:  // internal variables
    UdtSocket_private& _socket;                      // reference to top-level UDT socket private interface
    CongestionControlPointer _congestion;            // congestion control object for this socket
    QAtomicInteger<quint32> _lastSentPacketID{ 0 };  // packetID of most recently sent packet
    QAtomicInteger<unsigned> _mtu{ 1500 };           // the MTU for the connection
    unsigned _congestionWindow{ 16 };                // size of congestion window (in packets)
    std::chrono::microseconds _sndPeriod{ 0 };       // delay between sending packets

    mutable QMutex _eventProtect;
    QWaitCondition _eventCondition;
    bool _inShutdown{ false };
    EventList _events;

private:  // internal implementation
    virtual void run() override;
    void submitEvent(Event&& event);
    void handleEvent(const Event& event);

private:
    Q_DISABLE_COPY(UdtSocket_CongestionControl)
};

}  // namespace udt4

#endif /* hifi_udt4_UdtSocket_cc_h */