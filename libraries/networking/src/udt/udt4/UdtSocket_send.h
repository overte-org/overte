//
//  UdtSocket_send.h
//  libraries/networking/src/udt/udt4
//
//  Created by Heather Anderson on 2020-06-20.
//  Copyright 2020 Vircadia contributors
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_udt4_UdtSocket_send_h
#define hifi_udt4_UdtSocket_send_h

#include "Packet.h"
#include "PacketID.h"
#include <QtCore/QDeadlineTimer>
#include <QtCore/QElapsedTimer>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

namespace udt4 {

class UdtSocket_private;
class UdtSocket;
enum class UdtSocketState;

/* UdtSocket_send

Implements the "outgoing" side of a UDT socket connection, scheduling outgoing packets and listening
for packet-loss and acknowledgements from the far side

This class is private and not user-accessible
*/
class UdtSocket_send : public QThread {
    Q_OBJECT
public:
    UdtSocket_send(UdtSocket_private& socket);

    void setState(UdtSocketState newState);
    void configureHandshake(const HandshakePacket& hsPacket, bool resetSequence);
    void packetReceived(const Packet& udtPacket, const QElapsedTimer& timeReceived);
    void queueDisconnect();
    void resetReceiveTimer();

private slots:
    void SNDevent();
    void EXPevent();

private:
    enum class SendState
    {
        Closed,      // Connection is closed
        Idle,        // Connection is open and waiting
        Sending,     // Recently sent a packet
        Waiting,     // Waiting for the peer to process a packet
        ProcessDrop, // Dropping a lost packet
        Shutdown,    // Connection has been recently closed and is listening for packet-resend requests
    };

private:  // called exclusively within our private thread
    virtual void run() override;
    bool processEvent(QMutexLocker& eventGuard);
    void processDataMsg(bool isFirst);
    SendState reevalSendState() const;
    bool processSendLoss();
    bool processSendExpire();
    void processExpEvent();

private:
    UdtSocket_private& _socket;  // private interface pointing back at the UdtSocket

    // this is a condition-based thread.  All the variables in this block can only be accessed while holding _eventMutex
    QMutex         _eventMutex;
    QWaitCondition _eventCondition;
    UdtSocketState _socketState;
    bool           _flagRecentReceivedPacket{ false }; // has a packet been recently received from our peer?
    bool           _flagRecentEXPevent{ false };       // has the EXP timer fired recently?
    bool           _flagRecentSNDevent{ false };       // has the SND timer fired recently?
    bool           _flagSendDisconnect{ false };       // are we being asked to send a Disconnect packet?

//	// channels
//	sendEvent     <-chan recvPktEvent    // sender: ingest the specified packet. Sender is readPacket, receiver is goSendEvent
//	messageOut    <-chan sendMessage     // outbound messages. Sender is client caller (Write), Receiver is goSendEvent. Closed when socket is closed
//	sendPacket    chan<- packet.Packet   // send a packet out on the wire
//	shutdownEvent chan<- shutdownMessage // channel signals the connection to be shutdown

    // While the send thread is running these variables only to be accessed by that thread (can be initialized carefully by other threads)
	SendState      _sendState{SendState::Closed};  // current sender state
//	sendPktPend    sendPacketHeap  // list of packets that have been sent but not yet acknoledged
	PacketID       _sendPacketID;   // the current packet sequence number
//	msgPartialSend *sendMessage    // when a message can only partially fit in a socket, this is the remainder
//	msgSeq         uint32          // the current message sequence number
//	expCount       uint            // number of continuous EXP timeouts.
	QElapsedTimer  _lastReceiveTime; // the last time we've heard something from the remote system
	PacketID       _lastAckPacketID; // largest packetID we've received an ACK from
//	sentAck2       uint32          // largest ACK2 packet we've sent
//	sendLossList   packetIDHeap    // loss list
//	sndPeriod      atomicDuration  // (set by congestion control) delay between sending packets
//	rtoPeriod      atomicDuration  // (set by congestion control) override of EXP timer calculations
//	congestWindow  atomicUint32    // (set by congestion control) size of the current congestion window (in packets)
	uint           _flowWindowSize; // negotiated maximum number of unacknowledged packets (in packets)

	// timers
	QTimer _SNDtimer;              // if a packet is recently sent, this timer fires when SND completes
	QTimer _EXPtimer;              // Fires when we haven't heard from the peer in a while
	QDeadlineTimer _ACK2SentTimer; // if an ACK2 packet has recently sent, wait SYN before sending another one

private:
    Q_DISABLE_COPY(UdtSocket_send)
};


}  // namespace udt4

#endif /* hifi_udt4_UdtSocket_send_h */