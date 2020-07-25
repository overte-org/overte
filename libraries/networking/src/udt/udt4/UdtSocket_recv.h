//
//  UdtSocket_recv.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-07-19.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtSocket_recv_h
#define hifi_udt4_UdtSocket_recv_h

#include "Packet.h"
#include "PacketID.h"
#include <QtCore/QDeadlineTimer>
#include <QtCore/QElapsedTimer>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

namespace udt4 {

class UdtSocket_private;
enum class UdtSocketState;

/* UdtSocket_receive

Implements the "incoming" side of a UDT socket connection, listening for incoming data packets,
assembling messages, sending ACKs for successful messages and NAKs for packet loss

This class is private and not user-accessible
*/
class UdtSocket_receive : public QThread {
    Q_OBJECT
public:
    UdtSocket_receive(UdtSocket_private& socket);

    void setState(UdtSocketState newState);
    void configureHandshake(const HandshakePacket& hsPacket);
    void packetReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void setACKperiod(std::chrono::milliseconds ack);
    void setACKinterval(unsigned ack);

private slots:
    void ACKevent();

private: // private datatypes
    static constexpr unsigned ACK_SELF_CLOCK_INTERVAL{ 64 }; // involved in figuring whether we should get away with a "light" ACK packet with minimal information

    struct ReceivedPacket {
        Packet        udtPacket;
        QElapsedTimer timeReceived;

        inline ReceivedPacket(const Packet& p, const QElapsedTimer& t);
    };
    typedef QList<ReceivedPacket> ReceivedPacketList;

    struct ACKHistoryEntry {
        SequenceNumber ackID;
        PacketID       lastPacket;
        QElapsedTimer  sendTime;
    };
    typedef QHash<SequenceNumber, ACKHistoryEntry> ACKHistoryMap;
    
    struct ReceiveLossEntry {
        PacketID      packetID;
        QElapsedTimer lastFeedback;
        unsigned      numNAK{ 0 };

        inline ReceiveLossEntry(const PacketID& p) : packetID(p) {}
    };
    typedef std::map<PacketID, ReceiveLossEntry, WrappedSequenceLess<PacketID>> ReceiveLossMap;

    typedef std::map<PacketID, DataPacket, WrappedSequenceLess<PacketID>> DataPacketMap;

    typedef QList<std::chrono::microseconds> QDurationList;

private:  // called exclusively within our private thread
    void startupInit();
    virtual void run() override;
    bool processEvent(QMutexLocker& eventGuard);
    void ingestACK2(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void ingestMsgDropReq(const MessageDropRequestPacket& dropPacket, const QElapsedTimer& timeReceived);
    void ingestData(const DataPacket& dataPacket, const QElapsedTimer& timeReceived);
    bool attemptProcessPacket(const DataPacket& dataPacket, bool isNew);
    void sendLightACK();
    void getReceiveSpeeds(unsigned& recvSpeed, unsigned& bandwidth);
    void sendACK();
    void sendNAK(const ReceiveLossMap& receiveLoss);
    void ackEvent();

private:
    UdtSocket_private& _socket;  // private interface pointing back at the UdtSocket

    // this is a condition-based thread.  All the variables in this block can only be accessed while holding _eventMutex
    QMutex _eventMutex;
    QWaitCondition _eventCondition;
    bool _flagListenerShutdown{ false };      // are we wanting to shutdown this listener?
    bool _flagRecentACKevent{ false };        // has the ACK timer fired recently?
    ReceivedPacketList _receivedPacketList;   // list of packets we have not yet processed

private:
    PacketID _farNextPktSeq;               // the peer's next largest packet ID expected.
    PacketID _farRecdPktSeq;               // the peer's last "received" packet ID (before any loss events)
    SequenceNumber _lastACK;               // last ACK packet we've sent
    SequenceNumber _largestACK;            // largest ACK packet we've sent that has been acknowledged (by an ACK2).
    DataPacketMap _recvPktPend;            // list of packets that are waiting to be processed.
    ReceiveLossMap _recvLossList;          // loss list.
    ACKHistoryMap _ackHistory;             // list of sent ACKs.
    PacketID _sentACK;                     // largest PacketID we've sent an ACK regarding
    PacketID _recvACK2;                    // largest PacketID we've received an ACK2 from
    QElapsedTimer _recvLastArrival;        // time of the most recent data packet arrival
    QElapsedTimer _recvLastProbe;          // time of the most recent data packet probe packet
    QAtomicInteger<quint32> _ackPeriod;    // (set by congestion control) delay between sending ACKs (in milliseconds)
    QAtomicInteger<unsigned> _ackInterval; // (set by congestion control) number of data packets to send before sending an ACK
    unsigned _unackPktCount{ 0 };          // number of packets we've received that we haven't sent an ACK for
	unsigned _lightAckCount{ 0 };          // number of "light ACK" packets we've sent since the last ACK
    QDurationList _recvPktHistory;         // list of recently received packets.
    QDurationList _recvPktPairHistory;     // probing packet window.

	// timers
    QDeadlineTimer  _ACKsentEvent2; // if an ACK packet has recently sent, don't include link information in the next one
    QDeadlineTimer  _ACKsentEvent;  // if an ACK packet has recently sent, wait before resending it
    QTimer          _ACKtimer;      // controls when to send an ACK to our peer

private:
    Q_DISABLE_COPY(UdtSocket_receive)
};

}  // namespace udt4

#include "UdtSocket_recv.inl"
#endif /* hifi_udt4_UdtSocket_recv_h */