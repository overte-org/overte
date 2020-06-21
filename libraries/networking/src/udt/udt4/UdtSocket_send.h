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

#include <QtCore/QElapsedTimer>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include "Packet.h"

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

private:
    virtual void run() override;

private:
    enum class SendState
    {
        Idle,
        Sending,
        Waiting,
        ProcessDrop,
        Shutdown,
    };

    UdtSocket_private& _socket;          // private interface pointing back at the UdtSocket

//	// channels
//	sockClosed    <-chan struct{}        // closed when socket is closed
//	sockShutdown  <-chan struct{}        // closed when socket is shutdown
//	sendEvent     <-chan recvPktEvent    // sender: ingest the specified packet. Sender is readPacket, receiver is goSendEvent
//	messageOut    <-chan sendMessage     // outbound messages. Sender is client caller (Write), Receiver is goSendEvent. Closed when socket is closed
//	sendPacket    chan<- packet.Packet   // send a packet out on the wire
//	shutdownEvent chan<- shutdownMessage // channel signals the connection to be shutdown
//	socket        *udtSocket

	SendState      _sendState{SendState::Idle};  // current sender state
//	sendPktPend    sendPacketHeap  // list of packets that have been sent but not yet acknoledged
	quint32        _sendPacketID;   // the current packet sequence number
//	msgPartialSend *sendMessage    // when a message can only partially fit in a socket, this is the remainder
//	msgSeq         uint32          // the current message sequence number
//	expCount       uint            // number of continuous EXP timeouts.
	QElapsedTimer  _lastReceiveTime; // the last time we've heard something from the remote system
	quint32        _lastAckPacketID; // largest packetID we've received an ACK from
//	sentAck2       uint32          // largest ACK2 packet we've sent
//	sendLossList   packetIDHeap    // loss list
//	sndPeriod      atomicDuration  // (set by congestion control) delay between sending packets
//	rtoPeriod      atomicDuration  // (set by congestion control) override of EXP timer calculations
//	congestWindow  atomicUint32    // (set by congestion control) size of the current congestion window (in packets)
	uint           _flowWindowSize; // negotiated maximum number of unacknowledged packets (in packets)

	// timers
	QTimer _SNDevent;      // if a packet is recently sent, this timer fires when SND completes
	QTimer _ACK2SentEvent; // if an ACK2 packet has recently sent, wait SYN before sending another one
	QTimer _EXPtimerEvent; // Fires when we haven't heard from the peer in a while

private:
    Q_DISABLE_COPY(UdtSocket_send)
};


}  // namespace udt4

#endif /* hifi_udt4_UdtSocket_send_h */