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
#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

namespace udt4 {

class UdtSocket_private;
enum class UdtSocketState;

class UdtSocket_receive : public QThread {
    Q_OBJECT
public:
    UdtSocket_receive(UdtSocket_private& socket);

    void setState(UdtSocketState newState);
    void configureHandshake(const HandshakePacket& hsPacket);
    void packetReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);

private slots:
    void ACKevent();

private: // private datatypes
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
    };
    typedef std::map<PacketID, ReceiveLossEntry, WrappedSequenceLess<PacketID>> ReceiveLossMap;

private:  // called exclusively within our private thread
    virtual void run() override;
    bool processEvent(QMutexLocker& eventGuard);
    void ingestACK2(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void ingestMsgDropReq(const MessageDropRequestPacket& dropPacket, const QElapsedTimer& timeReceived);
    void ingestData(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void ingestError(const Packet& udtPacket);
    void ingestShutdown();
    bool attemptProcessPacket(const Packet& udtPacket, bool isNew);
    void sendLightACK();
    void getReceiveSpeeds(int& recvSpeed, int& bandwidth);
    void sendACK();
    //void sendNAK(receiveLossHeap rl);
    void ackEvent();

private:
    UdtSocket_private& _socket;  // private interface pointing back at the UdtSocket

    // this is a condition-based thread.  All the variables in this block can only be accessed while holding _eventMutex
    QMutex _eventMutex;
    QWaitCondition _eventCondition;
    UdtSocketState _socketState;
    bool _flagListenerShutdown{ false };      // are we wanting to shutdown this listener?
    bool _flagRecentACKevent{ false };        // has the ACK timer fired recently?
    ReceivedPacketList _receivedPacketList;   // list of packets we have not yet processed

// channels
//	recvEvent    <-chan recvPktEvent  // receiver: ingest the specified packet. Sender is readPacket, receiver is goReceiveEvent
//	messageIn    chan<- []byte        // inbound messages. Sender is goReceiveEvent->ingestData, Receiver is client caller (Read)
//	sendPacket   chan<- packet.Packet // send a packet out on the wire

private:
    PacketID _farNextPktSeq; // the peer's next largest packet ID expected.
    PacketID _farRecdPktSeq; // the peer's last "received" packet ID (before any loss events)
//	lastACK            uint32          // last ACK packet we've sent
    SequenceNumber _largestACK;          // largest ACK packet we've sent that has been acknowledged (by an ACK2).
//	recvPktPend        dataPacketHeap  // list of packets that are waiting to be processed.
    ReceiveLossMap _recvLossList;  // loss list.
    ACKHistoryMap _ackHistory;  // list of sent ACKs.
    PacketID _sentACK;       // largest PacketID we've sent an ACK regarding
    PacketID _recvACK2;      // largest PacketID we've received an ACK2 from
    QElapsedTimer _recvLastArrival;    // time of the most recent data packet arrival
    QElapsedTimer _recvLastProbe;       // time of the most recent data packet probe packet
//	ackPeriod          atomicDuration  // (set by congestion control) delay between sending ACKs
//	ackInterval        atomicUint32    // (set by congestion control) number of data packets to send before sending an ACK
//	unackPktCount      uint            // number of packets we've received that we haven't sent an ACK for
//	lightAckCount      uint            // number of "light ACK" packets we've sent since the last ACK
//	recvPktHistory     []time.Duration // list of recently received packets.
//	recvPktPairHistory []time.Duration // probing packet window.

	// timers
    //QDeadlineTimer	_ACKsentEvent2; // if an ACK packet has recently sent, don't include link information in the next one
    //QDeadlineTimer  _ACKsentEvent;  // if an ACK packet has recently sent, wait before resending it
    QTimer          _ACKtimer;      // controls when to send an ACK to our peer
};

}  // namespace udt4

#include "UdtSocket_recv.inl"
#endif /* hifi_udt4_UdtSocket_recv_h */