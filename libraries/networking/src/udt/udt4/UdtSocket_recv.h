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
#include <QtCore/QElapsedTimer>
#include <QtCore/QMutex>
#include <QtCore/QThread>

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

private:  // called exclusively within our private thread
    virtual void run() override;
    bool processEvent(QMutexLocker& eventGuard);

private:
    UdtSocket_private& _socket;  // private interface pointing back at the UdtSocket

    // this is a condition-based thread.  All the variables in this block can only be accessed while holding _eventMutex
    QMutex _eventMutex;
    QWaitCondition _eventCondition;
    UdtSocketState _socketState;
    bool _flagRecentReceivedPacket{ false };  // has a packet been recently received from our peer?
    bool _flagRecentACKevent{ false };        // has the ACK timer fired recently?

// channels
//	sockClosed   <-chan struct{}      // closed when socket is closed
//	sockShutdown <-chan struct{}      // closed when socket is shutdown
//	recvEvent    <-chan recvPktEvent  // receiver: ingest the specified packet. Sender is readPacket, receiver is goReceiveEvent
//	messageIn    chan<- []byte        // inbound messages. Sender is goReceiveEvent->ingestData, Receiver is client caller (Read)
//	sendPacket   chan<- packet.Packet // send a packet out on the wire
//	socket       *udtSocket

private:
    PacketID _farNextPktSeq; // the peer's next largest packet ID expected.
    PacketID _farRecdPktSeq; // the peer's last "received" packet ID (before any loss events)
//	lastACK            uint32          // last ACK packet we've sent
//	largestACK         uint32          // largest ACK packet we've sent that has been acknowledged (by an ACK2).
//	recvPktPend        dataPacketHeap  // list of packets that are waiting to be processed.
//	recvLossList       receiveLossHeap // loss list.
//	ackHistory         ackHistoryHeap  // list of sent ACKs.
    PacketID _sentAck; // largest packetID we've sent an ACK regarding
    PacketID _recvAck2; // largest packetID we've received an ACK2 from
//	recvLastArrival    time.Time       // time of the most recent data packet arrival
//	recvLastProbe      time.Time       // time of the most recent data packet probe packet
//	ackPeriod          atomicDuration  // (set by congestion control) delay between sending ACKs
//	ackInterval        atomicUint32    // (set by congestion control) number of data packets to send before sending an ACK
//	unackPktCount      uint            // number of packets we've received that we haven't sent an ACK for
//	lightAckCount      uint            // number of "light ACK" packets we've sent since the last ACK
//	recvPktHistory     []time.Duration // list of recently received packets.
//	recvPktPairHistory []time.Duration // probing packet window.

	// timers
//	ackSentEvent2 <-chan time.Time // if an ACK packet has recently sent, don't include link information in the next one
//	ackSentEvent  <-chan time.Time // if an ACK packet has recently sent, wait before resending it
    QTimer _ACKtimer;  // controls when to send an ACK to our peer
};

}  // namespace udt4

#endif /* hifi_udt4_UdtSocket_recv_h */